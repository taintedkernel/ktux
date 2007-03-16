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

// interrupts.c

#include <ktux.h>
#include <interrupts.h>		// this source
#include <selectors.h>
#include <isr.h>

// dependencies
#include <pic.h>
#include <io.h>				// outportb()
#include <stdio.h>			// panic()
#include <error.h>

int idtInitialized = false;
idtr_struct IDTR;
idt_entry_struct IDT[256];

// Load our standard exceptions and null for ones we will not provide
int init_interrupts()
{
	int i, status = true;

	void *idtExceptionList[] = {
		x86ExceptionHandler0,
		x86ExceptionHandler1,
		x86ExceptionHandler2,
		x86ExceptionHandler3,
		x86ExceptionHandler4,
		x86ExceptionHandler5,
		x86ExceptionHandler6,
		x86ExceptionHandler7,
		x86ExceptionHandler8,
		x86ExceptionHandler9,
		x86ExceptionHandlerA,
		x86ExceptionHandlerB,
		x86ExceptionHandlerC,
		x86ExceptionHandlerD,
		x86PageFault,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandler10,
		x86ExceptionHandler11,
		x86ExceptionHandler12,
	};

	// Array should contain MAX_IRQ_IMP elements
	void *idtIRQList[] = {
		x86InterruptHandler20,
		x86InterruptHandler21,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp,
		x86ExceptionHandlerUnimp
	};

	if (idtInitialized)
		return ERR_ALREADY_INIT;

	// Initialize all interrupts to unimplemented
	// handler then add the ones we actually handle
	for (i=0; i<NUM_INTS; i++)
	{
		status = set_idt_entry(i, x86ExceptionHandlerUnimp, 0);
		if (!status)
			return status;
	}

	// Add x86 standard exceptions
	for (i=0; i<NUM_STD_EXP; i++)
	{
		status = set_idt_entry(i, idtExceptionList[i], 0);
		if (!status)
			return status;
	}

	// Add our interrupts
	for (i=0; i<MAX_IRQ_IMP; i++)
	{
		status = set_idt_entry(IRQ0_INT + i, idtIRQList[i], 0);
		if (!status)
			return status;
	}
	
	set_idt_entry(INT_SYSCALL, syscallHandler, 0);

	idtInitialized = true;
	load_idtr();

	return status;
}

// Sets an entry to our IDT
int set_idt_entry(unsigned int vectorNum, void (*intHandler)(), unsigned int dpl)
{
	unsigned short settings;
	unsigned int offset;
	//unsigned short selector;

	if (vectorNum > 0xFF)
		return ERR_BOUNDS_EXCEED;

	if (intHandler == NULL)
		return ERR_BOUNDS_EXCEED;

	// This must be a proper assembly stub (complete w/IRET - C functions do not work!)
	offset = (unsigned int)intHandler;

	switch (dpl)
	{
	case 0: settings = DPL_0; break;
	case 1:
	case 2:
	case 3:
	default: settings = DPL_3;
	}
	
	//asm volatile("movw %%cs, %0" :"=g"(selector));
	IDT[vectorNum].offsetLow = offset & 0xFFFF;
	IDT[vectorNum].selector = KERNEL_CODE_SELECTOR;
	IDT[vectorNum].settings = settings;
	IDT[vectorNum].offsetHigh = offset >> 16;

	return true;
}

// Loads the IDT
void load_idtr()
{
	if (!idtInitialized)
		return;

	// Calculate limit and base of IDT
    IDTR.limit = 0x100 * (sizeof(idt_entry_struct)-1);
    IDTR.base = IDT;

	// Load IDTR
    asm volatile("lidt (%0) ": :"r" (&IDTR));
}

void reboot(void)
{
	unsigned temp;

	cli();
/* flush the keyboard controller */
	do
	{
		temp = inportb(0x64);
		if((temp & 0x01) != 0)
		{
			(void)inportb(0x60);
			continue;
		}
	} while((temp & 0x02) != 0);
/* pulse the CPU reset line */
	outportb(0x64, 0xFE);
/* ...and if that didn't work, just halt */
	hlt();
}

// Our C interrupt/exception handler
void exception_handler(int_stack_regs_struct *intRegs)
{
	// Dump CPU state to console
	kprintf("\nunhandled exception %d at %p:%p\n",
		intRegs->interruptNum, intRegs->cs, intRegs->eip);
	kprintf("EAX=%08X EBX=%08X ECX=%08X EDX=%08X\n",
		intRegs->eax, intRegs->ebx, intRegs->ecx, intRegs->edx);
	kprintf("ESI=%08X EDI=%08X EBP=%08X ESP=%08X\n",
		intRegs->esi, intRegs->edi, intRegs->ebp, intRegs->esp);
	kprintf(" DS=%04X  ES=%04X  FS=%04X  GS=%04X\n",
		intRegs->ds, intRegs->es, intRegs->fs, intRegs->gs);
	kprintf(" EFLAGS=%08X UNKNOWN=%08X\n", intRegs->eflags, 0); //intRegs->temp);

	// Handle it
	switch(intRegs->interruptNum)
	{
	case 0x0:
		panic(false, "kernel panic: divide by zero error!");
		break;
	case 0x1:
		panic(false, "kernel panic: debug error!");
		break;
	case 0x2:
		panic(false, "kernel panic: NMI interrupt!");
		break;
	case 0x3:
		panic(false, "kernel panic: breakpoint reached!");
		break;
	case 0x4:
		panic(false, "kernel panic: overflow!");
		break;
	case 0x5:
		panic(true, "kernel panic: bound range exceeded!");
		break;
	case 0x6:
		panic(true, "kernel panic: invalid opcode!");
		break;
	case 0x7:
		panic(true, "kernel panic: device not available!");
		break;
	case 0x8:
		panic(true, "kernel panic: double fault!");
		break;
	case 0x9:
		panic(false, "kernel panic: coprocessor segment overrun!");
		break;
	case 0xA:
		panic(false, "kernel panic: invalid tss!");
		break;
	case 0xB:
		panic(false, "kernel panic: segment not present!");
		break;
	case 0xC:
		panic(false, "kernel panic: stack segment fault!");
		break;
	case 0xD:
		panic(false, "kernel panic: general protection fault!");
		break;
	case 0xE:		// Should be handled by our page fault handler, not here
		panic(false, "kernel panic: page fault");
		break;
	case 0xF:
		panic(true, "kernel panic: intel reservered interrupt called!");
		break;
	case 0x10:
		panic(false, "kernel panic: fpu floating-point error");
		break;
	case 0x11:
		panic(false, "kernel panic: alignment check!");
		break;
	case 0x12:
		panic(true, "kernel panic: machine check!");
		break;
	case 0x13:
		panic(false, "kernel panic: simd floating-point");
		break;
	case INT_UNHANDLED:
		panic(true, "kernel panic: unhandled interrupt caught");
		break;
	default:
		panic(true, "kernel panic: unknown interrupt caught");
		break;
	}
	
	//while(1);
}
