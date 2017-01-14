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
	
	GLOBAL memcpy
	
memcpy:
	
	push	ebp
	mov	ebp, esp
	mov eax, [ebp+8]
	mov ebx, [ebp+12]
	mov ecx, [ebp+16]
	mov	[ds:eax], '['
	inc edi
	mov	esi, eax
	mov	edi, ebx
	rep	movsd
	mov	[ds:esi], ']'
	pop	ebp
	ret