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

// kernel.c - kernel proper

#include <ktux.h>
#include <interrupts.h>
#include <isr.h>
#include <hardware.h>
#include <memory.h>
#include <pic.h>
#include <timer.h>
#include <keyboard.h>
#include <stdio.h>
#include <video.h>
#include <process.h>
#include <sh.h>

#define BUFFER_SIZE 0x1000

systemInfo *thisSystem;

unsigned int *pointer;
//extern unsigned int t1;

// We come in with interrupts disabled
void _kernel_main(void *bootParam)
{
	// First initialize RAM and enable paging
	init_virt_mem(bootParam);

	// After paging enabled, update pointers
	thisSystem = virt2Phys(bootParam);
	bootParam = NULL;				// NULL our old one

	// Initialize console for stdout
	init_console();
	kprintf("initializing system:\nmemory ...\n");

	// Display memory info
	print_mem_info(thisSystem);
	kprintf("  [ OK ]\n");

	// Display video info
	kprintf("video : ");
	print_video_info();
	kprintf("  [ OK ]\n");

	// Initialize interrupts
	kprintf("interrupts ...\n");
	init_interrupts();
	kprintf("  [ OK ]\n");

	// Heap
	kprintf("kernel heap ...\n");
	init_heap();
	//while (1);
	kprintf("  [ OK ]\n");

	// Hardware
	kprintf("hardware ...\n");
	init_pic();
	init_timer();
	init_keyboard();
	kprintf("  [ OK ]\n");

	kprintf("scheduler ...\n");
	init_multitasking();
	kprintf("  [ OK ]\n");

	//init_multitasker();
	//spawn_threads();
	//init_tasks();

	/* ok, so we can't use malloc here.
	 * calling it returns 0xd0000000 for some reason
	 * even though it's been called before, the heap
	 * certainly is not empty. */
	/*pointer = kmalloc(0x1000);
	pointer = kmalloc(0x2880);
	pointer = kmalloc(0x488);
	kprintf("pointer=0x%p\n", pointer);*/

	/*pointer = kmalloc(BUFFER_SIZE);
	kprintf("pointer=0x%p\n", pointer);
	for (i=0;i<BUFFER_SIZE>>2;i++) {
		//kprintf("0x%p %d\t", &pointer[i], i);
		pointer[i]=0;
	}
	kprintf("wrote to buffer successfully\n");*/
	//while(1);

	/*kprintf("malloc(100000) = 0x%p\n", kmalloc(0x10000));
	kprintf("malloc(100000) = 0x%p\n", kmalloc(0x10000));
	kprintf("malloc(100000) = 0x%p\n", kmalloc(0x10000));
	kprintf("malloc(100000) = 0x%p\n", kmalloc(0x10000));
	kprintf("malloc(80000) = 0x%p\n", kmalloc(0x8000));*/

	// Remap our PICs, set IRQ0 to 18Hz, enable only IRQs 0&1, and turn on interrupts
	kprintf("Reprogramming timer and enabling IRQs...\n");

	init_shell();
	shell_prompt();

	reprogram_timer(1000);
	enable_irq(1);
	enable_irq(0);
	start_scheduler();
	sti();

	//kill_thread();
	//kprintf("\nPrepare idle...");
	while(1) {
		//t1++;
		//do_idle();
		//yield();
		//asm volatile("xchg bx, bx");
		//asm volatile("int $0x20");
	}

	//kprintf("\nmalloc(10000) = 0x%p\n", kmalloc(0x10000));
	//__asm__ __volatile("int $0x80");

	kprintf("kernel panic: exceution continuing beyond _kernel_main address space!");
	while(1);
}
