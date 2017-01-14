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
	GLOBAL kloaderLoadKernel

; Global variables (data) this file provides
	;GLOBAL

; External functions this source file accesses
	EXTERN kloaderFatalError
	EXTERN kloaderPrintString
	EXTERN kloaderPrintCrLf

; External variables this source file accesses
	EXTERN dataSel
	EXTERN SectorsPerTrack
	EXTERN NumberHeads

	EXTERN bootDrive
	EXTERN BytesPerSector
	EXTERN MaxRootDirEntryCount
	EXTERN SectorsPerCluster
	EXTERN SectorsPerFAT
	EXTERN ReservedSectorCount


	[BITS 16]
	SECTION .text

	%include "kloader.h"

; Standard C calling convention
kloaderLoadKernel:

	; Save 4 bytes for return code
	sub	sp, 0x04

	pusha
	mov	bp, sp

	; Save kernel load address
	mov	eax, dword [ss:(bp+24)]
	mov	[pKernelAddress], eax

	; Reset drive we booted from (floppy?)
	mov	dl, [bootDrive]
.resetDrive:
	xor	ah, ah
	int	0x13
	or	ah, ah
	jnz	.resetDrive

	; Compute volume info and load root directory into memory
	call	computeVolumeInfo
	call	loadRootDirectory

	; Check our return code
	cmp	ax, 0
	jge	.searchFile

	; Update progress (error)
	mov	dx, TEXT_WHITE
	mov	si, msgError
	call	kloaderPrintString
	call	kloaderPrintCrLf
	mov	si, msgLoadRootDirError
	call	kloaderPrintString
	call	kloaderPrintCrLf

	; Return -1 (failure)
	mov	dword [ss:(bp+16)], ERR_LOAD_ROOT
	jmp	.done

.searchFile:
	; Update progress
	mov	dx, TEXT_WHITE
	mov	si, msgDot
	call	kloaderPrintString

	; Push name of kernel file on stack (passed as parameter) and search for file
	push	word [ss:(bp+22)]
	call	findFile
	add	sp, 0x02

	; Did we find the file?
	cmp	ax, 0
	jge	.loadFAT

	; Update progress (error)
	mov	dx, TEXT_WHITE
	mov	si, msgError
	call	kloaderPrintString
	call	kloaderPrintCrLf
	mov	si, msgLoadFindError
	call	kloaderPrintString
	call	kloaderPrintCrLf

	; Couldn't find file, return -2
	mov	dword [ss:(bp+16)], ERR_NO_FILE
	jmp	.done

.loadFAT:
	; Update progress
	push	ax								; Save starting cluster of file
	mov	dx, TEXT_WHITE
	mov	si, msgDot
	call	kloaderPrintString

	; Load the FAT into memory
	call	loadFAT

	; Did we load the FAT ok?
	cmp	ax, 0
	jge	.loadFile

	; Update progress
	mov	dx, TEXT_WHITE
	mov	si, msgError
	call	kloaderPrintString
	call	kloaderPrintCrLf
	mov	si, msgLoadFATError
	call	kloaderPrintString
	call	kloaderPrintCrLf

	; Failed to load FAT, return -3
	mov	dword [ss:(bp+16)], ERR_LOAD_FAT
	jmp .done

.loadFile:
	; Update progress
	mov	dx, TEXT_WHITE
	mov	si, msgDot
	call	kloaderPrintString

	; Pass starting cluster of file, memory
	; destination address and load the kernel
	pop	ax									; Get starting cluster saved earlier
	push	dword [pKernelAddress]			; 2nd param - kernel destination
	push	ax								; 1st param - staring cluster
	call	loadFile
	add	sp, 0x06

	; Successful?
	cmp	ax, 0
	jge	.checkKernel

	; Update progress
	mov	dx, TEXT_WHITE
	mov	si, msgError
	call	kloaderPrintString
	call	kloaderPrintCrLf
	mov	si, msgLoadKernelError
	call	kloaderPrintString
	call	kloaderPrintCrLf

	; Failed to load kernel, return -4
	mov	dword [ss:(bp+16)], ERR_LOAD_KERNEL
	jmp	.done

