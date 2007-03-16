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

; Functions this source file provides
	GLOBAL kloaderInitConsole
	GLOBAL kloaderClearScreen
	GLOBAL kloaderPrintSpace
	GLOBAL kloaderPrintCrLf
	GLOBAL kloaderPrintString
	GLOBAL kloaderPrintRegisters
	GLOBAL kloaderPrintEBP16

; Global variables (data) this file provides
	;GLOBAL

; External functions this source file accesses
	;EXTERN


	[BITS 16]
	SECTION .text

	%include "kloader.h"


;*******************************************************************************
; kloaderInitConsole: Initializes console
; Input: None
; Output: None
; Scope: Global
;*******************************************************************************
kloaderInitConsole:

	call	kloaderClearScreen
	
	; For now just get current cursor location from BIOS
;	mov	ax, 14
;	mov	dx, BIOS_CURSOR_PORT1
;	out	dx, al
;	mov	dx, BIOS_CURSOR_PORT2
;	in	al, dx
;	shl	ax, 8
;	mov	[textCursorPointer], ax
;	
;	mov	ax, 15
;	mov	dx, BIOS_CURSOR_PORT1
;	out	dx, al
;	mov	dx, BIOS_CURSOR_PORT2
;	in	al, dx
;	or	[textCursorPointer], ax

	; Bochs hack (line 15, 1st char)
	;mov	[textCursorPointer], word 0

	ret


;*******************************************************************************
; kloaderClearScreen: Clears screen
; Input: None
; Output: None
; Scope: Global
;*******************************************************************************
kloaderClearScreen:
	push	ax
	push	cx
	push	es
	push	di

	; Destination pointers
	mov	ax, (TEXT_SCREENSTART / 16)
	mov	es, ax
	mov	di, 0

	; Clear the screen
	mov	cx, (TEXT_NUMCOLUMNS * TEXT_NUMCOLUMNS)
	mov	al, 0x20				; Little endian byte ordering
	mov	ah, TEXT_WHITE
	rep	stosw

	; Set cursor
	mov	word [textCursorPointer], 0

	pop	di
	pop	es
	pop	cx
	pop	ax
	ret


;*******************************************************************************
; kLoaderScrollLine: Scrolls console screen up one line
; Input: None
; Scope: Local
;*******************************************************************************
kloaderScrollLine:
	push	ax
	push	cx
	push	ds
	push	si
	push	es
	push	di

	; Set up destination pointers
	mov	ax, (TEXT_SCREENSTART / 16)
	mov	ds, ax
	mov	es, ax
	mov	si, (TEXT_NUMCOLUMNS*2)
	mov	di, 0

	; Scroll up one line
	mov	cx, (TEXT_NUMROWS-1) * TEXT_NUMCOLUMNS / 2
	rep	movsd				; Move DS:SI++ to ES:DI++ (dword)

	; Clear last line
	mov	cx, TEXT_NUMCOLUMNS
	xor	al, al				; Little endian byte ordering
	mov	ah, TEXT_WHITE
	rep	stosw

	; Set cursor
	mov	word [textCursorPointer], (TEXT_NUMROWS-1)*TEXT_NUMCOLUMNS*2

	pop	di
	pop	es
	pop	si
	pop	ds
	pop	cx
	pop	ax
	ret


;*******************************************************************************
; kLoaderPrintSpace: Print a space to console at current cursor location.
; Input: None
; Scope: Global
;*******************************************************************************
kloaderPrintSpace:
	push	ax				; Save registers we mangle
	push	es
	push	di

	; Set up destination pointers
	mov	ax, (TEXT_SCREENSTART / 16)
	mov	es, ax
	mov	di, [textCursorPointer]

	mov	al, 32				; Print space (ASCII char 32)
	mov	ah, TEXT_WHITE
	stosw					; Little endian byte ordering

	; Check bounds
	cmp	di, (TEXT_NUMROWS*TEXT_NUMCOLUMNS*2)
	jge	.scrollUp
	jmp	.stopPrint

.scrollUp:
	; Scroll up a line
	call	kloaderScrollLine
	mov	di, (TEXT_NUMROWS-1)*TEXT_NUMCOLUMNS*2	; Move to 1st char, 25th line
	
.stopPrint:
	; Update cursor location
	mov	[textCursorPointer], di

	pop	di
	pop	es
	pop	ax
	ret


