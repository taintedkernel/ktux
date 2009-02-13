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
#include <tasks.h>
#include <ktux.h>

extern unsigned int tss;
//extern void *pageBuffer;

int numTasks;
unsigned int idlePID;
//unsigned int nextPID;
//process_struct *threads[MAX_THREADS];
//tss_struct systemTSS;
//process_struct *process;
//pmode_task_struct *pmodeTask;
unsigned int numThreads=0;

// *NEW*
process_struct threads[MAX_THREADS];
int currentTask = -1;

void init_multitasking()
{
	// 1 task: kernel idle loop, index=0
	numThreads = 0;

	// index zero occupied by original kernel thread
	create_thread(1, thread_one);
	create_thread(2, thread_two);
	create_thread(3, thread_monitor);

	kprintf("new thread[1].esp0 = 0x%p\n", threads[1].esp0);
	kprintf("new thread[2].esp0 = 0x%p\n", threads[2].esp0);
	kprintf("new thread[3].esp0 = 0x%p\n", threads[3].esp0);

	/*lea	eax, [tss]
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
		"ltr %ax\n\t");*/
}

void start_scheduler()
{
	currentTask = 0;
	threads[currentTask].time = 0;
}

volatile unsigned int task_switch(unsigned int currentESP)
{
	if (currentTask < 0)		// if not init
		return currentESP;

	if ( threads[currentTask].time == INT_PER_SLICE )
	{
		threads[currentTask].time = 0;
		threads[currentTask].esp0 = currentESP;
		threads[currentTask].status = PROCESS_SLEEP;
		while(1) {
			currentTask = ++currentTask % (numThreads+1);
			if ( threads[currentTask].status == PROCESS_SLEEP ||
				threads[currentTask].status == PROCESS_READY )
				break;
		}
		/*if ( currentTask == 0 )
		{
			// clean-up
		}*/
		//asm volatile("xchg %bx, %bx");

		threads[currentTask].status = PROCESS_RUN;
		return threads[currentTask].esp0;
	}

	threads[currentTask].time++;
	return currentESP;
}

/*void create_thread(void (*thread)())
{
	return;
}*/

void create_thread(unsigned int id, void (*thread)())
{
	unsigned int *stack;

	//kprintf("create_thread(%u) : entering function\n", id);

	threads[id].status = PROCESS_INIT;
	threads[id].esp0 = (unsigned int)kmalloc(STACK_SIZE) + STACK_SIZE;

	kprintf("create_thread(), esp0 : 0x%08X\n", threads[id].esp0);

	stack = (unsigned int *)threads[id].esp0;

	*--stack = (unsigned int)0x0202;					// EFLAGS
	*--stack = (unsigned int)KERNEL_CODE_SELECTOR;		// CS
	*--stack = (unsigned int)thread;					// EIP

	// PUSHAD
	*--stack = id;
	*--stack = id;
	*--stack = id;
	*--stack = id;

	*--stack = id;
	*--stack = id;
	*--stack = id;
	*--stack = id;

	*--stack = KERNEL_DATA_SELECTOR;			// DS
	*--stack = KERNEL_DATA_SELECTOR;			// ES
	*--stack = KERNEL_DATA_SELECTOR;			// FS
	*--stack = KERNEL_DATA_SELECTOR;			// GS

	numThreads++;
	threads[id].id = get_next_pid();
	threads[id].time = 0;
	threads[id].priv = THREAD_KERNEL;
	threads[id].esp0 = (unsigned int)stack;
	threads[id].status = PROCESS_READY;
	threads[id].priority = 0;
}

unsigned int get_next_pid(void)
{
	return numTasks;
}

unsigned int get_page_directory(unsigned int pid)
{
	return threads[pid].cr3;
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
	unsigned int *stack;
	process_struct *process;

	//kprintf("in create_kernel_thread()\n");
	cli();

	// find unallocated entry in thread array
	i = 0;
	//while((unsigned int)threads[i] != 0 && i < MAX_THREADS) i++;
	if (i == MAX_THREADS) return -1;
	//kprintf("found i=%d\n", i);

	numTasks++;

	unsigned int *test = kmalloc(0x10);
	int j;
	for (j=0;j<0x10;j++)
		test[j] = j;

	process = (process_struct *)kmalloc(sizeof(process_struct));

	kprintf("creating thread '%s', allocd process struct=0x%p\n", name, process);
	kprintf("\tentry=0x%p, ds=0x%p, eflags=0x%p, priv=0x%p\n", entry, ds, eflags, priv);

	//threads[i] = process;

	//kprintf("marked process address\n");

	//memcpy(&process->name, name, strlen(name));

	/*memsetw(&process, 0, sizeof(process_struct));
	memsetw(&process->stack, 0, sizeof(process->stack));
	memsetw(&process->p10_stack, 0, sizeof(process->p10_stack));*/

	//kprintf("finished memset and memcpy\n", i);

	process->id = i;
	process->priv = priv;
	process->status = PROCESS_INIT;

	process->esp0 = (unsigned int)kmalloc(0x100) + 0x100;
	stack = (unsigned int *)process->esp0;

	kprintf("\tallocd stack: from 0x%p to 0x%p\n", ((unsigned int)stack)-0x100, stack);

	// EFLAGS, CS, & EIP (SS & ESP also for DPL change)
	*--stack = eflags;
	*--stack = KERNEL_CODE_SELECTOR;
	*--stack = (unsigned int)entry;

	// Regular stacks
	*--stack = i;			// EDI
	*--stack = i;			// ESI
	*--stack = i;			// EBP
	*--stack = i;			// offset
	*--stack = i;			// EBX
	*--stack = i;			// EDX
	*--stack = i;			// ECX
	*--stack = i;			// EAX

	// Segment & other registers
	// Kernel-level (DPL 0) is easy,
	*--stack = ds;			// DS
	*--stack = ds;			// ES
	*--stack = ds;			// FS
	*--stack = ds;			// GS

	// struct (*MUST* be after user-level page mapping)
	process->esp0 = (unsigned int)stack;
	process->id = get_next_pid();
	process->priority = 0;
	process->status = PROCESS_INIT;
	process->cr3 = (unsigned int)P_KERNEL_PAGE_DIRECTORY;

	/*
	//process->kstack = (unsigned int)process->p10_stack + STACK_SIZE;	// ?????
	//kprintf("process->kstack=0x%p\n", process->kstack);

	pmodeTask = (pmode_task_struct *)(process->stack + STACK_SIZE);
	process->ustack = (unsigned int)pmodeTask;

	//stack = (unsigned int *)0xE0000000;
	// *stack = 100;

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
	*/

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

void do_idle()
{
	static unsigned int i=0;
	static char progress[] = "\\-/|";
	static unsigned short cursor=0;
	static unsigned int p=0;
	//putch(' ');

	kprintf("\nidle task : [   ]");
	cursor = get_csr_x() - 3;

	while (1) {
		i++;
		if (i%3) {
			kprintf("\x1B[%d;%dH\x1B[3%dm", get_csr_y(), cursor, p%8);
			putch(progress[p++%4]);
		}
	}
}