.checkKernel:
	; Update progress
	mov	dx, TEXT_WHITE
	mov	si, msgDot
	call	kloaderPrintString

	; Check the kernel ELF header
	call	parseELFHeader

	; Successful?
    ; Returns end of kernel marker (0xC000?000)
    ; Error number start at 1 and go until ERR_LAST_ERROR (~dozen)
    ; Check return code is greater then last error number
	cmp	eax, ERR_LAST_ERROR
	ja	.prepareKernel
	push	eax								; Save parseELFHeader() error return code

	; Update progress
	mov	dx, TEXT_WHITE
	mov	si, msgError
	call	kloaderPrintString
	call	kloaderPrintCrLf
	mov	si, msgParseELFError
	call	kloaderPrintString
	call	kloaderPrintCrLf

	pop	eax									; Return error code from parseELFHeader()
	mov	dword [ss:(bp+16)], eax
	jmp	.done

.prepareKernel:
	mov	dword [kernelEnd], eax

	; Update progress
	mov	dx, TEXT_WHITE
	mov	si, msgDot
	call	kloaderPrintString

	; Modify kernel image in memory
	call	prepareKernel

	; Successful?
    ; Testing/error message will not occur
    ; prepareKernel does not return error code (always returns 0)
	cmp	ax, 0
	jge	.success

	;push	ax								; Save parseELFHeader() error return code

	; Update progress
	mov	dx, TEXT_WHITE
	mov	si, msgError
	call	kloaderPrintString
	call	kloaderPrintCrLf
	mov	si, msgParseELFError
	call	kloaderPrintString
	call	kloaderPrintCrLf

	; We were successful, return kernel end address
.success:
	mov	eax, dword [kernelEnd]
	mov	dword [ss:(bp+16)], eax

.done:
	popa
	xor	eax, eax
	pop	eax
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; computeVolumeInfo:
;	Calculates filesystem constants based upon our current
;	volume and allocates a buffer for our cluster data
;
; Calculates = RootDirSectorCount, BytesPerCluster, ClusterBuffer
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
computeVolumeInfo:
	pusha

	; Calculate size of root directory in sectors
	mov	ax, FAT_ROOTDIR_ENTRYSIZE
	mul	word [MaxRootDirEntryCount]
	xor	dx, dx
	div	word [BytesPerSector]
	mov	word [RootDirSectorCount], ax

	; Calculate bytes per cluster
	mov	ax, word [BytesPerSector]
	xor	bx, bx
	mov	bl, byte [SectorsPerCluster]
	mul	bx
	mov	word [BytesPerCluster], ax

	; Store cluster data after the FAT
	xor	eax, eax
	mov	ax, word [SectorsPerFAT]
	mul	word [BytesPerSector]
	add	eax, FAT_BUFFER
	shr	eax, 4
	mov	word [ClusterBuffer], ax

	popa
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; loadRootDirectory: Loads root directory into memory buffer at ROOTDIR_BUFFER
; Standard C calling convention
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
loadRootDirectory:

	; Save 2 bytes for return value
	sub	sp, 0x02

	pusha
	mov	bp, sp

	; Calculate start of root directory
	mov	ax, word [ReservedSectorCount]		; Bootsector
	add	ax, word [SectorsPerFAT]			; FAT
	add	ax, word [SectorsPerFAT]			; Redundant FAT copy

	call LBA2CHS

	; Count read attempts
	push	word 0

.readAttempt:
	push	es
	mov	ax, word TO_SEGMENT(ROOTDIR_BUFFER)
	mov	es, ax
	xor	bx, bx
	mov	ch, byte [track]
	mov	cl, byte [sector]
	mov	dx, word [bootDrive]
	mov	dh, byte [head]
	mov	ax, word [RootDirSectorCount]
	mov	ah, 0x02
	int	0x13
	jnc	.rootDirLoaded

	pop	es

	; Reset disk controller
	xor	ah, ah
	mov	dx, word [bootDrive]
	int	0x13

	; Increment counter and try again
	pop	ax
	inc	ax
	push	ax
	cmp	ax, 0x05
	jnae	.readAttempt

