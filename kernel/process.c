/***************************************************************************
 *   Copyright (C) 2004 by Anthony DeChiaro                                *
 *   axd6491@njit.edu                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

// process.c - process management

#include <process.h>
#include <pic.h>
#include <memory.h>			// allocatePage()
#include <selectors.h>
#include <string.h>			// memsetw
#include <stdio.h>
#include <video.h>			// putch()

extern unsigned int tss;
//extern void *pageBuffer;

//unsigned int nextPID;
unsigned int idlePID;
unsigned int currentTask;
unsigned int numTasks;
process_struct *threads[MAX_THREADS];
tss_struct systemTSS;
process_struct *process;
pmode_task_struct *pmodeTask;

void test()
{
	static int i=0;
	
	while (1) {
		kprintf("test");
		for(i=0;i<1000;i++);
	}
}

void one()
{
	static int i=0;
	
	while (1) {
		if (i % 25) {
			move_cursor(1, 1);
			kprintf("%d", i);
		}
		i++;
		if (i%1000) i=0;
	}
}

void two()
{
	static int i=0;
	
	while (1) {
		if (i % 25) {
			move_cursor(40, 1);
			kprintf("%d", i);
		}
		i++;
		if (i%1000) i=0;
	}
}

void idle()
{
	static int i=0;
	
	while (1) {
		kprintf("idle");
	}
}

void init_multitasker()
{
	numTasks = 0;
	currentTask = 0;
	idlePID = create_kernel_thread("idle", (unsigned int)&idle, KERNEL_DATA_SELECTOR, 0x0202, 0);	

	kprintf("idle() PID=%d\n", idlePID);

/*	lea	eax, [tss]
	mov	ebx, 0x0528				; GDTbase (500) + TSS selector (28)
	mov	[ebx+2], ax
	shr	eax, 16
	mov [ebx+4], al
	mov [ebx+7], ah
	
	mov	ax, 0x28				; ../include/selectors.h
	ltr	ax
	ret*/

	/*
	// Load our system TSS
	__asm__("nop" : : "a" (tss));

	asm("movl $0x528, %ebx\n\t"
		"mov %ax, 2(%ebx)\n\t"
		"shr $0x10, %eax\n\t"
		"mov %al, 4(%ebx)\n\t"
		"mov %ah, 7(%ebx)\n\t"
		"mov $0x28, %ax\n\t"
		"ltr %ax\n\t");
	*/
}

void kill_thread()
{
	task_switch(0);
}

unsigned int task_switch(unsigned int oldESP)
{
	static unsigned int foundTask;

	// save current process stack, then load new task
	//if (oldESP > 0) {
		process = threads[currentTask];
		process->ustack = oldESP;
	//}
	
	foundTask = 0;
	while (!foundTask) {
		if (++currentTask > numTasks) currentTask = 0;
		if (threads[currentTask] != 0)
		{
			foundTask = 1;
			process = threads[currentTask];
			systemTSS.esp0 = process->kstack;
			putch('.');
			return process->ustack;
		}
	}

	
	/*kprintf("fatal error!\n");
		while(1);*/

	return 0;
}

void spawn_threads(void)
{
	kprintf("spawning threads...\ncreating thread 'one', returned=%d\n", create_kernel_thread("one", (unsigned int)&one, KERNEL_DATA_SELECTOR, 0x0202, 0));
	kprintf("creating thread 'two', returned=%d\n", create_kernel_thread("two", (unsigned int)&two, KERNEL_DATA_SELECTOR, 0x0202, 0));
}

unsigned int get_next_pid(void)
{
	return numTasks;
}

unsigned int get_page_directory(unsigned int pid)
{
	return pid; //threads[pid].cr3;
}

/* unsigned int CreateThread(index, type, *entryPoint())
 *	
 * This funtion creates a new thread of execution in the system.  Functionality
 * differs greatly for a kernel-level thread or a user application.  In all cases
 * a new stack is allocated, EIP is set with 'entryPoint', segment descriptors are
 * set correctly.  If creating a user-level thread, a new page directory is created
 * and virtual address space is allocated, set to size 'addressSpace'.  
 */