;*******************************************************************************
; kLoaderPrintCrLf: Print a carriage return/line feed combo to the screen
; Input: None
; Scope: Global
;*******************************************************************************
kloaderPrintCrLf:
	push	ax			; Save registers we mangle
	push	bx
	push	es
	push	di

	; Set up destination pointers
	mov	ax, (TEXT_SCREENSTART / 16)
	mov	es, ax
	mov	di, [textCursorPointer]

	; Do a CR
	mov	bl, TEXT_NUMCOLUMNS*2	; 2 bytes/char * 80 chars/line
	mov	ax, di
	div	bl			; di/160
	movzx	bx, ah			; Save remainder
	mov	ax, di
	sub	ax, bx			; Subtract remainder
	mov	di, ax			; Reflect changes

	; Do a LF
	add	di, TEXT_NUMCOLUMNS*2	; Add 160 (2 bytes/char * 80 chars/line)

	; Check bounds
	cmp	di, (TEXT_NUMROWS*TEXT_NUMCOLUMNS*2)
	jge	.scrollUp
	jmp	.stopPrint

.scrollUp:
	; Scroll up a line
	call	kloaderScrollLine
	mov	di, (TEXT_NUMROWS-1)*TEXT_NUMCOLUMNS*2	; Move to 1st char, 25th line
	
.stopPrint:
	; Update cursor location
	mov	[textCursorPointer], di

	pop	di			
	pop	es
	pop	bx
	pop	ax
	ret


;*******************************************************************************
; kLoaderPrintString: Print a string to console at current cursor location.
; Input: String @ DS:SI, color=DX (DL w/ DH zeroed)
; Modifies: SI
; Scope: Global
;*******************************************************************************
kloaderPrintString:
	push	ax			; Save registers we mangle
	push	bx
	push	cx
	push	es
	push	di

	; Get length of string
	xor	cx, cx
	push	si
.getStringLen:
	lodsb				; Load AL with DS:SI++
	or	al, al
	jz	.gotLength
	inc	cx
	jmp	.getStringLen
.gotLength:
	pop	si

	; Set up destination pointers
	mov	ax, (TEXT_SCREENSTART / 16)
	mov	es, ax
	mov	di, [textCursorPointer]

.characterLoop:
	lodsb				; Move byte at DS:SI++ to AL
	cmp	ax, msgCr
	je	.doCr
	cmp	ax, msgLf
	je	.doLf
	mov	ah, dl			; Move attribute byte into AH
	stosw				; Put AX at ES:DI++ (little endian)

	; Check bounds
.checkBounds:
	cmp	di, (TEXT_NUMROWS*TEXT_NUMCOLUMNS*2)
	jge	.scrollUp

	; Check remaining string length
.checkStringLen:
	dec	cx
	jnz	.characterLoop
	jmp	.stopPrint


	; These following 3 functions (sorta) handle ASCII control characters
.doCr:
	; Do a CR
	mov	bl, TEXT_NUMCOLUMNS*2	; 2 bytes/char * 80 chars/line
	mov	ax, di
	div	bl			; di/160
	movzx	bx, ah			; Save remainder
	mov	ax, di
	sub	ax, bx			; Subtract remainder
	mov	di, ax			; Reflect changes
	jmp	.checkBounds

.doLf:
	; Do a LF
	add	di, TEXT_NUMCOLUMNS*2	; Add 160 (2 bytes/char * 80 chars/line)
	jmp	.checkBounds

.scrollUp:
	; Scroll up a line
	call	kloaderScrollLine
	mov	di, (TEXT_NUMROWS-1)*TEXT_NUMCOLUMNS*2	; Move to 1st char, 25th line
	jmp	.checkStringLen


.stopPrint:
	; Update cursor location
	mov	[textCursorPointer], di

	pop	di
	pop	es
	pop	cx
	pop	bx
	pop	ax
	ret


;*******************************************************************************
; kloaderEBP2ASCII(16/32): Wrapper functions for kloaderEBP2ASCII
; Scope: Global (possibly)
;*******************************************************************************
kloaderPrintEBP32:
	mov	byte [textDataSize], 8
	call	kloaderPrintEBP
	ret

kloaderPrintEBP16:
	mov	byte [textDataSize], 4
	call	kloaderPrintEBP
	ret