.fail:
	add	sp, 0x02							; Remove read attempt counter
	call	kloaderDiskError
	mov	word [ss:(bp+16)], -1				; Return -1 (failure)
	jmp	.done

.rootDirLoaded:
	pop	es
	add	sp, 0x02							; Remove read attempt counter
	mov	word [ss:(bp+16)], 0				; Return 0 (success)

.done:
	popa
	xor	eax, eax
	pop	ax									; Return code
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; findFile: Searches root directory and returns starting cluster of file
; Standard C calling convention
;
; Input: char *filename (pushed on stack)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
findFile:

	; Save 2 bytes for return value
	sub	sp, 0x02

	pusha
	mov	bp, sp

	; Initialize index
	mov	word [CurrentDirEntry], 0

	;mov	si, word [ss:(bp+20)]

	push	es
	mov	ax, word TO_SEGMENT(ROOTDIR_BUFFER)
	mov	es, ax

	; Grab entry from root directory
.entryLoop:
	mov	si, word [ss:(bp+20)]				; Filename pushed onto stack
	mov	di, word [CurrentDirEntry]
	mov	al, byte [ES:DI]
	cmp	al, FAT_FILE_ENTRY_DELETED
	je	.nextFile
	cmp	al, 0x0								; No more entries
	je	.file404

	; Compare filenames
.checkLetter:
	cld
	mov	di, word [CurrentDirEntry]
	mov	cx, FAT_FILENAME_SIZE
	repe	cmpsb
	jne	.nextFile
	jmp	.foundFile

	; Increment index and check next entry
.nextFile:
	add	word [CurrentDirEntry], FAT_ROOTDIR_ENTRYSIZE

	; Make sure we're not at end of directory
	mov	ax, word [BytesPerSector]
	mul	word [RootDirSectorCount]
	cmp	word [CurrentDirEntry], AX
	jae	.file404
	jmp	.entryLoop

.file404:
	pop	es
	mov	word [ss:(bp+16)], -1
	jmp	.done

.foundFile:
	mov	bx, word [CurrentDirEntry]
	add	bx, FAT_ROOTENTRY_START_CLUSTER
	mov	ax, word [es:bx]
	mov	word [ss:(bp+16)], ax				; Save starting cluster

	; Save size of file
	mov	bx, word [CurrentDirEntry]
	add	bx, FAT_ROOTENTRY_FILESIZE
	mov	eax, dword [es:bx]					; Size in bytes
	xor	edx, edx
	mov	ecx, 1024
	div ecx									; Convert to kilobytes
	cmp	edx, 0								; Round up?
	je	.noRound
	inc	eax									; Round up to next highest kilobyte
.noRound:
	mov	word [FileSize], ax
	pop	es

.done:
	popa
	xor	eax, eax
	pop	ax
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; loadFAT: Load FAT into memory at FAT_BUFFER
; Standard C calling convention
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
loadFAT:

	; Save 2 bytes for return value
	sub	sp, 0x2

	pusha
	mov	bp, sp

	; FAT is after bootsector
	mov	ax, word [ReservedSectorCount]
	call	LBA2CHS

	; Count read attempts
	push	word 0

.readFAT:
	push	es
	mov	ax, word TO_SEGMENT(FAT_BUFFER)
	mov	es, ax
	xor	bx, bx
	mov	ch, byte [track]
	mov	cl, byte [sector]
	mov	dx, word [bootDrive]
	mov	dh, byte [head]
	mov	ax, word [SectorsPerFAT]
	mov	ah, 0x02
	int	0x13
	jnc	.success

	pop	es

	; Reset and try 4 more times
	xor	ah, ah
	mov	dx, word [bootDrive]
	int	0x13

	; Increment counter
	pop	ax
	inc	ax
	push	ax
	mov	ax, 0x05
	jnae	.readFAT

.fail:
	add	sp, 0x02							; Remove counter
	call	kloaderDiskError
	mov	word [ss:(bp+16)], -1				; Return failure
	jmp	.done

.success:
	pop	es
	add	sp, 0x02							; Remove counter
	mov	word [ss:(bp+16)], 0				; Return success

