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
    GLOBAL kloaderMain
    GLOBAL kloaderFatalError

; Global variables (data) this file provides
    GLOBAL dataSel
    GLOBAL SectorsPerTrack
    GLOBAL NumberHeads
    GLOBAL cpuInfoStructAddress
    GLOBAL totalMemory
    GLOBAL e820BufferAddress

    GLOBAL bootDrive
    GLOBAL BytesPerSector
    GLOBAL MaxRootDirEntryCount
    GLOBAL SectorsPerCluster
    GLOBAL ReservedSectorCount
    GLOBAL SectorsPerFAT

; External functions this source file accesses
    EXTERN kloaderLoadKernel
    EXTERN kloaderDetectHardware
    EXTERN kloaderInitConsole
    EXTERN kloaderPrintString
    EXTERN kloaderPrintCrLf
    EXTERN kloaderClearScreen


    [BITS 16]
    SECTION .text

    %include "kloader.h"


;*******************************************************************************
; Function: kloaderMain
; Input: Bootdrive and FS on stack from bootsector
; Output: None
;   This is the first function called when the bootsector transfers control
; to the kernel loading stub.
;*******************************************************************************
kloaderMain:

    ; Set segments for correct address
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Initialize video and display loading message
    call    kloaderInitConsole
    call    kloaderClearScreen
    mov dx, TEXT_WHITE
    mov si, msgInitializing
    call    kloaderPrintString

    ; *** Prepare hardware info struct ***
    ; Check filesystem type of boot drive & sector
    pop word [bootDrive]
    pop word [bootFileSystem]
    cmp word [bootFileSystem], FILESYS_FAT12
    je  .fileSysOK
    jmp kloaderUnknownFS

.fileSysOK:
    ; Copy FAT12 BPB over
    mov cx, [fat12BPB_SIZE]
    pop si              ; Pointer to fat12BPB 2nd stack parameter
    push    ds
    xor ax, ax
    mov ds, ax
    mov di, fat12BPB
    cld
    rep movsb               ; DS:SI -> ES:DI
    pop ds

    ; Mark location of the struct
    mov eax, fat12BPB
    ;add    eax, BOOTSTUB_ADDRESS
    mov [bootFATinfo], eax

    ; Update progress
    mov dx, TEXT_WHITE
    mov si, msgDot
    call    kloaderPrintString

    ; Enable A20 line and check for success
    call    EnableA20
    cmp eax, -1
    jnz .A20enabled         ; This hack is annoying
    jmp kloaderFatalError

    ; Update progress
.A20enabled:
    mov dx, TEXT_WHITE
    mov si, msgDot
    call    kloaderPrintString

    ; Move GDT to new location and load
    push    es
    xor ax, ax
    mov es, ax
    mov si, GDT             ; Move From [DS:SI]
    mov di, [GDTbase]           ; Move to [ES:DI]
    mov cx, [GDTsize]           ; size of GDT
    cld                 ; Clear the Direction Flag
    rep movsb
    lgdt    [GDTR]
    pop es

    ; Update progress
    mov dx, TEXT_WHITE
    mov si, msgDot
    call    kloaderPrintString

    ; Start initialization
    call    kloaderDetectHardware

    ; Display message
    call    kloaderPrintCrLf
    mov dx, TEXT_WHITE
    mov si, msgLoadingKernel
    call    kloaderPrintString

    ; Load the kernel
    push    dword KERNEL_PHYSICAL
    push    word kernelFilename
    call    kloaderLoadKernel

    ; Check for error
    cmp eax, ERR_LAST_ERROR
    ja      .loadOK
    ;cmp    ax, 0
    ;jge    .loadOK
    push    ax
    jmp displayLoadError
    add sp, 0x02

    ; Mark size of kernel (round up to nearest page boundry)
.loadOK:
    mov dword [kernelEndAddr], eax
    call    kloaderClearScreen
    mov dx, TEXT_WHITE
    mov si, msgInitSystem
    call    kloaderPrintString
    call    kloaderPrintCrLf
    mov si, msgMemory
    call    kloaderPrintString
    ;xor    eax, eax
    ;mov    eax, KERNEL_END
    ;sub    eax, KERNEL_START
    ;and    eax, 0xFFFFF000
    ;add    eax, 0x1000

    ; Go to protected mode!
    cli
    mov eax, cr0
    or  al, 1
    mov cr0, eax

    ; Short circuit far jump instruction hack
    ; NASM does not add in 0x8000 offset for jump address calculation
    ; Need to change code segment to clear the pipeline
    ;db 0xEA                ; jmp codeSel:.clearPipeline
    ;dw $+0x8004
    ;dw codeSel
    jmp codeSel:clearPipeline

