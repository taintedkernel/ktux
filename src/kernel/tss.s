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
	SEGMENT .data
	
	GLOBAL tss
	GLOBAL tss_esp0
	GLOBAL tss_ss0

tss:
backlink	dw 0,0

tss_esp0:
		dd 0					; esp0
tss_ss0:
		dw 0x20, 0				; ss0		KERNEL_DATA_SELECTOR ../include/selectors.h
		dd 0					; esp1
		dw 0, 0					; ss1
		dd 0					; esp2
		dw 0, 0					; ss2
		dd 0					; cr3
		dd 0					; eip
		dd 0					; eflags
		dd 0					; eax
		dd 0					; ecx
		dd 0					; edx
		dd 0					; ebx
		dd 0					; esp
		dd 0					; ebp
		dd 0					; esi
		dd 0					; edi
		dw 0, 0					; es
		dw 0, 0					; cs
		dw 0, 0					; ss
		dw 0, 0					; ds
		dw 0, 0					; fs
		dw 0, 0					; gs
		dw 0, 0					; idt
		dw 0					; debug
		dw 0					; ioperm