.done:
	popa
	xor	eax, eax
	pop	ax
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; loadFile: Load file into memory
; Standard C calling convention
;
; Input: unsigned int startingCluster (stack param 1)
;	void *memoryLocation (stack param 2)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
loadFile:

	; Save 2 bytes for return value
	sub	sp, 0x2

	pusha
	mov	bp, sp

	mov	dword [BytesRead], 0
	;mov	word [OldProgress], 0

	; Load starting cluster
	mov	ax, word [ss:(bp+20)]
	mov	word [NextCluster], ax

	; Initialize memory pointer
	mov eax, dword [ss:(bp+22)]
	mov	dword [MemoryPointer], eax

	push	es

	; Track read attempts
.FATloop:
	push	word 0

	; Get next cluster
.readAttempt:
	mov	ax, word [NextCluster]
	call	cluster2Logical
	call	LBA2CHS

	; Read cluster from disk
	mov	es, word [ClusterBuffer]
	xor	bx, bx
	mov	ch, byte [track]
	mov	cl, byte [sector]
	mov	dx, word [bootDrive]
	mov	dh, byte [head]
	mov	al, byte [SectorsPerCluster]
	mov	ah, 0x02
	int	0x13
	jnc	.gotCluster

	; Failed, reset drive and try again
	mov	si, msgError
	mov	dx, TEXT_WHITE
	call	kloaderPrintString

	mov	ah, 0x0
	mov	dx, word [bootDrive]
	int	0x13

	; Up to 5 times
	pop	ax
	inc	ax
	push	ax
	cmp	al, 0x05
	jnae	.readAttempt

.fail:
	add	sp, 0x02							; Remove counter
	call	kloaderDiskError
	mov	word [ss:(bp+16)], -1
	jmp	.done

.gotCluster:
	add	sp, 0x02							; Remove counter

	; Update number of bytes read
	mov	eax, dword [BytesRead]
	add	ax, word [BytesPerCluster]
	mov	dword [BytesRead], eax
	and	eax, 0x1000
	;test	ax
	jne	.noProgress

	; Update progress
	mov	si, msgDot
	mov	dx, TEXT_WHITE
	call	kloaderPrintString

	; Move our loaded cluster data into memory buffer
	; Do this through "big real mode"

	; Go to protected mode temporarily
.noProgress:
	cli
	mov	eax, cr0
	or	al, 1
	mov	cr0, eax

	;[BITS 32]

	; Load our 32-bit GDT selector
	xor	ax, ax
	mov	ax, dataSel
	mov	es, ax

	; Swtich back to unreal mode
	mov	eax, cr0
	and	al, 0xfe
	mov	cr0, eax

	;[BITS 16]

	; Copy cluster we just read to correct destination address
	; a32 prefix will use DS:ESI as source and ES:EDI as target
	xor	ecx, ecx
	mov	cx, word [BytesPerCluster]
	shr	ecx, 2
	xor	esi, esi
	mov	edi, dword [MemoryPointer]			; ES was set above in protected mode

	push	ds
	mov	ds, word [ClusterBuffer]

	cld
	a32 rep movsd
	pop	ds
	sti

	; Update variables and loop if not finished
	xor	eax, eax
	mov	ax, word [BytesPerCluster]
	add	dword [MemoryPointer], eax			; Update memory pointer

	; Point to FAT buffer again to read next cluster
	mov	ax, word TO_SEGMENT(FAT_BUFFER)
	mov	es, ax
	mov	ax, word [NextCluster]
	mov	bx, FAT_NYBBLES_PER_ENTRY
	mul bx

	mov	bx, ax
	shr	bx, 1

	; CF set?  Mask value in AX if needed
	jnc .mask

	mov	ax, word [es:bx]
	shr	ax, 4
	jmp	.doneConvert

.mask:
	mov	ax, word [es:bx]
	and	ax, 0x0FFF

.doneConvert:
	cmp	ax, FAT_END_OF_CLUSTER
	jae	.success

	mov	word [NextCluster], ax
	jmp	.FATloop

.success:
	mov	word [ss:(bp+16)], 0