unsigned int create_kernel_thread(char *name, unsigned int entry, unsigned int ds, unsigned int eflags, char priv)
{
	unsigned int i;
	//unsigned int *stack;

	//kprintf("in create_kernel_thread()\n");
	cli();

	// find unallocated entry in thread array
	i = 0;
	while((unsigned int)threads[i] != 0 && i < MAX_THREADS) i++;
	if (i == MAX_THREADS) return -1;

	//kprintf("found i=%d\n", i);

	numTasks++;
	process = (process_struct *)kmalloc(sizeof(process_struct));

	kprintf("creating thread '%s', allocd' process struct=0x%p\n", name, process);

	threads[i] = process;
	
	//kprintf("marked process address\n");

	//memcpy(&process->name, name, strlen(name));
	
	/*memsetw(&process, 0, sizeof(process_struct));
	memsetw(&process->stack, 0, sizeof(process->stack));
	memsetw(&process->p10_stack, 0, sizeof(process->p10_stack));*/
	
	//kprintf("finished memset and memcpy\n", i);

	process->id = i;
	process->priv = priv;
	process->status = PROCESS_INIT;
	process->kstack = (unsigned int)process->p10_stack + STACK_SIZE;
	
	//kprintf("process->stack=0x%p\n", process->stack);
	
	pmodeTask = (pmode_task_struct *)(process->stack + STACK_SIZE);

	//stack = (unsigned int *)0xE0000000;
	//*stack = 100;

	pmodeTask->gs = ds;
	pmodeTask->fs = ds;
	pmodeTask->es = ds;
	pmodeTask->ds = ds;
	pmodeTask->ss = ds;
	pmodeTask->cs = KERNEL_CODE_SELECTOR;
	pmodeTask->eip = entry;
	pmodeTask->eflags = eflags;
	pmodeTask->esp = (unsigned int)process->stack + STACK_SIZE;
	pmodeTask->eax = 0;
	pmodeTask->ecx = 0;
	pmodeTask->edx = 0;
	pmodeTask->ebx = 0;
	pmodeTask->ebp = 0;
	pmodeTask->esi = 0;
	pmodeTask->edi = 0;
	process->ustack = (unsigned int)pmodeTask;

	kprintf("finished create_kernel_thread(), re-enabling interrupts\n");
	sti();

	return i;
}
	
	
/*
	newProcess = kmalloc(sizeof(process));
	memsetw((void *)&threads[index], 0, sizeof(threads[index]));

	// Stack
	threads[index].esp0 = alloc_page();
	map_kpage((unsigned int)pageBuffer, threads[index].esp0, (PDE_FLAG_PRESENT | PDE_FLAG_RW));
	
	stack = (unsigned int *)threads[index].esp0;

	// EFLAGS, CS, & EIP (SS & ESP also for DPL change)
	*--stack = 0x202;
	*--stack = KERNEL_CODE_SELECTOR;
	*--stack = (unsigned int)entryPoint;

	// Regular stacks
	*--stack = 0;			// EDI
	*--stack = 0;			// ESI
	*--stack = 0;			// EBP
	*--stack = 0;			// offset
	*--stack = 0;			// EBX
	*--stack = 0;			// EDX
	*--stack = 0;			// ECX
	*--stack = 0;			// EAX

	// Segment & other registers
	// Kernel-level (DPL 0) is easy, 
	*--stack = KERNEL_DATA_SELECTOR;		// DS
	*--stack = KERNEL_DATA_SELECTOR;		// ES
	*--stack = KERNEL_DATA_SELECTOR;		// FS
	*--stack = KERNEL_DATA_SELECTOR;		// GS
	
	// struct (*MUST* be after user-level page mapping)
	threads[index].esp0 = (unsigned int)stack;
	threads[index].pid = get_next_pid();
	threads[index].priority = 0;
	threads[index].status = PROCESS_INIT;
	threads[index].cr3 = (unsigned int)P_KERNEL_PAGE_DIRECTORY;

	numTasks++;
*/

