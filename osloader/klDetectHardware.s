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
	GLOBAL kloaderDetectHardware

; Global variables (data) this file provides
	GLOBAL e820Buffer

; External functions this source file accesses
	EXTERN kloaderPrintString
	EXTERN kloaderFatalError
	EXTERN kloaderPrintCrLf
	EXTERN kloaderPrintEBP16

; External variables this source file accesses
	EXTERN cpuInfoStructAddress
	EXTERN totalMemory
	EXTERN e820BufferAddress


	[BITS 16]
	SECTION .text

	%include "kloader.h"


;*******************************************************************************
; Function: kloaderDetectHardware
; Input: None
; Output: Detects hardware in system
;*******************************************************************************
kloaderDetectHardware:
	pusha

	call	kloaderPrintCrLf
	mov	si, msgDetectingHardware
	mov	dx, TEXT_WHITE
	call	kloaderPrintString

	call	DetectProcessor
	
	; Update progress
	mov	si, msgDot
	mov	dx, TEXT_WHITE
	call	kloaderPrintString

	call	DetectMemory
	
	; Update progress	
	mov	si, msgDot
	mov	dx, TEXT_WHITE
	call	kloaderPrintString

	; Detect more hardware here
	;call	kloaderPrintCrLf

	popa
	ret


;*******************************************************************************
; Function: DetectProcessor
;
; - Tries to detect the CPU
; - Aborts on error or 16-bit CPU found
; 
; Inputs: None
; Outputs:
;	- Returns CPU in 'cpuType'
;	- Aborts (fatal error) if 16-bit CPU found (needs 32-bit to run)
;
; Notes:
;	- Try some known CPU id techniques (bits in flags/eflags)
;	- Use CPUID (if CPU supports it)
;	- No reset signature crap
;	- Determine clock frequency (using several methods)
;*******************************************************************************

DetectProcessor:
	mov	[cpuType], byte CPU_UNKNOWN	; Set default

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; First check to see for 16/32 bit CPU                 ;
	; Ktux cannot run on a 16-bit CPU (32-bit 386 minimum) ;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Set 16-bit flags to on, if bits [12-15] don't stay set it's a 16-bit
	pushf				
	push	byte -1				; Can't push FF (signed byte exceeds bounds)
	popf					; Set flags on
	pushf
	pop	ax				; Flags in ax
	popf

	; If bits [12-15]=0 it's a 286 (16-bit)
	and	ah, 0xf0
	jz	.procChkIs16Bit
	; If bit 15=0, it's a 386 or better (32-bit)
	and	ah, 0x80
	jz	.procChkIs32Bit
	; Execution shouldn't get here but if it does, display error
	mov	si, msgProcChkError
	jmp	.procChkError

	; We got a 16-bit or otherwise unknown CPU here, must abort
.procChkIs16Bit:
	mov	[cpuType], word CPU_80286	; Must be a 286
	mov	si, msgProcChk16Bit
.procChkError:
	mov	dx, TEXT_WHITE
	call	kloaderPrintString
	call	kloaderFatalError

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; We got this far, must be a 32-bit CPU, check for 386/486 ;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Toggle AC bit in eflags (bit 18), if stays it's a 486
.procChkIs32Bit:
	mov	[cpuType], word CPU_80386	; At least a 386

	mov	bx, sp				; Save stack pointer
	and	sp, 0xFFFC			; Align stack to avoid AC fault

	pushfd
	pushfd
	pop	eax				; Put flags in eax
	mov	ecx, eax

	xor	eax, 0x40000			; Toggle AC bit
	push	eax
	popfd					; Set eflags
	pushfd

	pop	eax				; Get flags again
	popfd
	xor	eax, ecx			; Compare
	mov	sp, bx				; Restore stack pointer
	jnz	.procChkIs486			; Was bit set?

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Must be 386, check for SX/DX ;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Bit 4 of MSW can be flipped if it's a DX
	mov	eax, cr0			; Get CR0
	mov	ebx, eax
	xor	al, 0x10			; Flip bit
	mov	cr0, eax			; Put back CR0
	mov	eax, cr0			; Get again to compare
	mov	cr0, ebx			; Restore original CR0
	xor	eax, ebx			; Compare
	jnz	.procChkIs386DX

	;;;;;;;;;;;;;;;;;;
	; We got a 386SX ;
	;;;;;;;;;;;;;;;;;;
	mov	[cpuType], word CPU_80386SX
	jmp	.procChkDone

	;;;;;;;;;;;;;;;;;;
	; We got a 386DX ;
	;;;;;;;;;;;;;;;;;;