.done:
	pop	es
	popa
	xor	eax, eax
	pop	ax
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; parseELFHeader: Parses kernel ELF header and saves relevant information
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
parseELFHeader:

	; Save 4 bytes for return value
	sub	sp, 0x4

	pusha
	mov	bp, sp

	; Go to big-real mode or whatever it's called

	; Go to protected mode
	cli
	mov	eax, cr0
	or	al, 0x01
	mov	cr0, eax

	; Load GS with global data segment selector
	mov	eax, dataSel
	mov	gs, ax

	; Back to real mode
	mov	eax, cr0
	and	al, 0xFE
	mov	cr0, eax
	sti

	; Check for magic ELF number
	mov	esi, [pKernelAddress]
	mov	eax, dword [gs:esi]
	cmp	eax, ELF_MAGIC_NUMBER
	je	.isValidELF

	; Error: Not an ELF binary
	mov	dword [ss:(bp+16)], ERR_INVALID_ELF
	jmp	.done

	; Make sure it's an executable ELF binary
.isValidELF:
	mov	esi, [pKernelAddress]
	mov	ax, word [gs:(esi+ELF_FILE_TYPE)]
	cmp	ax, ELF_ET_EXECUTABLE
	je	.isExecutable

	; Error: Not an executable ELF binary
	mov	dword [ss:(bp+16)], ERR_NOT_EXEC_ELF
	jmp	.done

	; OK.  Now parse header.  Gather entry point, program & section tables
	; header offset and size and number of entries in section table
.isExecutable:
	mov	esi, [pKernelAddress]
	mov	eax, dword [gs:(esi+ELF_ENTRY_POINT)]
	mov	dword [kernelEntryPoint], eax

	mov	esi, [pKernelAddress]
	mov	eax, dword [gs:(esi+ELF_PROGRAM_HEADER)]
	mov	dword [elfProgramHeader], eax

	;mov	esi, [pKernelAddress]
	;mov	eax, dword [gs:(esi+ELF_SECTION_HEADER)]
	;mov	dword [elfSectionHeader], eax
	;
	;mov	esi, [pKernelAddress]
	;xor	eax, eax
	;mov	ax, word [gs:(esi+ELF_SH_ENTRY_SIZE)]
	;mov	dword [elfSHeaderEntrySize], eax
	;
	;mov	esi, [pKernelAddress]
	;xor	eax, eax
	;mov	ax, word [gs:(esi+ELF_NUM_SH_ENTRIES)]
	;mov	dword [elfSHeaderNumEntries], eax

	; Right now we can only handle 2 program header entries (code + data)
	mov	esi, [pKernelAddress]
	mov	eax, dword [gs:(esi+ELF_NUM_SEGMENTS)]
	cmp	ax, 0x2
	je	.parseHeader

	; Error: Kernel image does not have exactly 2 ELF segments
	mov	dword [ss:(bp+16)], ERR_NUM_SEGMENTS
	jmp	.done

.parseHeader:
	mov	esi, [pKernelAddress]
	add	esi, dword [elfProgramHeader]

	; Skip segment type
	add	esi, 0x4

	mov	eax, dword [gs:esi]
	mov	dword [kernelCodeOffset], eax
	add	esi, 0x4
	mov	eax, dword [gs:esi]
	mov	dword [kernelCodeVirtAddress], eax

	; Skip physical address
	add	esi, 0x8

	; Make sure size in file and memory is the same
	mov	eax, dword [gs:esi]
	mov	dword [kernelCodeFileSize], eax
	add	esi, 0x4
	mov	eax, dword [gs:esi]
	cmp	eax, [kernelCodeFileSize]
	je	.codeSegmentOK

	; Error: Kernel image does not look way we anticipate
	mov	dword [ss:(bp+16)], ERR_SEGMENT_LAYOUT
	jmp	.done

	; Skip flags, check alignment (must equal PAGE_SIZE)
.codeSegmentOK:
	add	esi, 0x8
	mov	eax, dword [gs:esi]
	cmp	eax, PAGE_SIZE
	je	.dataSegment

	; Error: Kernel image segment alignment invalid
	mov	dword [ss:(bp+16)], ERR_SEGMENT_ALIGN
	jmp	.done

	; Now we do the data segment.  Skip segment type.