unsigned int create_user_thread(unsigned int index, unsigned int entryPoint)
{
	/*unsigned int i;
	unsigned int *stack;
	unsigned int *newPD = NULL;
	unsigned int *newPT = NULL;
	
	newProcess = kmalloc(sizeof(process));
	memsetw((void *)&threads[index], 0, sizeof(threads[index]));

	// Stack
	threads[index].esp0 = alloc_page();
	map_kpage((unsigned int)pageBuffer, threads[index].esp0, (PDE_FLAG_PRESENT | PDE_FLAG_RW));
	
	stack = (unsigned int *)threads[index].esp0;

	// EFLAGS, CS, & EIP (SS & ESP also for DPL change)
	*--stack = 0x202;
	*--stack = USER_CODE_SELECTOR;
	*--stack = 0;							// EIP = 0
	*--stack = USER_DATA_SELECTOR;			// SS
	*--stack = USER_STACK + PAGE_SIZE;		// ESP

	// Regular stacks
	*--stack = 0;			// EDI
	*--stack = 0;			// ESI
	*--stack = 0;			// EBP
	*--stack = 0;			// offset
	*--stack = 0;			// EBX
	*--stack = 0;			// EDX
	*--stack = 0;			// ECX
	*--stack = 0;			// EAX

	*--stack = USER_DATA_SELECTOR;			// DS
	*--stack = USER_DATA_SELECTOR;			// ES
	*--stack = USER_DATA_SELECTOR;			// FS
	*--stack = USER_DATA_SELECTOR;			// GS	

	// Allocate new paging structures
	*newPD = alloc_page();
	*newPT = alloc_page();
	
	// we will have to initially map these pages to the kernel to write to them
	map_kpage(USER_PAGE_DIRECTORY, (unsigned int)newPD, (PTE_FLAG_PRESENT | PDE_FLAG_RW));

	// Page directory //
	// Map our inital page table to 1st 4mb in page directory
	i=0; newPD[i++] = makePDEphys((unsigned int)newPT, (PDE_FLAG_PRESENT | PDE_FLAG_RW | PDE_FLAG_USER));
	
	// Mark rest of page directory not present
	while (i % PDE_PER_PD > 0) {
		newPD[i++] = makePDEphys(0, PDE_FLAG_USER);
	}
	
	// Page tables //		
	// Map our firstly allocated page to the first 4kb
	i=0; newPT[i++] = makePTEphys((unsigned int)entryPoint, (PTE_FLAG_PRESENT | PTE_FLAG_RW | PTE_FLAG_USER));
	
	// Initialize rest of page table (mark not present)
	while (i % PTE_PER_PT > 0) {
		newPT[i++] = makePTEphys(0, (PTE_FLAG_RW | PTE_FLAG_USER));
	}

	// Map user stack
	map_page_helper(newPD, USER_STACK, threads[index].esp0, (PTE_FLAG_PRESENT | PTE_FLAG_RW | PTE_FLAG_USER));
	
	// Map kernel virtual address space
	map_ktables(newPD);
	
	// Map our page directory and tables...  read-only access to process
	map_page_helper(newPD, USER_PAGE_DIRECTORY, (unsigned int)newPD, (PTE_FLAG_PRESENT | PTE_FLAG_USER));
	map_page_helper(newPD, USER_PAGE_TABLE_1, (unsigned int)newPT, (PTE_FLAG_PRESENT | PTE_FLAG_USER));
	
	// struct (*MUST* be after user-level page mapping)
	threads[index].esp0 = USER_STACK + PAGE_SIZE;		
	threads[index].heapStart = KERNEL_HEAP_ADDR;

	//threads[index].esp0 = (unsigned int)stack;
	threads[index].pid = get_next_pid();
	threads[index].priority = 0;
	threads[index].status = PROCESS_INIT;
	threads[index].cr3 = (unsigned int)newPD;

	numTasks++;*/
	return index+entryPoint;
}
