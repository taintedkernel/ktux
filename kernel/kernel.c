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
#include <list.h>
#include <debug.h>

systemInfo *thisSystem;

// We come in with interrupts disabled
void _kernel_main(void *bootParam)
{
	// First initialize RAM and enable paging
	init_virt_mem(bootParam);

	// After paging enabled, update pointers
	thisSystem = virt2Phys(bootParam);
	bootParam = NULL;			// NULL our old one

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
	kprintf("  [ OK ]\n");

	// Hardware
	kprintf("hardware .");
	init_pic();
	kprintf(".");
	init_timer();
	kprintf(".");
	init_keyboard();
	kprintf("\n  [ OK ]\n");

	kprintf("scheduler ...\n");
	init_multitasking();
	kprintf("  [ OK ]\n");

	// Remap our PICs, set IRQ0 to 18Hz, enable only IRQs 0&1, and turn on interrupts
	kprintf("Reprogramming timer and enabling IRQs...\n");

	init_shell();
	shell_prompt();

	reprogram_timer(1000);
	enable_irq(1);
	start_scheduler();
	sti();

	while(1) {
		kill();
		//DEBUG_BP
		//asm volatile("int $0x20");
	}

	kprintf("kernel panic: exceution continuing beyond _kernel_main address space!");
	while(1);		// one more final catch
}