.dataSegment:
	add	esi, 0x8

	mov	eax, dword [gs:esi]
	mov	dword [kernelDataOffset], eax
	add	esi, 0x4
	mov	eax, dword [gs:esi]
	mov	dword [kernelDataVirtAddress], eax

	; Skip physical address
	add	esi, 0x8

	mov	eax, dword [gs:esi]
	mov	dword [kernelDataFileSize], eax
	add	esi, 0x4
	mov	eax, dword [gs:esi]
	mov	dword [kernelDataMemSize], eax

	; Skip flags
	add	esi, 0x8

	; Check alignment
	mov	eax, dword [gs:esi]
	cmp	eax, PAGE_SIZE
	je	.calcKernelSize

	; Error: Kernel image segment alignment invalid
	mov	dword [ss:(bp+16)], ERR_SEGMENT_ALIGN
	jmp	.done

.calcKernelSize:
	mov	eax, dword [kernelDataVirtAddress]
	add	eax, dword [kernelDataMemSize]


;	; Now we try to calculate the total size in memory of the allocated
;	; kernel image.  Must determine at runtime, as a BSS section is present
;	; in the symbol table, but not in the file executable.  Need to implicitly
;	; reserve memory for this.
;	;
;	; Instead of searching for BSS specifically, we will just search for the
;	; last section with SHF_ALLOC set in case additional sections are added.
;	; We will take the starting address of the next section as the end of our
;	; kernel.
;	;
;	; Another approach should be possible by searching for sections with
;	; SHT_NOBITS and increasing address range to match but would be more
;	; complex to implement.
;	mov	cx, word [elfSHeaderNumEntries]
;	mov	esi, [pKernelAddress]
;	add	esi, dword [elfSectionHeader]
;	xor	eax, eax
;	mov	ax, word [elfSHeaderEntrySize]
;	add	esi, eax								; Skip first NULL entry
;
;.sectionLoop:
;	mov	eax, dword [gs:(esi+ELF_SH_FLAGS)]
;	and	eax, ELF_SHF_ALLOC
;	jz	.foundFirstNoAlloc						; Look for first entry without SHF_ALLOC
;	xor	eax, eax
;	mov ax, word [elfSHeaderEntrySize]
;	add	esi, eax
;	dec	cx
;	cmp	cx, 0x0
;	jne	.sectionLoop
;
;	; Error: Cannot find an entry without SHF_ALLOC
;	mov	dword [ss:(bp+16)], ERR_ALL_ALLOC
;	jmp	.done
;
;	; We found the first entry without SHF_ALLOC.
;	; Start of this section will be the end of our kernel.
;.foundFirstNoAlloc:
;	mov	eax, dword [gs:(esi+ELF_SH_ADDRESS)]
;	;mov	dword [elfSectionAddress], eax


	; Return end of kernel marker
.success:
	mov	dword [ss:(bp+16)], eax
.done:
	popa
	xor	eax, eax
	pop	eax
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; preapareKernel: Takes kernel image in memory and prepare to execute
; 	Provides functionality similar to objcopy -O binary
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
prepareKernel:

	; Save 2 bytes for return
	sub	sp, 0x2

	; Save regs
	pusha
	mov	bp, sp

	; First move code segment to entry point.  Do so by moving entire
	; code segment forward codeOffset bytes.  Will overwrite ELF header
	; and program header.
	mov	ecx, dword [kernelCodeFileSize]
	mov	esi, dword [pKernelAddress]
	add	esi, dword [kernelCodeOffset]
	mov	edi, dword [pKernelAddress]

.moveCodeSegment:
	mov	al, byte [gs:esi]
	mov	byte [gs:edi], al
	inc	esi
	inc	edi

	dec	ecx
	cmp	ecx, 0
	jne	.moveCodeSegment

	; Do same for data segment.  Ensure difference b/w code & data
	; virtual addresses is same as difference b/w offsets in file.
	mov	eax, dword [kernelDataVirtAddress]
	sub	eax, dword [kernelCodeVirtAddress]
	cmp	eax, dword [kernelDataOffset]
	je	.dataSegmentOK

	; Kernel image does not look way we expected.  Adjust it.
	; Move initialized data forward from original offset so it matches
	; difference b/w code & data segments virtual addresses
	mov	ecx, dword [kernelDataFileSize]
	mov	esi, dword [pKernelAddress]
	add	esi, dword [kernelDataOffset]
	mov	edi, dword [kernelDataVirtAddress]
	sub	edi, dword [kernelCodeVirtAddress]
	mov	dword [kernelDataOffset], edi		; Update data offset
	add	edi, dword [pKernelAddress]

