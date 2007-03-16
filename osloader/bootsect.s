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

; bootsect.asm
; Bootloader for Ktux
; Written by Anthony DeChiaro - 2/19/04
; v0.01

; x86 Lower 1mb Memory Map (Real Mode)
;
; 0x00000 - 0x003FF	IVT
; 0x00400 - 0x004FF	BDA
; 0x00500 - 0x00600	[GDT]

; 0x00600 - 0x07A00	=Available=

; 0x07A00 - 0x07C00 [Stack]
; 0x07C00 - 0x07E00	[Bootsector]

; 0x07E00 - 0x08000	=Available=

; 0x08000 - 0xA0000	[Kernel Loader]

; 0x9FC00 - 0x9FFFF	EBDA
; 0xA0000 - 0xB7FFF	Video RAM (VGA Buffer)
; 0xB8000 - 0xBFFFF	Video RAM (Text Buffer)
; 0xC0000 - 0xC7FFF	Video BIOS
; 0xC8000 - 0xEFFFF	ROM BIOS/BIOS Shadow
; 0xF0000 - 0xFFFFF	Base/System ROM

; 0x100000		[Kernel Proper]

;
; Ktux bootsector will use the following:
;
; 0x07A00 - 0x07B99 Stack (in real mode)
; 0x01000 - 0x9FFFF Kernel loader
; 0x100000 - ?      Kernel
;
; Kernel size will be calculated at compile
; time and hardcoded into bootsector

%include "kloader.h"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


	[BITS 16]
	ORG 0x7C00
	SEGMENT .text

codeStart:
	jmp	boot

fat12BPB:					; 00 - 02 jump instruction
OEMName			db 'Ktux    '		; 03 - 0A OEM Name
BytesPerSector		dw 0200h		; 0B - 0C Bytes per sector
SectorsPerCluster	db 01h			; 0D - 0D Sectors per cluster
ReservedSectorCount	dw 0001h		; 0E - 0F Reserved sectors
NumberFATs		db 02h 			; 10 - 10 Copies of FAT
MaxRootDirEntryCount	dw 00E0h		; 11 - 12 Max. rootdir entries
TotalSectors16		dw 0B40h		; 13 - 14 Sectors in logical image
Media			db 0F0h	     		; 15 - 15 Media descriptor byte
SectorsPerFAT		dw 09h			; 16 - 17 Sectors in FAT
SectorsPerTrack		dw 0012h		; 18 - 19 Sectors per track
NumberHeads		dw 0002h		; 1A - 1B Number of heads
HiddenSectors		dd 00000000h		; 1C - 1D Number of hidden sectors
TotalSectors32		dd 00000B40h    	; 1D - 20 Real number of sectors
DriveNumber		db 00h			; 21 - 21 BIOS dr. #
Reserved1		db 00h 			; 22 - 22 ?
BootSignature		db 29h          	; 23 - 23 Signature
VolumeID		dd 00000001h    	; 24 - 27 Volume ID
VolumeName		db 'Ktux       '	; 28 - 42 Volume name
FSType			db 'FAT12   '   	; 43 - 4A Filesystem type

boot:	
	; Set up a stack
	cli
	mov	ax, TO_SEGMENT(STACK_SEGMENT)
	mov	ss, ax
	mov	sp, STACK_OFFSET
	sti

	; Set segment registers
	xor	ax, ax
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax

	; Save boot drive
	mov	[bootDrive], dl

; Reset drive we booted from (floppy?)
.resetDrive:
	xor	ah, ah							; Reset drive before we try reading from it
	int	0x13
	or	ah, ah
	jnz	.resetDrive

	mov	si, msgK
	call	PrintString

; Calculate size of root directory, put in 'cx'
.loadRootDir:
	xor	cx, cx
	xor	dx, dx
	mov	ax, FAT_ROOTDIR_ENTRYSIZE
	mul	word [MaxRootDirEntryCount]
	div	word [BytesPerSector]
	xchg	ax, cx
	
; Calculate location of root directory, put in 'ax'
	mov	al, byte [NumberFATs]
	mul	word [SectorsPerFAT]
	add	ax, word [ReservedSectorCount]

; Read root directory into memory
	mov	word [firstDataSector], ax
	add	word [firstDataSector], cx
	mov	bx, ROOTDIR_BUFFER
	call	ReadSectors

; Browse root directory for binary
	mov	cx, word [MaxRootDirEntryCount]
	mov	di, ROOTDIR_BUFFER
.loop:
	push cx
	mov	cx, FAT_FILENAME_SIZE			; Compare 11 characters (8+3)
	mov	si, bootstubFile				; Bootstub image filename
	push	di
	rep	cmpsb
	pop	di
	je	loadFAT
	pop	cx
	add	di, FAT_ROOTDIR_ENTRYSIZE		; Jump to next directory entry
	loop	.loop
	jmp	FatalError

loadFAT:
	mov	si, msgT
	call	PrintString

