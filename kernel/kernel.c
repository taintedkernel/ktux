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

#define BUFFER_SIZE 0x1000

systemInfo *thisSystem;

unsigned int *pointer;


// We come in with interrupts disabled
void _kernel_main(void *bootParam)
{
	unsigned int i;

	init_phys_mem(bootParam);
	thisSystem = bootParam;

	init_video();

	//clrscr();
	/*kprintf("kernel called with eax=%p\n", bootParam);
	kprintf("*bootParam=0x%p\n", thisSystem, thisSystem);
	kprintf("bootDrive=%d\n", thisSystem->bootDrive);
	kprintf("bootFS=%d\n", thisSystem->bootFileSystem);
	kprintf("*bootFATInfo=0x%p\n", thisSystem->bootFATinfo);
	kprintf("kernelSize=%d\n", thisSystem->kernelSize);
	kprintf("*cpuInfo=0x%p\n", thisSystem->cpuInfo);
	kprintf("*memoryInfo=0x%p\n", thisSystem->memoryInfo);*/

	//displayCPUInfo(thisSystem->cpuInfo);

	// Initialize PICs
	kprintf("Initializing:\n\tinterrupts...\n");
	init_interrupts();

	kprintf("\thardware...\n");
	init_pic();
	init_timer();
	init_keyboard();
	
	init_virt_mem();

	print_mem_info();
	//init_multitasker();
	spawn_threads();
	//init_tasks();
	
	pointer = kmalloc(BUFFER_SIZE);
	kprintf("pointer=0x%p\n", pointer);
	for (i=0;i<BUFFER_SIZE>>2;i++) {
		//kprintf("0x%p %d\t", &pointer[i], i);
		pointer[i]=0;
	}
		
	kprintf("wrote to buffer successfully\n");
	//while(1);
	
/*	kprintf("malloc(100000) = 0x%p\n", kmalloc(0x10000));
	kprintf("malloc(100000) = 0x%p\n", kmalloc(0x10000));
	kprintf("malloc(100000) = 0x%p\n", kmalloc(0x10000));
	kprintf("malloc(100000) = 0x%p\n", kmalloc(0x10000));
	kprintf("malloc(80000) = 0x%p\n", kmalloc(0x8000));*/

	// Remap our PICs, set IRQ0 to 18Hz, enable only IRQs 0&1, and turn on interrupts
	kprintf("Reprogramming timer and enabling IRQs...\nshell:~# ");

	reprogram_timer(10);
	//enable_irq(0);
	enable_irq(1);
	sti();
	
	//__asm__ __volatile("int $0x80");
	
	//toggle_interrupts(false);
	//schedule();
	while(1) {
		//kprintf("\nmalloc(10000) = 0x%p\n", kmalloc(0x10000));
		//for(i=0;i<1000000;i++);
		//__asm__ __volatile("int $0x80");
	}
	kprintf("kernel panic: exceution continuing beyond _kernel_main address space!");
}
