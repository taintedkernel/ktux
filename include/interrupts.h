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

// interrupts.h - interrupt handler

#ifndef __KTUX_INTERRUPTS_H
#define __KTUX_INTERRUPTS_H

#define NUM_INTS		0xFF
#define NUM_STD_EXP		0x13
#define MAX_IRQ_IMP		0x15

#define INT_SYSCALL		0x80

// Interrupt 'number' for passed for unhandled interrupt
// Must have same value in isr.h
#define INT_UNHANDLED	0x100

// IRQ Interrupts
#define IRQ_ALL			0xFF
#define IRQ_TIMER		0
#define IRQ_KEYBOARD	1
#define IRQ_CASCADE		2
#define IRQ_COM2_4		3
#define IRQ_COM1_3		4
#define IRQ_LPT			5
#define IRQ_FLOPPY		6
#define IRQ_FREE7		7
#define IRQ_CLOCK		8
#define IRQ_FREE9		9
#define IRQ_FREE10		10
#define IRQ_FREE11		11
#define IRQ_PS2MOUSE	12
#define IRQ_COPROC		13
#define IRQ_IDE_1		14
#define IRQ_IDE_2		15

#define IRQ0_INT		0x20
#define IRQ1_INT		0x21
#define IRQ2_INT		0x22
#define IRQ3_INT		0x23
#define IRQ4_INT		0x24
#define IRQ5_INT		0x25
#define IRQ6_INT		0x26
#define IRQ7_INT		0x27
#define IRQ8_INT		0x28
#define IRQ9_INT		0x29
#define IRQ10_INT		0x2A
#define IRQ11_INT		0x2B
#define IRQ12_INT		0x2C
#define IRQ13_INT		0x2D
#define IRQ14_INT		0x2E
#define IRQ15_INT		0x2F

#define DPL_0			0x8E00	// 10001110 00000000
#define DPL_3			0xEE00	// 11101110 00000000


typedef unsigned short irqNumber;

// x86 idt interrupt struct
typedef volatile struct
{
	unsigned short offsetLow;
	unsigned short selector;
	unsigned short settings;
	unsigned short offsetHigh;
} __attribute__ ((packed)) idt_entry_struct;

// idtr_struct struct
typedef volatile struct
{
	unsigned short limit;
	idt_entry_struct *base;
} __attribute__ ((packed)) idtr_struct;

// stack register struct
typedef volatile struct
{
	// registers pushed individually
	unsigned int ds, es, fs, gs;

	// registers pushed via pusha
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;

	// interrupt number which occured
	unsigned int interruptNum;

	// registers pushed from x86 hardware handling of interrupt
	unsigned int eip;
	unsigned int cs;
	unsigned int eflags;
} __attribute__ ((packed)) int_stack_regs_struct;


int set_idt_entry(unsigned int number, void (*intHandler)(), unsigned int dpl);
int init_interrupts();
void exception_handler(int_stack_regs_struct *intRegs);
//void pageFaultHandler(unsigned long address);
void reboot(void);
void load_idtr(void);

#endif
