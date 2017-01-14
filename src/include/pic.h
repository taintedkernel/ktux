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

// pic.h

#ifndef __KTUX_PIC_H
#define __KTUX_PIC_H

// PIC ports/data
#define PIC1			0x20
#define PIC2			0xA0
#define ICW1			0x11
#define ICW4			0x01

#define cli() __asm__ __volatile__ ("cli");
#define sti() __asm__ __volatile__ ("sti");
#define hlt() __asm__ __volatile__ ("hlt");
//#define do_int(num) __asm__ __volatile__("int %0"::"r" (num))

#define load_gdt(dtr) __asm__ __volatile("lgdt %0"::"m" (*dtr))
#define load_idt(dtr) __asm__ __volatile("lidt %0"::"m" (*dtr))
#define load_tr(tr) __asm__ __volatile("ltr %0"::"mr" (tr))
#define load_ldt(ldt) __asm__ __volatile("lldt %0"::"mr" (ldt))

// function prototypes
int init_pic(void);
void remap_pics(unsigned int irq0Int, unsigned int irq8Int);
int enable_irq(unsigned int irqNum);
int disable_irq(unsigned int irqNum);
int toggle_irq(unsigned int);
void set_irqs(int mask82590, int mask82591);

#endif