clearPipeline:
    [BITS 32]

    ; Load new segment descriptors
    ;mov    eax, dataSel
    mov eax, dataSelTmp
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov eax, dataSel
    mov ss, eax
    mov esp, PM_STACK_ADDRESS

    ; Pass our environment info to the kernel
    ; Adjust pointer for temporary pre-paging kernel segment
    mov eax, hardwareStruct
    add eax, KERNEL_VIRTUAL
    sub eax, KERNEL_PHYSICAL
    push    eax

    ; Jump to kernel and halt for good measure
    jmp codeSelTmp:KERNEL_VIRTUAL
    hlt

    ; I forgot this for awhile and couldn't figure out
    ; why NASM was generating incorrect opcodes
    ; for all the functions/code below :)

    [BITS 16]

; Fatal Error has occured, stop execution
kloaderFatalError:
    mov si, msgFatalError
    call    kloaderPrintString

    hlt
    jmp $               ; No ret

kloaderUnknownFS:
    mov si, msgUnknownFS
    call    kloaderPrintString
    jmp kloaderFatalError

displayLoadError:
    pusha
    mov bp, sp

    mov dl, TEXT_ERROR
    mov ax, word [SS:(BP+18)]

    cmp ax, ERR_LOAD_ROOT
    jne .1

    mov si, msgNoRoot
    call    kloaderPrintString
    mov si, msgCorruption
    call    kloaderPrintString
    jmp .done

.1:
    cmp ax, ERR_NO_FILE
    jne .2

    mov si, msgKernelFile
    call    kloaderPrintString
    mov si, msgNoFile1
    call    kloaderPrintString
    mov si, msgNoFile2
    call    kloaderPrintString
    jmp .done

.2:
    cmp ax, ERR_LOAD_FAT
    jne .3

    mov si, msgNoFAT
    call    kloaderPrintString
    mov si, msgCorruption
    call    kloaderPrintString
    jmp .done

.3:
    cmp ax, ERR_LOAD_KERNEL
    jne .4

    mov si, msgKernelFile
    call    kloaderPrintString
    mov si, msgBadFile
    call    kloaderPrintString
    mov si, msgCorruption
    call    kloaderPrintString

.4:
    cmp AX, ERR_NOT_EXEC_ELF
    jne .5

    mov si, msgKernelFile
    call    kloaderPrintString
    mov si, msgBadFile
    call    kloaderPrintString

.5:
    cmp AX, ERR_NUM_SEGMENTS
    jne .6

    mov si, msgKernelFile
    call    kloaderPrintString
    mov si, msgNumSegments
    call    kloaderPrintString

.6:
    cmp AX, ERR_SEGMENT_LAYOUT
    jne .7

    mov si, msgKernelFile
    call    kloaderPrintString
    mov si, msgSegmentLayout
    call    kloaderPrintString

.7:
    cmp AX, ERR_SEGMENT_ALIGN
    jne .done

    mov si, msgKernelFile
    call    kloaderPrintString
    mov si, msgSegmentAlign
    call    kloaderPrintString

.done:
    popa
    jmp $


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; EnableA20: Enables A20 address line
; Input: None
;
; Originally by J. Andrew McLaughlin, modified by Anthony L. DeChiaro
;
; Original copyright notice attached below:
;   Copyright (c) 2000, J. Andrew McLaughlin
;   You're free to use this code in any manner you like, as long as this
;   notice is included (and you give credit where it is due), and as long
;   as you understand and accept that it comes with NO WARRANTY OF ANY KIND.
;   Contact me at jamesamc@yahoo.com about any bugs or problems.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
EnableA20:

    ; This subroutine will enable the A20 address line in the keyboard
    ; controller.  Takes no arguments.  Returns 0 in EAX on success,
    ; -1 on failure.
    pusha
    cli
    mov cx, 5               ; Retry up to 5 times to enable A20

.startAttempt1:
    call    KeyboardCmdWait         ; Wait for the controller to be ready for a command
    mov al, 0xd0            ; Tell the controller we want to read the current status.
    out 0x64, al            ; Send the command 0xd0 (Read Output Port)

    call    KeyboardDataWait        ; Wait for the controller to be ready for the data
    xor ax, ax              ; Read the current port status from port 60h
    in  al, 0x60
    push    ax