.moveDataSegment:
	mov	al, byte [gs:esi]
	mov	byte [gs:edi], al
	inc	esi
	inc	edi

	dec	ecx
	cmp	ecx, 0
	jne	.moveDataSegment

	; Zero new memory range for difference b/w size in
	; memory and file of data segment
.dataSegmentOK:
	mov	ecx, dword [kernelDataMemSize]
	sub	ecx, dword [kernelDataFileSize]
	mov	edi, dword [pKernelAddress]
	add	edi, dword [kernelDataOffset]
	add	edi, dword [kernelDataFileSize]

.zeroLoop:
	mov	byte [gs:edi], 0
	inc	edi
	dec	ecx
	cmp	ecx, 0
	jne	.zeroLoop

.success:
	mov	word [ss:(bp+16)], 0

.done:
	popa
	xor	eax, eax
	pop	ax
	ret


; *** Various conversion functions follow ***

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; cluster2Logical: Converts cluster number to LBA value
; Input: AX=cluster number
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
cluster2Logical:

	; Save 2 bytes for return code
	sub	sp, 0x2

	pusha
	mov	bp, sp

	; Cluster numbering starts at 2
	sub	ax, 0x2

	; Multiply by sectors per cluster
	;xor	bx, bx
	mov	bl, byte [SectorsPerCluster]
	mul	bl

	; Calculate start of data clusters
	add	ax, word [ReservedSectorCount]		; Bootsector
	add	ax, word [SectorsPerFAT]			; FAT
	add	ax, word [SectorsPerFAT]			; Redundant FAT copy
	add	ax, word [RootDirSectorCount]

	; Return value
	mov	word [ss:(bp+16)], ax

	popa
	pop	ax
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; LBA2CHS: Converts LBA addressing scheme to CHS values
;
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

; Quick function to display disk error
kloaderDiskError:
	mov	si, msgDiskError
	mov	dx, TEXT_WHITE
	call	kloaderPrintString
	jmp $

;*******************************************************************************
;*                              DATA SEGMENT                                   *
;*******************************************************************************

	SECTION .data
	ALIGN 4

; System messages
msgDot					db ".", 0
msgError				db "!", 0
msgR					db "r", 0
msgC					db "c", 0
msgDiskError			db "Loader Ktux kernel failed.", 0
msgLoadRootDirError		db "Failed to load root directory", 0
msgLoadKernelError		db "Failed to load kernel", 0
msgLoadFATError			db "Failed to load FAT", 0
msgLoadFindError		db "Failed to find kernel file", 0
msgParseELFError		db "Kernel image is not a valid executable ELF binary", 0

; Physical drive information
head					db 0
track					db 0
sector					db 0

; FAT filesystem information
RootDirSectorCount		dw 0
BytesPerCluster			dw 0
ClusterBuffer			dw 0
CurrentDirEntry			dw 0
FileSize				dw 0
BytesRead				dd 0
NextCluster				dw 0
MemoryPointer			dd 0

; ELF header information
pKernelAddress			dd 0
kernelEntryPoint		dd 0
kernelEnd				dd 0
elfProgramHeader		dd 0
elfSectionHeader		dd 0
elfSHeaderEntrySize		dw 0
elfSHeaderNumEntries	dw 0
elfSectionType			dd 0
elfSectionFlags			dd 0
elfSectionAddress		dd 0
elfSectionSize			dd 0
kernelCodeOffset		dd 0
kernelCodeVirtAddress	dd 0
kernelCodeFileSize		dd 0
kernelDataOffset		dd 0
kernelDataVirtAddress	dd 0
kernelDataFileSize		dd 0
kernelDataMemSize		dd 0
