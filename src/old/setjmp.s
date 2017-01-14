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
	GLOBAL setjmp

setjmp:
	push ebx
		mov ebx,[8 + esp]

		mov [0 + ebx],edi	; buf->edi == 0(ebx) == EDI
		mov [4 + ebx],esi	; buf->esi == 4(ebx) == ESI
		mov [8 + ebx],ebp	; buf->ebp == 8(ebx) == EBP

		mov [20 + ebx],edx	; buf->edx == 20(ebx) == EDX
		mov [24 + ebx],ecx	; buf->ecx == 24(ebx) == ECX
		mov [28 + ebx],eax	; buf->eax == 28(ebx) == EAX

; use EBX value saved on stack; not the current value
		mov eax,[esp]
		mov [16 + ebx],eax	; buf->ebx == 16(ebx) == EBX

; use ESP value after RET; not the current value
		lea eax,[8 + esp]
		mov [12 + ebx],eax	; buf->esp == 32(ebx) == ESP

; use return address of this routine (EIP value saved on stack);
; not the current value
		;mov eax,[4 + esp]
		;mov [44 + ebx],eax	; buf->eip == 36(ebx) == EIP

; none of the PUSH or MOV instructions changed EFLAGS!
		pushf
		pop dword [36 + ebx]	; buf->eflags == 40(ebx) == EFLAGS
	pop ebx
	xor eax,eax
	ret