.commandWait2:              ; Wait for the controller to be ready for a command
    in  al, 0x64
    bt  ax, 1
    jc  .commandWait2

    mov al, 0x0d1           ; Tell the controller we want to write the status byte again
    out 0x64, al

    call    KeyboardCmdWait         ; Wait for the controller to be ready for the data
    pop ax              ; Write the new value to port 60h (we saved old value before)
    or  al, 00000010b           ; Turn on the A20 enable bit
    out 0x60, al

    ; Finally, we will attempt to read back the A20 status to ensure it was enabled
    call    KeyboardCmdWait         ; Wait for the controller to be ready for a command
    mov al, 0x0d0           ; Send the command D0h: read output port.
    out 0x64, al

    call    KeyboardDataWait        ; Wait for the controller to be ready with a byte of data
    xor     ax, ax              ; Read the current port status from port 60h
    in  al, 0x60
    bt  ax, 1               ; Is A20 enabled?
    jc  .success            ; Check the result.  If carry is on, A20 is on.
    loop    .startAttempt1          ; Retry until ECX=0

    ; Well, our initial attempt to set A20 has failed.  Now we will
    ; try a backup method (which is supposedly not supported on many
    ; chipsets, but which seems to be the only method that works on
    ; other chipsets)
    mov cx, 5               ; Another retry counter

.startAttempt2:             ; Wait for the keyboard to be ready for another command
    call    KeyboardCmdWait
    mov al, 0x0df           ; Tell the controller we want to turn on A20
    out 0x64, al

    ; Again, we will attempt to read back the A20 status to ensure it was enabled
    call    KeyboardCmdWait         ; Wait for the controller to be ready for a command
    mov al, 0x0d0           ; Send the command D0h: read output port.
    out     0x64, al

    call    KeyboardDataWait        ; Wait for the controller to be ready with a byte of data
    xor ax, ax              ; Read the current port status from port 60h
    in  al, 0x60
    bt  ax, 1               ; Is A20 enabled?

    jc  .warn               ; Check the result.  If carry is on, A20 is on
    loop    .startAttempt2          ; Retry again until ECX=0
    jmp .fail               ; Failed enabling A20 address line

.warn:                  ; A20 was enabled but through nonstandard method
.success:               ; A20 enabled through first method
    sti
    popa
    xor eax, eax
    ret

.fail:                  ; Couldn't enable A20, return -1
    sti
    popa
    mov eax, -1
    ret

; KeyboardCmdWait: Waits for keyboard controller to be ready to accept data ;
KeyboardCmdWait:
.cmdWait:
    xor ax, ax
    in  al, 0x64
    bt  ax, 1
    jc  .cmdWait
    ret

; KeyboardDataWait: Waits for keyboard controller to be ready with data ;
KeyboardDataWait:
.dataWait:
    xor ax, ax
    in  al, 0x64
    bt  ax, 0
    jnc .dataWait
    ret



;*******************************************************************************
; DATA SEGMENT
;*******************************************************************************

    SECTION .data

; Messages
msgDot              db ".", 0
msgFatalError       db "Fatal error!", 0
msgInitializing     db "Initializing", 0
msgInitSystem       db "initializing system :", 0
msgMemory           db "memory ...", 0
msgLoadingKernel    db "Loading kernel", 0
msgHaltingSystem    db "Halting system!", 0
msgProtected        db "protected mode enabled", 0
msgUnknownFS        db "Filesystem of boot drive not recognized, halting", 0
msgJumping          db "Switching to kernel-space", 0

msgNoRoot           db "The root directory could not be read", 0
msgNoFAT            db "The FAT could not be read", 0
msgKernelFile       db "Ktux kernel file 'KTUXKRNL'"
msgNoFile1          db " could not be found", 0
msgNoFile2          db "Please verify it exists in the root directory", 0
msgBadFile          db " could not be read", 0
msgInvalidElf       db " is not a valid executable ELF file", 0
msgNumSegments      db " does not contain 2 ELF segments", 0
msgSegmentLayout    db " has incorrect ELF segment layout", 0
msgSegmentAlign     db " has incorrectly aligned ELF segments", 0
msgCorruption       db "Possible filesystem corruption on boot device", 0

msgAlignmentError   db "Kernel image has invalid segment alignment", 0


kernelFilename      db 'KTUXKRNL   ', 0


    ALIGN 4

hardwareStruct:
bootDrive               dw 0
bootFileSystem          dw 0
bootFATinfo             dd 0
kernelEndAddr           dd 0                ; In kiloBytes
cpuInfoStructAddress    dd 0
totalMemory             dd 0
e820BufferAddress       dd 0

fat12BPB_SIZE           dw fat12BPB_END-fat12BPB-1