;*******************************************************************************
; kloaderEBP2ASCII: Converts value in EBP (2 or 4 bytes) to ASCII string
;	and displays on screen at [textCursorPointer]
; Input:
;	EBP - Value to convert
;	Size of data value at [textDataSize] in characters (4=2 bytes or 8=4 bytes)
; Output: Contents of EBP at [textCursorPointer]
; Scope: Local
; Note: This was written for readability, not efficiency.  There are about a
;	a million ways of optimizing this, it's used for debugging only.
;*******************************************************************************
kloaderPrintEBP:
	push	eax
	push	bx
	push	ecx
	push	dx
	push	si

	; Check to see if [textDataSize] has valid value
	cmp	byte [textDataSize], 4
	je	.paramsOK
	cmp	byte [textDataSize], 8
	je	.paramsOK
	ret					; Error here: Incorrect data found

.paramsOK:
	; Do this one byte at a time (AL), breaking each
	; into 2 nibbles (DL:BL) and converting to ASCII character
	mov	eax, ebp
	movzx	ecx, byte [textDataSize]	; dword:7, word:3
	dec	ecx

.mainLoop:
	; Break AL into 2 digits, DL:BL
	mov	bl, al
	mov	dl, al

	; Work on BL (low nibble)
	and	bl, 00001111b			; bl=[0:15]
	cmp	bl, 9
	jg	.blIsLetter
	add	bl, 48
	jmp	.blDoneConvert
.blIsLetter:
	add	bl, 55
.blDoneConvert:
	mov	[ecx + textASCIIBuffer], bl
	dec	ecx

	; Now DL (high nibble)
	and	dl, 11110000b
	shr	dl, 4				; dl=[0:15]
	cmp	dl, 9
	jg	.dlIsLetter
	add	dl, 48
	jmp	.dlDoneConvert
.dlIsLetter:
	add	dl, 55
.dlDoneConvert:
	mov	[ecx + textASCIIBuffer], dl
	cmp	ecx, 0				; Are we finished?
	je	.doneConvert
	dec	ecx
	shr	eax, 8				; Shift to get next byte
	jmp	.mainLoop			; Repeat

.doneConvert:
	; Convert non-placeholding zeros into spaces
	mov	si, textASCIIBuffer
	movzx	ecx, byte [textDataSize]	; 8:dword, 4:word
.zero2Space:
	mov	al, [si]
	cmp	al, 48				; Is byte a zero (ASCII char 48)?
	jg	.checkSize			; If bigger than it's non-zero, go to next step
	mov	byte [si], 32			; Can't be less than so convert to space
	inc	si
	dec	ecx
	jnz	.zero2Space

	; If we are here, means entire ASCII buffer is all
	; blanks (contents of register were zero).  Put a
	; single zero in last place
	dec	si
	mov	byte [si], 48
	inc	si

.checkSize:
	; Check if we had a 2 or 4 byte data size
	; If it's only 2 bytes (4 chars), put null
	; at end of string to only display 4 characters
	cmp	byte [textDataSize], 8
	je	.printIt
	mov	byte [textASCIIBuffer + 4], 0

	; Print to screen
.printIt:
	mov	dx, TEXT_WHITE
	mov	si, textASCIIBuffer
	call	kloaderPrintString

	pop	si
	pop	dx
	pop	ecx
	pop	bx
	pop	eax
	ret


