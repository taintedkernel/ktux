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
#include <memory.h>         // allocatePage()
#include <interrupts.h>     // IRQ_TIMER
#include <selectors.h>
#include <string.h>         // memsetw
#include <stdio.h>
#include <video.h>          // putch()
#include <tasks.h>
#include <list.h>
#include <debug.h>          // DEBUG_BP
#include <ktux.h>


extern unsigned int tss;

//extern void *pageBuffer;
//unsigned int nextPID;
//process_struct *threads[MAX_THREADS];
//tss_struct systemTSS;
//process_struct *process;
//pmode_task_struct *pmodeTask;
//dlNode *pCurrentTaskNode = NULL;
//dList *processList;

//unsigned int idlePID;
unsigned int currentTask = 0;
unsigned int numThreads = MAX_THREADS;

kernel_thread_t *pCurrentTask = NULL;
kernel_thread_t *pThreads[MAX_THREADS];

struct list_head lThreads = list_head_init(lThreads);

void init_multitasking()
{
    unsigned int i;

    // Initialize data
    numThreads = 0;
    for (i=0; i<MAX_THREADS; i++)
        pThreads[i] = NULL;

    // First we need to allocate storage for the
    // original kernel execution thread.  This will be
    // pre-empted when our first timer interrupt occurs
    create_kernel_thread("init", 0);

    // create more threads of execution
    create_kernel_thread("monitor", thread_monitor);
    create_kernel_thread("foo", thread_foo);
    create_kernel_thread("bar", thread_bar);
    create_kernel_thread("gamma", thread_gamma);

    /*lea   eax, [tss]
    mov ebx, 0x0528             ; GDTbase (500) + TSS selector (28)
    mov [ebx+2], ax
    shr eax, 16
    mov [ebx+4], al
    mov [ebx+7], ah

    mov ax, 0x28                ; ../include/selectors.h
    ltr ax
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
    if (numThreads == MAX_THREADS)
        return;

    currentTask = 0;
    pCurrentTask = pThreads[currentTask];
    pCurrentTask->time = 0;
    pCurrentTask->quantum = 0;

    enable_irq(IRQ_TIMER);
}

unsigned int __task_switch(unsigned int currentESP)
{
    // if not init
    if ( pCurrentTask == NULL)
        return currentESP;

    // update timeslice, cpu time and return
    // we should RDTSC here to accurately count processor time
    if (pCurrentTask->quantum > 0)
    {
        pCurrentTask->time++;
        pCurrentTask->quantum--;
        return currentESP;
    }

    // thread has been killed
    // do not release memory, re-init and re-use (free if needed)
    if (pCurrentTask->status == PROCESS_ZOMBIE)
    {
        pCurrentTask->id = 0;
        pCurrentTask->esp0 = pCurrentTask->esp0base;
        pCurrentTask->status = PROCESS_FREE;
        //kfree(pCurrentTask);
    }
    // process finished with timeslice
    // return to waiting queue
    else if ( pCurrentTask->quantum == 0 )
    {
        pCurrentTask->time++;
        pCurrentTask->esp0 = currentESP;
        pCurrentTask->status = PROCESS_WAIT;
    }

    // find new thread
    // this will be changed shortly to use priority queues
    while(1) {
        if ( ++currentTask == get_num_threads() )
            currentTask = 0;
        if ( pThreads[currentTask] != NULL )
        {
            pCurrentTask = pThreads[currentTask];
            if ( pCurrentTask->status == PROCESS_READY ||
                    pCurrentTask->status == PROCESS_WAIT )
                break;
        }
    }

    // set a quantum, thread to running state, and return stack pointer
    pCurrentTask->quantum = QuantumAmount(pCurrentTask);
    pCurrentTask->status = PROCESS_RUNNING;
    return pCurrentTask->esp0;
}

void yield()
{
    pCurrentTask->quantum = 0;
    asm volatile("int $0x20");
    //DEBUG_BP
}

void kill()
{
    pCurrentTask->status = PROCESS_ZOMBIE;
    cli();
    //free(threads[currentTask]);
    list_del((void *)&pCurrentTask->list);
    pThreads[currentTask] = NULL;
    sti();
    yield();
}

// create a new thread of execution.  returns PID.
unsigned int create_kernel_thread(char *name, void (*eip)())
{
    unsigned int *stack;
    kernel_thread_t *thread;

    thread = (kernel_thread_t*)kmalloc(sizeof(kernel_thread_t));
    if (!thread) {
        return ERROR;
    }
    thread->status = PROCESS_INIT;
    thread->esp0base = (unsigned int)kmalloc(STACK_SIZE);

    // allocate a stack and push default register data
    stack = (unsigned int*)(thread->esp0base + STACK_SIZE);
    kprintf("create_thread(), thread=0x%08X, esp0=0x%08X\n", thread, stack);
    *--stack = (unsigned int)0x0202;                    // EFLAGS
    *--stack = (unsigned int)KERNEL_CODE_SELECTOR;      // CS
    *--stack = (unsigned int)eip;                       // EIP

    // pushad
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;

    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;

    // segment registers
    *--stack = KERNEL_DATA_SELECTOR;        // DS
    *--stack = KERNEL_DATA_SELECTOR;        // ES
    *--stack = KERNEL_DATA_SELECTOR;        // FS
    *--stack = KERNEL_DATA_SELECTOR;        // GS

    // initialize rest of struct
    memcpy((void *)thread->name, name, NAME_SIZE);
    thread->id = alloc_pid();
    thread->time = 0;
    thread->priv = THREAD_KERNEL;
    thread->esp0 = (unsigned int)stack;
    thread->status = PROCESS_READY;
    thread->quantum = 0;
    thread->priority = 0;

    // add new struct to the list
    cli();
    list_add((void *)&thread->list, &lThreads);
    pThreads[thread->id] = thread;
    sti();
    return thread->id;
}

// we limit modifying process priority to these two functions
// check bounds here and should be safe
inline void nice(int newPriority)
{
    if ((-1*INT_PER_SLICE) < newPriority && newPriority < INT_PER_SLICE)
        pCurrentTask->priority = newPriority;
}
inline void p_nice(kernel_thread_t *thread, int newPriority)
{
    if ((-1*INT_PER_SLICE) < newPriority && newPriority < INT_PER_SLICE)
        thread->priority = newPriority;
}

inline unsigned int alloc_pid(void) {
    // *TODO* : Make into a PID hash
    return numThreads++;
}

inline unsigned int get_num_threads() {
    return numThreads;
}

inline unsigned int p_get_page_directory(kernel_thread_t *thread, unsigned int pid) {
    return thread->cr3;
}



void ps()
{
    struct list_head *pos;
    kernel_thread_t *tmp;

    kprintf("\n");
    list_for_each_prev(pos, &lThreads) {
        tmp = list_entry(pos, kernel_thread_t, list);
        kprintf("thread : (%u) %s\n", tmp->id, tmp->name);
        kprintf("priority : %d\tstatus : %u\n", tmp->priority, tmp->status);
        kprintf("esp0 : %08x\tcpu_time: %u\n", tmp->esp0, tmp->time);
    }
}

void do_idle()
{
    static unsigned int i=0;
    static char progress[] = "\\-/|";
    static unsigned short cursor=0;
    static unsigned int p=0;
    //putch(' ');

    while(1) DEBUG_BP

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

/*unsigned int create_user_thread(unsigned int index, unsigned int entryPoint)
{
    unsigned int i;
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
    *--stack = 0;                           // EIP = 0
    *--stack = USER_DATA_SELECTOR;          // SS
    *--stack = USER_STACK + PAGE_SIZE;      // ESP

    // Regular stacks
    *--stack = 0;           // EDI
    *--stack = 0;           // ESI
    *--stack = 0;           // EBP
    *--stack = 0;           // offset
    *--stack = 0;           // EBX
    *--stack = 0;           // EDX
    *--stack = 0;           // ECX
    *--stack = 0;           // EAX

    *--stack = USER_DATA_SELECTOR;          // DS
    *--stack = USER_DATA_SELECTOR;          // ES
    *--stack = USER_DATA_SELECTOR;          // FS
    *--stack = USER_DATA_SELECTOR;          // GS

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

    numTasks++;
    return index+entryPoint;
}*/