.procChkIs386DX:
	mov	[cpuType], word CPU_80386DX
	jmp	.procChkDone

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; CPU at least a 486, check for CPUID support ;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.procChkIs486:
	mov	[cpuType], word CPU_80486
	pushfd
	pushfd
	pop	eax				; Put flags in eax
	mov	ecx, eax

	xor	eax, 0x200000			; Toggle ID bit
	push	eax
	popfd					; Set eflags
	pushfd

	pop	eax				; Get flags again
	popfd
	xor	eax, ecx			; Compare
	jnz	.procChkHasCPUID		; Was bit set?
	jmp	.procChkNoCPUID

.procChkHasCPUID:
	;;;;;;;;;;;;;;;;;;;;;;;;
	; We got CPUID, use it ;
	;;;;;;;;;;;;;;;;;;;;;;;;
	xor	eax, eax			; CPUID function 0
	cpuid
	mov	[cpuIDMaxFunc], eax
	mov	[cpuIDVendorString], ebx
	mov	[cpuIDVendorString+4], edx
	mov	[cpuIDVendorString+8], ecx
	
	; CPUID function 1 (all chips have this)
	mov	eax, 1
	cpuid
	mov	[cpuIDSignature], eax
	mov	[cpuIDBrand], ebx
	mov	[cpuIDFeatures2], ecx
	mov	[cpuIDFeatures], edx

	; Try first CPUID extended function
	mov	eax, 0x80000000
	cpuid
	mov	[cpuIDMaxExtFunc], eax
	cmp	eax, 0
	;or	eax, eax
	jnz	.procChkJmp1
	jmp	.procChkCalcFreq		; No extended support

.procChkJmp1:
	cmp	eax, 0x80000001			; Safety check
	jnl	.procChkJmp2
	jmp	.procChkCalcFreq

.procChkJmp2:
	; Extended CPUID function 1
	mov	eax, 0x80000001
	cpuid
	mov	[cpuIDExtSignature], eax
	mov	[cpuIDExtBrand], ebx
	mov	[cpuIDExtFeatures2], ecx
	mov	[cpuIDExtFeatures], edx

	cmp	[cpuIDMaxExtFunc], dword 0x80000004	; CPU Name
	jnl	.procChkJmp3
	jmp	.procChkCalcFreq

.procChkJmp3:
	; Extended CPUID function 2 (CPU name)
	mov	bp, cpuIDExtName
	mov	eax, 0x80000002
	cpuid
	mov	[bp], eax
	mov	[bp+4], ebx
	mov	[bp+8], ecx
	mov	[bp+12], edx

	mov	eax, 0x80000003
	cpuid
	mov	[bp+16], eax
	mov	[bp+20], ebx
	mov	[bp+24], ecx
	mov	[bp+28], edx

	mov	eax, 0x80000004
	cpuid
	mov	[bp+32], eax
	mov	[bp+36], ebx
	mov	[bp+40], ecx
	mov	[bp+44], edx

	; Extended CPUID function 5 - TLB & Cache info
	cmp	[cpuIDMaxExtFunc], dword 0x80000005
	jl	.procChkCalcFreq
	mov	eax, 0x80000005
	cpuid
	mov	[cpuIDExtTLBInfo], ebx
	mov	[cpuIDExtL1DCache], ecx
	mov	[cpuIDExtL1ICache], edx

	; Extended CPUID function 5 - L2 Cache (only on AMD K6-II+)
	cmp	[cpuIDMaxExtFunc], dword 0x80000006
	jl	.procChkCalcFreq
	mov	eax, 0x80000006
	cpuid
	mov	[cpuIDExtL2Cache], ecx

.procChkCalcFreq:
	; Try and calculate CPU clock frequency with RDTSC timing instruction ;
	bt	dword [cpuIDFeatures], 4	; Test for RDTSC instruction
	jnc	.procChkNoRDTSC

	push	es
;	push	es
;	xor	ax, ax
;	mov	ds, ax
;	push	BIOSDATA_SEGMENT
;	pop	es
;	mov	di, BIOSDATA_TICKCOUNT
;	mov	ecx, [es:di]
;	mov	ebx, [es:di]
;	mov	bx, BIOSDATA_SEGMENT
;	shl	bx, 4
;	add	bx, BIOSDATA_TICKCOUNT
;	mov	ecx, [bx]

	; Get BIOS tick counter (~18.2 Hz)
	push	BIOSDATA_SEGMENT
	pop	es
	mov	si, BIOSDATA_TICKCOUNT
	mov	ebx, dword [es:si]

.procChkNewTick:
	; Wait for a new tick
	cmp	ebx, dword [es:si]
	je	.procChkNewTick

	; Timed period starts here
	rdtsc
	mov	[cpuTSCLoWord], eax
	mov	[cpuTSCHiWord], edx
	add	ebx, CPU_TSC_TICKCOUNT+1

	; Wait for CPU_TSC_TICKCOUNT ticks