;*******************************************************************************
; kloaderPrintRegisters: Displays contents of all registers on screen
; Input: None
; Output: Contents of registers on screen
; Scope: Global
;*******************************************************************************
kloaderPrintRegisters:

	; Copy registers
	mov	[regEAX], eax
	mov	[regEBX], ebx
	mov	[regECX], ecx
	mov	[regEDX], edx
	mov	[regESI], esi
	mov	[regEDI], edi
	mov	[regEBP], ebp
	mov	[regESP], esp		; Do before any stack operations

	; EFLAGS needs special treatement
	push	eax
	pushfd
	pop	eax
	mov	[regEFL], eax
	pop	eax

	; Copy segments
	mov	[regCS], cs
	mov	[regDS], ds
	mov	[regES], es
	mov	[regFS], fs
	mov	[regGS], gs
	mov	[regSS], ss

	push	dx
	push	si
	push	ebp
	
	; Display segments/registers on screen
	mov	dx, TEXT_WHITE

	mov	si, msgEAX
	call	kloaderPrintString
	mov	ebp, [regEAX]
	call	kloaderPrintEBP32

	call	kloaderPrintSpace

	mov	si, msgEBX
	call	kloaderPrintString
	mov	ebp, [regEBX]
	call	kloaderPrintEBP32

	call	kloaderPrintSpace

	mov	si, msgECX
	call	kloaderPrintString
	mov	ebp, [regECX]
	call	kloaderPrintEBP32

	call	kloaderPrintSpace

	mov	si, msgEDX
	call	kloaderPrintString
	mov	ebp, [regEDX]
	call	kloaderPrintEBP32

	call	kloaderPrintCrLf

	mov	si, msgESI
	call	kloaderPrintString
	mov	ebp, [regESI]
	call	kloaderPrintEBP32

	call	kloaderPrintSpace

	mov	si, msgEDI
	call	kloaderPrintString
	mov	ebp, [regEDI]
	call	kloaderPrintEBP32

	call	kloaderPrintSpace

	mov	si, msgEBP
	call	kloaderPrintString
	mov	ebp, [regEBP]
	call	kloaderPrintEBP32

	call	kloaderPrintSpace

	mov	si, msgESP
	call	kloaderPrintString
	mov	ebp, [regESP]
	call	kloaderPrintEBP32

	call	kloaderPrintCrLf

	mov	si, msgEFL
	call	kloaderPrintString
	mov	ebp, [regEFL]
	call	kloaderPrintEBP32
	
	call	kloaderPrintCrLf
	mov	si, msgLine
	call	kloaderPrintString
	call	kloaderPrintCrLf

	mov	si, msgCS
	call	kloaderPrintString
	mov	ebp, [regCS]
	call	kloaderPrintEBP16

	call	kloaderPrintSpace
	call	kloaderPrintSpace

	mov	si, msgDS
	call	kloaderPrintString
	mov	ebp, [regDS]
	call	kloaderPrintEBP16

	call	kloaderPrintSpace
	call	kloaderPrintSpace

	mov	si, msgES
	call	kloaderPrintString
	mov	ebp, [regES]
	call	kloaderPrintEBP16

	call	kloaderPrintSpace
	call	kloaderPrintSpace

	mov	si, msgFS
	call	kloaderPrintString
	mov	ebp, [regFS]
	call	kloaderPrintEBP16

	call	kloaderPrintSpace
	call	kloaderPrintSpace

	mov	si, msgGS
	call	kloaderPrintString
	mov	ebp, [regGS]
	call	kloaderPrintEBP16

	call	kloaderPrintSpace
	call	kloaderPrintSpace

	mov	si, msgSS
	call	kloaderPrintString
	mov	ebp, [regSS]
	call	kloaderPrintEBP16

	call	kloaderPrintCrLf
	
	pop	ebp
	pop	si
	pop	dx
	ret



;*******************************************************************************
;*                              DATA SEGMENT                                   *
;*******************************************************************************

	SECTION .data

textCursorPointer	dw 0
textASCIIBuffer		times 9 db 0	; 2 bytes/nibble * 4 byte register + null
textDataSize		db 0

; Copy of data in registers
regEAX		dd 0
regEBX		dd 0
regECX		dd 0
regEDX		dd 0
regESI		dd 0
regEDI		dd 0
regESP		dd 0
regEBP		dd 0
regEFL		dd 0
regCS		dw 0
regDS		dw 0
regES		dw 0
regFS		dw 0
regGS		dw 0
regSS		dw 0
regEIP		dd 0

; Local messages
msgEAX		db "EAX ", 0
msgEBX		db "EBX ", 0
msgECX		db "ECX ", 0
msgEDX		db "EDX ", 0
msgESI		db "ESI ", 0
msgEDI		db "EDI ", 0
msgESP		db "ESP ", 0
msgEBP		db "EBP ", 0
msgEFL		db "FLG ", 0
msgLine		db "----------------------------------------------------", 0
msgCS		db "CS ", 0
msgDS		db "DS ", 0
msgES		db "ES ", 0
msgFS		db "FS ", 0
msgGS		db "GS ", 0
msgSS		db "SS ", 0

msgText		db msgCr, msgLf, "1", msgCr, msgLf, "2", msgCr, msgLf, "3", msgCr, msgLf, "4"
		db msgCr, msgLf, "5", msgCr, msgLf, "6", msgCr, msgLf, "7", msgCr, msgLf, "8"
		db msgCr, msgLf, "9", msgCr, msgLf, "10", msgCr, msgLf, "11", msgCr, msgLf, "12"
		db msgCr, msgLf, "13", msgCr, msgLf, "14", msgCr, msgLf, "15", msgCr, msgLf, "16"
		db msgCr, msgLf, "17", msgCr, msgLf, "18", msgCr, msgLf, "19", msgCr, msgLf, "20"
		db msgCr, msgLf, "21", msgCr, msgLf, "22", msgCr, msgLf, "23", msgCr, msgLf, "24"
		db msgCr, msgLf, "25", 0