; Save starting cluster of boot image
	mov	dx, word [di + FAT_ROOTENTRY_START_CLUSTER]
	mov	word [cluster], dx				; First cluster of file
	
; Calculate size of FAT, put in 'cx'
	xor	ax, ax
	mov	al, byte [NumberFATs]
	mul	word [SectorsPerFAT]
	mov	cx, ax

; Calculate location of FAT, put in 'ax'
	mov	ax, [ReservedSectorCount]
	
; Read FAT into memory
	mov	bx, FAT_BUFFER
	call	ReadSectors
	
	mov	si, msgU
	call	PrintString

; Read bootstub image into memory
	mov	bx, BOOTSTUB_ADDRESS
	push	bx
.loadImage:
	mov	ax, word [cluster]
	pop	bx
	call	Cluster2LBA
	xor	cx, cx
	mov	cl, byte [SectorsPerCluster]
	call	ReadSectors
	push	bx

; Calculate next cluster
	mov	ax, word [cluster]
	mov	cx, ax
	mov	dx, ax
	shr	dx, 0x01
	add	cx, dx							; cluster * (3/2)
	mov	bx, FAT_BUFFER
	add	bx, cx
	mov	dx, word [bx]					; Read two bytes from FAT
	test	ax, 0x01
	jnz	.oddCluster
.evenCluster:
	and	dx, 0000111111111111b			; Lower 12 bits
	jmp	.done
.oddCluster:
	shr	dx, 0x04						; Upper 12 bits
.done:
	mov	word [cluster], dx				; Store new cluster
	cmp	dx, 0x0FF0
	jb	.loadImage

bootstubLoaded:
	mov	si, msgX
	call	PrintString
	
	; Pass boot drive & filesystem info
	push	fat12BPB
	push	word FILESYS_FAT12
	push	word [bootDrive]

	; Jump to our bootstub
	jmp	0x00:BOOTSTUB_ADDRESS


; We've reached a fatal error, so just halt
; Had to put this before functions due to range of short jumps
FatalError:
	mov	si, msgFatalError
	call	PrintString

	hlt
	jmp	$


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ReadSectors: Reads sectors from disk into memory
; Input: AX=starting sector, CX=sector count, ES:BX=memory buffer 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ReadSectors:
.main:
	mov	di, 0x5
.sectorloop:
	push	ax
	push	bx
	push	cx
	call	LBA2CHS
	mov	ah, 0x2
	mov	al, 0x1
	mov	ch, byte [track]
	mov	cl, byte [sector]
	mov	dh, byte [head]
	mov	dl, byte [bootDrive]
	int	0x13
	jnc	.success
	xor	ax, ax
	int	0x13
	dec	di
	pop	cx
	pop	bx
	pop	ax
	jnz	.sectorloop
	int	0x18
.success:
	; Update progress
	mov	si, msgDot
	call	PrintString
	pop	cx
	pop	bx
	pop	ax
	add	bx, word [BytesPerSector]
	inc	ax
	loop	.main
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; LBA2CHS: Converts LBA addressing scheme to CHS values
; Input: AX=LBA value
; Output: variables populated=sector, head, track
; Algorithm:
;	sector = (LBA / sectors_per_track) + 1
;	head = (LBA / sectors_per_track) % number_of_heads
;	track = LBA / (sectors_per_track * number_of_heads)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LBA2CHS:
	xor	dx, dx
	div	word [SectorsPerTrack]
	inc	dl
	mov	byte [sector], dl
	xor	dx, dx
	div	word [NumberHeads]
	mov	byte [head], dl
	mov	byte [track], al
	ret
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Cluster2LBA: Converts FAT cluster number to LBA addressing scheme
; Input: AX=cluster number
; Output: AX=LBA value
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Cluster2LBA:
	sub	ax, 0x02						; Clusters start at 2, so subtract
	mul byte [SectorsPerCluster]
	add	ax, word [firstDataSector]
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PrintString: Prints string on the screen
; Input: DS:SI=null-terminated string
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PrintString:
	push	ax
	push	bx
	push	es
	mov	ah, 0x0e
.charLoop:
	lodsb					; Move byte from DS:SI++ to AL
	or	al, al				; Test for null
	jz	.donePrint
	int	0x10				; Print it
	jmp	.charLoop
.donePrint:
	pop	es
	pop	bx
	pop	ax
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DATA SEGMENT (bootsectors don't have seperate text/data segments) ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

bootDrive			dw 0
head				db 0
track				db 0
sector				db 0

firstDataSector		dw 0
cluster				dw 0

bootstubFile		db "KTUXLDR    "

msgK				db "K", 0
msgT				db "T", 0
msgU				db "U", 0
msgX				db "X", 0
msgDot				db ".", 0
msgFatalError		db "Fatal error!", 0

codeSize		equ $-codeStart
%if codeSize > 510
	%error "Code is too large to fit in boot sector"
%endif

times 510-codeSize	db 0		; Pad with zeros
			dw 0xAA55	; Boot signature
