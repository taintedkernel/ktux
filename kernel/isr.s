;***************************************************************************
;*   Copyright (C) 2004 by Anthony DeChiaro                                *
;*   axd6491@njit.edu                                                      *
;*                                                                         *
;*   This program is free software; you can redistribute it and/or modify  *
;*   it under the terms of the GNU General Public License as published by  *
;*   the Free Software Foundation; either version 2 of the License, or     *
;*   (at your option) any later version.                                   *
;*                                                                         *
;*   This program is distributed in the hope that it will be useful,       *
;*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
;*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
;*   GNU General Public License for more details.                          *
;*                                                                         *
;*   You should have received a copy of the GNU General Public License     *
;*   along with this program; if not, write to the                         *
;*   Free Software Foundation, Inc.,                                       *
;*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
;***************************************************************************

	[BITS 32]
	SEGMENT .text

	; Our exception handlers
	GLOBAL x86ExceptionHandler0
	GLOBAL x86ExceptionHandler1
	GLOBAL x86ExceptionHandler2
	GLOBAL x86ExceptionHandler3
	GLOBAL x86ExceptionHandler4
	GLOBAL x86ExceptionHandler5
	GLOBAL x86ExceptionHandler6
	GLOBAL x86ExceptionHandler7
	GLOBAL x86ExceptionHandler8
	GLOBAL x86ExceptionHandler9
	GLOBAL x86ExceptionHandlerA
	GLOBAL x86ExceptionHandlerB
	GLOBAL x86ExceptionHandlerC
	GLOBAL x86ExceptionHandlerD
	GLOBAL x86PageFault
	GLOBAL x86ExceptionHandlerF
	GLOBAL x86ExceptionHandler10
	GLOBAL x86ExceptionHandler11
	GLOBAL x86ExceptionHandler12
	GLOBAL x86ExceptionHandler13
	GLOBAL x86ExceptionHandler14
	GLOBAL x86ExceptionHandler15
	GLOBAL x86ExceptionHandler16
	GLOBAL x86ExceptionHandler17
	GLOBAL x86ExceptionHandler18
	GLOBAL x86ExceptionHandler19
	GLOBAL x86ExceptionHandler1A
	GLOBAL x86ExceptionHandler1B
	GLOBAL x86ExceptionHandler1C
	GLOBAL x86ExceptionHandler1D
	GLOBAL x86ExceptionHandler1E
	GLOBAL x86ExceptionHandler1F
	GLOBAL x86ExceptionHandlerUnimp

	; IRQ handlers
	GLOBAL x86InterruptHandler20
	GLOBAL x86InterruptHandler21

	; Other handlers
	GLOBAL syscallHandler

	GLOBAL asm_one
	GLOBAL asm_two

	; External C interrupt handlers
	EXTERN exception_handler
	EXTERN page_fault_handler
	EXTERN task_switch
	EXTERN kbd_irq_handler
	EXTERN task_switch_test


; Must have same value in interrupts.h
%define INT_UNHANDLED		0x100

; Macros for saving/restoring registers
%macro MACRO_REGS_SAVE 0
	pushad
	push	gs
	push	fs
	push	es
	push	ds
	mov	ebx, 0x20
	mov	ds, bx
	mov	es, bx
	mov	fs, bx
	mov	gs, bx
%endmacro

%macro MACRO_REGS_RESTORE 0
	mov	al, 0x20
	out	0xA0, al
	jmp	$+2
	jmp	$+2
	out	0x20, al
	jmp	$+2
	jmp	$+2
	pop	ds
	pop	es
	pop	fs
	pop	gs
	popad
%endmacro


; ********** INTERRUPT SERVICE ROUTINES **********
; Intel-defined exceptions for the x86
x86ExceptionHandler0:
	push	dword 0x0
	jmp	mainIsrStub

x86ExceptionHandler1:
	push	dword 0x1
	jmp	mainIsrStub

x86ExceptionHandler2:
	push	dword 0x2
	jmp	mainIsrStub