.procChkWaitForTicks:
	cmp	ebx, dword [es:si]
	jne	.procChkWaitForTicks

	; Read timestamp and calculate difference
	rdtsc
	sub	eax, [cpuTSCLoWord]
	sbb	edx, [cpuTSCHiWord]

	; This really calculates MIPS, not CPU frequency
	; (1/18.2) * 1 mHz = 54945
	mov	ebx, 54945 * CPU_TSC_TICKCOUNT	; Convert to TIPS
	div	ebx
	mov	[cpuFreq], eax
	pop	es				; Restore ES

.procChkNoRDTSC:
.procChkNoCPUID:
.procChkDone:
	mov	eax, cpuInfoStruct		; Mark location of buffer
	;add	eax, BOOTSTUB_ADDRESS
	mov	[cpuInfoStructAddress], eax
	ret


;*******************************************************************************
; Function: DetectMemory
;
; - Tries to detect the amount of memory in the system
; - Gets listing of reserved memory address ranges for kernel MM
; 
; Inputs: None
; Outputs:
;	- Fills memory map entries in 'e820Buffer'
;	- Puts total memory size in 'totalPhysicalMemory' (not yet)
;*******************************************************************************
DetectMemory:

	; Try and determine memory size in PC from int 0x15, ax=0xe820 BIOS call
	xor	ebx, ebx			; First call -> EBX=0
	mov	edx, 0x534d4150			; 'SMAP'
	mov	di, e820Buffer

.memCheckLoop:
	; Set up BIOS call
	mov	eax, 0x0000e820
	mov	ecx, 20
	int	0x15
	jc	.memCheckError			; Carry flag set on error
	cmp	eax, 0x534d4150
	jnz	.memCheckError			; Error if eax != 'SMAP'

	cmp	ecx, 20				; BIOS can't copy less than 20 bytes, problem occured
	jl	.memCheckError
	jg	.memCheckError			; BIOS can't copy more than 20 bytes either
	
	mov	eax, [es:di]			; Count total RAM
	add	eax, [es:di+8]
	mov	[totalMemory], eax

	add	di, 20
	cmp	ebx, 0
	je	.memCheckDone			; ebx=0 if BIOS mapping done
	jmp	.memCheckLoop

.memCheckError:
	cmp	ah, 0x86
	je	.memCheckNotSupported		; For some BIOS's: Call not supported
	mov	si, msgMemChkError
	mov	dx, TEXT_WHITE
	call	kloaderPrintString
	call	kloaderFatalError

.memCheckNotSupported:
	mov	si, msgMemChkNotSupported
	mov	dx, TEXT_WHITE
	call	kloaderPrintString
	call	kloaderFatalError

.memCheckDone:
	mov	[es:di], dword 0xFFFFFFFF	; Put a end-of-buffer marker
	mov	eax, e820Buffer			; Mark location of buffer
	;add	eax, BOOTSTUB_ADDRESS
	mov	[e820BufferAddress], eax
	ret



;*******************************************************************************
;*                              DATA SEGMENT                                   *
;*******************************************************************************
	
	SECTION .data
	ALIGN 4

; Messages
msgDetectingHardware	db "Detecting hardware", 0
msgDot			db ".", 0

msgProcChk16Bit		db "!", msgCr, msgLf, "A 32-bit CPU is required to run Ktux", 0
msgProcChkError		db "!", msgCr, msgLf, "Error detecting processor!", 0

msgMemChkError		db "!", msgCr, msgLf, "Error checking memory!", 0
msgMemChkNotSupported	db "!", msgCr, msgLf, "Your BIOS does not support E820 memory detection!", 0

	ALIGN 4

; CPU detection
cpuInfoStructSize	dw cpuInfoStructEnd-cpuInfoStruct-1

cpuInfoStruct:
cpuType			dw 0
cpuIDMaxFunc		dd 0			; Zero if does not support CPUID
cpuIDVendorString	times 13 db 0		; Extra byte for terminating null
cpuIDSignature		dd 0
cpuIDBrand		dd 0
cpuIDFeatures2		dd 0
cpuIDFeatures		dd 0
cpuIDMaxExtFunc		dd 0			; Mostly on non-Intel chips only
cpuIDExtSignature	dd 0
cpuIDExtBrand		dd 0
cpuIDExtFeatures2	dd 0
cpuIDExtFeatures	dd 0
cpuIDExtName		times 48 db 0
cpuIDExtTLBInfo		dd 0
cpuIDExtL1DCache	dd 0
cpuIDExtL1ICache	dd 0
cpuIDExtL2Cache		dd 0
cpuTSCLoWord		dd 0
cpuTSCHiWord		dd 0
cpuFreq			dd 0
cpuInfoStructEnd:

	ALIGN 4

; BIOS INT 0x15, E820 memory detection buffer
e820Buffer		times MEMORYMAP_MAXENTRIES*20 db 0
