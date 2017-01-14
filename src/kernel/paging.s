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

	; Our functions
	GLOBAL invalidate
	GLOBAL reload_cr3
	GLOBAL read_cr0
	GLOBAL write_cr0
	GLOBAL read_cr3
	GLOBAL write_cr3
	GLOBAL enable_paging
	;GLOBAL pageBuffer

; Utility functions
invalidate:
	push	ebp
	mov	ebp, esp
	invlpg	[ebp+8]
	pop	ebp
	ret

reload_cr3:
	push	eax
	call	read_cr3
	push	eax
	call	write_cr3
	pop		eax
	pop		eax
	ret

read_cr0:
	mov	eax, cr0
	ret

write_cr0:
	push	ebp
	mov	ebp, esp
	mov	eax, [ebp+8]
	mov	cr0, eax
	pop	ebp
	ret

read_cr3:
	mov	eax, cr3
	ret

write_cr3:
	push	ebp
	mov	ebp, esp
	mov	eax, [ebp+8]
	mov	cr3, eax
	pop	ebp
	ret

;
; Enable paging function
; Arguments - Address of kernel page directory
;
; Enables paging and updates segment registers with new selectors (base=0)
; Selector update is hackish since we can't access the variable
; pointers directly (stored in ../osloader/klMain.s).
;
; We know that there is a temporary code & data segment selector
; with modified base address to work with kernel virtual address
; until this function is called.  Directly following that is our
; permanent selectors with base=0, so just update CS,DS,ES,etc
; with the same segment + 0x10, jumping the temporary selectors
;
enable_paging:
	push	ebp
	mov	ebp, esp
	mov	eax, [ebp+8]
	
	; Set page directory
	mov	cr3, eax

	; Enable paging
	mov	eax, cr0
	or	eax, 0x80000000
	mov	cr0, eax

	; Update selectors
	mov	ecx, cs
	add	ecx, 0x10

	; Update CS + EIP simultaneously
	push	ecx
	push	updatedCS
	retf

	; Update rest of segment registers
updatedCS:
	mov	edx, ds
	add	edx, 0x10
	mov	ds, edx
	mov	es, edx
	mov	fs, edx
	mov	gs, edx

	pop	ebp
	ret
	


;	SEGMENT .data
;	ALIGN 4096
;
;pageBuffer		times 4096 db 0	