x86ExceptionHandler3:
	push	dword 0x3
	jmp	mainIsrStub

x86ExceptionHandler4:
	push	dword 0x4
	jmp	mainIsrStub

x86ExceptionHandler5:
	push	dword 0x5
	jmp	mainIsrStub

x86ExceptionHandler6:
	push	dword 0x6
	jmp	mainIsrStub

x86ExceptionHandler7:
	push	dword 0x7
	jmp	mainIsrStub

x86ExceptionHandler8:
	push	dword 0x8
	jmp	mainIsrStub

x86ExceptionHandler9:
	push	dword 0x9
	jmp	mainIsrStub

x86ExceptionHandlerA:
	push	dword 0xA
	jmp	mainIsrStub

x86ExceptionHandlerB:
	push	dword 0xB
	jmp	mainIsrStub

x86ExceptionHandlerC:
	push	dword 0xC
	jmp	mainIsrStub

x86ExceptionHandlerD:
	push	dword 0xD
	jmp	mainIsrStub

; Exception/Interrupt 14 is Page Fault
; Exception/Interrupt 15 is Intel-reserved

x86ExceptionHandler10:
	push	dword 0x10
	jmp	mainIsrStub

x86ExceptionHandler11:
	push	dword 0x11
	jmp	mainIsrStub

x86ExceptionHandler12:
	push	dword 0x12
	jmp	mainIsrStub

x86ExceptionHandlerUnimp:
	push	dword INT_UNHANDLED
	jmp	mainIsrStub

; ********** PAGE FAULT HANDLER **********
x86PageFault:
	cli
	xchg	bx,bx
	push	eax
	push	ebx
	MACRO_REGS_SAVE

	; Push pointer to register stack struct
	;mov	eax, esp
	;push	eax
	;
	; Call general C exception handler
	;call	exception_handler
	;pop	eax

	; Push pointer to old EIP that caused page fault
	mov	eax, [esp+52]
	push	eax

	; Push pointer to page fault error code
	mov eax, [esp+52]
	push	eax

	; Push pointer to offending address
	mov	eax, cr2
	push	eax

	call	page_fault_handler
	pop	eax
	pop eax
	pop	eax

	; Why is this needed?? *BUG*
	mov	ebx, [esp+48]
	mov	eax, [esp+52]
	mov	[esp+48], eax
	mov	eax, [esp+56]
	mov	[esp+52], eax
	mov	[esp+56], ebx

	MACRO_REGS_RESTORE
	sti
	iret

mainIsrStub:
	;xchg bx, bx
	MACRO_REGS_SAVE
	mov	eax, esp
	push	eax
	call	exception_handler
	;xchg bx, bx
	pop	eax
	MACRO_REGS_RESTORE
	add	esp, 4				; Pop exception number
	iret


; ********** IRQ HANDLERS **********
; IRQ 0 - Timer
x86InterruptHandler20:
	cli
	;xchg	bx, bx
	MACRO_REGS_SAVE

	; Copy user regs to process regs struct
	;call getCurrentTask
	;mov	edi, eax
	;lea	esi, [esp]
	;mov	ecx, 17
	;rep	movsd
	;add	esp, 68

	; Call task switcher
	mov		eax, esp
	push	eax
	call	task_switch
	mov		esp, eax
	;xchg	bx, bx

	;cmp		dword [eax+0xc], 0
	;jne		.cont

	; Update system TSS
	;mov	ebx, [eax+8]
	;mov	 [tss_esp0], ebx
;.cont:
	MACRO_REGS_RESTORE
	sti
	iret

; IRQ 1 - Keyboard
x86InterruptHandler21:
	cli
	MACRO_REGS_SAVE
	;xchg	bx, bx
	call	kbd_irq_handler
	MACRO_REGS_RESTORE
	sti
	iret

; Handles our OS/system calls
syscallHandler:
	push	dword 0x80
	jmp	mainIsrStub

asm_one:
	mov	eax, 1
	jmp	asm_one

asm_two:
	mov	eax, 2
	jmp	asm_two
