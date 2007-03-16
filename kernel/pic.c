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

// pic.c - PIC driver

#include <ktux.h>
#include <pic.h>
#include <interrupts.h>
#include <stdio.h>
#include <io.h>

extern int idtInitialized;

int init_pic(void)
{
	// Remap our PICs
	remap_pics(IRQ0_INT, IRQ8_INT);

	// Just disable interrupts for now
	set_irqs(~0x00, ~0x00);		// Disable them all

	return 0;
}

void remap_pics(unsigned int irq0Int, unsigned int irq8Int)
{
	outportb(PIC1, ICW1);
	outportb(PIC2, ICW1);
	
	outportb(PIC1+1, irq0Int);
	outportb(PIC2+1, irq8Int);

	outportb(PIC1+1, 4);
	outportb(PIC2+1, 2);

	outportb(PIC1+1, ICW4);
	outportb(PIC2+1, ICW4);
}

void enable_irq(unsigned int irqNum)
{
	if (!idtInitialized) {
		panic(false, "kernel error: set_irqs() called without initialization");
		return;
	}

	if (irqNum < 8)
		outportb(PIC1+1, inportb(PIC1+1) & ~(1 << irqNum));
	else if (irqNum < 15)
	{
		outportb(PIC1+1, inportb(PIC1+1) & ~0x04);
		outportb(PIC2+1, inportb(PIC2+1) & ~(1 << irqNum));
	}
}

void disable_irq(unsigned int irqNum)
{
	if (!idtInitialized) {
		panic(false, "kernel error: set_irqs() called without initialization");
		return;
	}

	if (irqNum < 8)
		outportb(PIC1+1, inportb(PIC1+1) | (1 << irqNum));
	else if (irqNum < 15)
	{
		outportb(PIC1+1, inportb(PIC1+1) & ~0x04);
		outportb(PIC2+1, inportb(PIC2+1) | (1 << irqNum));
	}
}

// Enable IRQs, pass 8-bit mask to function
// Will enable IRQ2 if needed
void set_irqs(int mask825921, int mask8259a1)
{
	if (!idtInitialized) {
		panic(false, "kernel error: set_irqs() called without initialization");
		return;
	}

	// Enable IRQ2 cascade to 2nd 8259 chip if needed
	if (mask8259a1 < 0xFF)
		mask825921 &= ~0x04;

	outportb(PIC1+1, mask825921);
	outportb(PIC2+1, mask8259a1);
}