fat12BPB:
OEMName                 db '        '       ; 03 - 0A OEM Name
BytesPerSector          dw 0            ; 0B - 0C Bytes per sector
SectorsPerCluster       db 0            ; 0D - 0D Sectors per cluster
ReservedSectorCount     dw 0            ; 0E - 0F Reserved sectors
NumberFATs              db 0            ; 10 - 10 Copies of FAT
MaxRootDirEntryCount    dw 0            ; 11 - 12 Max. rootdir entries
TotalSectors16          dw 0            ; 13 - 14 Sectors in logical image
Media                   db 0            ; 15 - 15 Media descriptor byte
SectorsPerFAT           dw 0            ; 16 - 17 Sectors in FAT
SectorsPerTrack         dw 0            ; 18 - 19 Sectors per track
NumberHeads             dw 0            ; 1A - 1B Number of heads
HiddenSectors           dd 0            ; 1C - 1D Number of hidden sectors
TotalSectors32          dd 0            ; 1D - 20 Real number of sectors
DriveNumber             db 0            ; 21 - 21 BIOS dr. #
Reserved1               db 0            ; 22 - 22 ?
BootSignature           db 0            ; 23 - 23 Signature
VolumeID                dd 0            ; 24 - 27 Volume ID
VolumeName              db '           '    ; 28 - 42 Volume name
FSType                  db '        '       ; 43 - 4A Filesystem type
fat12BPB_END:


    ; GDT selector
    ALIGN 4

GDTR:
GDTsize         dw GDTEND-GDT
GDTbase         dd 0x500                ; ../include/selectors.h


    ; Ktux's GDT
    ; Selectors must match ../include/selectors.h
    ALIGN 4

; Null descriptor is required
GDT:
nullSel equ $-GDT
            dd 0x0
            dd 0x0

; These are temporary selectors with offseted base
; address to do virtual->physical address conversion
; *MUST* precede real selectors as referenced by ../kernel/paging.s
; Base = 0x40100000
;   ( 0xC0003000(kernel_virt) + 0x40100000(segment base) =
;     [lower 32 bits] 0x103000(kernel_phys) )
codeSelTmp equ $-GDT            ; 4GB Flat Code at 0x40100000 with 0xBFEFFFFF limit
            dw 0xFFFF       ; Limit(2):0xFFFF
            dw 0x0          ; Base(3)
            db 0x10         ; Base(2)
            db 0x9A         ; Type: present,ring0,code,exec/read/accessed (10011010)
            db 0xCF         ; Limit(1):0xF | Flags:4Kb,32bit (11001011)
            db 0x40         ; Base(1)
dataSelTmp equ $-GDT            ; 4GB Flat Data at 0x40100000 with max 0xBFEFFFFF limit
            dw 0xFFFF       ; Limit(2):0xFFFF
            dw 0x0          ; Base(3)
            db 0x10         ; Base(2)
            db 0x92         ; Type: present,ring0,data/stack,read/write (10010010)
            db 0xCF         ; Limit(1):0xF | Flags:4Kb,32bit (11001011)
            db 0x40         ; Base(1)

; Selectors to use once paging has been enabled -> Base = 0
; *MUST* follow temporary selectors as referenced by ../kernel/paging.s
codeSel equ $-GDT               ; 4GB Flat Code at 0x0 with max 0xFFFFF limit
            dw 0xFFFF       ; Limit(2):0xFFFF
            dw 0x0          ; Base(3)
            db 0x0          ; Base(2)
            db 0x9A         ; Type: present,ring0,code,exec/read/accessed (10011010)
            db 0xCF         ; Limit(1):0xF | Flags:4Kb inc,32bit (11001111)
            db 0x0          ; Base(1)
dataSel equ $-GDT               ; 4GB Flat Data at 0x0 with max 0xFFFFF limit
            dw 0xFFFF       ; Limit(2):0xFFFF
            dw 0x0          ; Base(3)
            db 0x0          ; Base(2)
            db 0x92         ; Type: present,ring0,data/stack,read/write (10010010)
            db 0xCF         ; Limit(1):0xF | Flags:4Kb inc,32bit (11001111)
            db 0x0          ; Base(1)
userTSS equ $-GDT
            dw 0x103        ; Limit (103 bytes)
            dw 0x0          ; Base(2)
            db 0x0          ; Base(1)
            db 0xE9         ; Type: present,ring3,32-bit available TSS
            db 0x0
            db 0x0
userCodeSel equ $-GDT               ; 4GB Flat Code at 0x0 with max 0xFFFFF limit
            dw 0xFFFF       ; Limit(2):0xFFFF
            dw 0x0          ; Base(3)
            db 0x0          ; Base(2)
            db 0xFA         ; Type: present,ring3,code,exec/read/accessed (10011010)
            db 0xCF         ; Limit(1):0xF | Flags:4Kb inc,32bit (11001111)
            db 0x0          ; Base(1)
userDataSel equ $-GDT               ; 4GB Flat Data at 0x0 with max 0xFFFFF limit
            dw 0xFFFF       ; Limit(2):0xFFFF
            dw 0x0          ; Base(3)
            db 0x0          ; Base(2)
            db 0xF2         ; Type: present,ring3,data/stack,read/write (10010010)
            db 0xCF         ; Limit(1):0xF | Flags:4Kb inc,32bit (11001111)
            db 0x0          ; Base(1)
GDTEND:
