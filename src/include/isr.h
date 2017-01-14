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

// isr.h

#ifndef __KTUX_ISR_H
#define __KTUX_ISR_H

#include <interrupts.h>					// idtr_struct

extern idtr_struct *idtPointer;

// Exception handlers
extern void x86ExceptionHandler0(void);
extern void x86ExceptionHandler1(void);
extern void x86ExceptionHandler2(void);
extern void x86ExceptionHandler3(void);
extern void x86ExceptionHandler4(void);
extern void x86ExceptionHandler5(void);
extern void x86ExceptionHandler6(void);
extern void x86ExceptionHandler7(void);
extern void x86ExceptionHandler8(void);
extern void x86ExceptionHandler9(void);
extern void x86ExceptionHandlerA(void);
extern void x86ExceptionHandlerB(void);
extern void x86ExceptionHandlerC(void);
extern void x86ExceptionHandlerD(void);
extern void x86ExceptionHandler10(void);
extern void x86ExceptionHandler11(void);
extern void x86ExceptionHandler12(void);
extern void x86ExceptionHandlerUnimp(void);

// Page fault handler
extern void x86PageFault(void);

// IRQ handlers
extern void x86InterruptHandler20(void);
extern void x86InterruptHandler21(void);

// Other handlers
extern void syscallHandler(void);
extern void x86GeneralExceptionHandler(void);

#endif
