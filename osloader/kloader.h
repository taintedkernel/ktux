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

; Header file for bootsector/bootstub
; First section are the user-defines constants for Ktux
; Second section contain constants specific to the x86 architecture (DO NOT CHANGE)
; NO CODE/DATA SHOULD BE IN HERE, ONLY %DEFINEs

;*******************************************************************************
; Ktux OS Constants
;*******************************************************************************

; Boolean values
%define BOOL_TRUE                       0
%define BOOL_FALSE                      1

%define ERR_LOAD_ROOT                   1
%define ERR_NO_FILE                     2
%define ERR_LOAD_FAT                    3
%define ERR_LOAD_KERNEL                 4
%define ERR_INVALID_ELF                 5
%define ERR_NOT_EXEC_ELF                6
%define ERR_NUM_SEGMENTS                7
%define ERR_SEGMENT_LAYOUT              8
%define ERR_SEGMENT_ALIGN               9
%define ERR_ALL_ALLOC                   10
%define ERR_LAST_ERROR                  10          ; Needs to be same as highest error number

;**********************************************

; Memory
; Real-mode memory offsets
%define ROOTDIR_BUFFER                  0x1000
%define FAT_BUFFER                      0x3000
%define STACK_SEGMENT                   0x7000
%define STACK_OFFSET                    0x0C00
%define BOOTSTUB_ADDRESS                0x8000
%define KERNEL_PHYSICAL                 0x100000
%define KERNEL_VIRTUAL                  0xC0000000
%define KERNEL_SEGMENT                  (KERNEL_ADDRESS-1)/0x10
%define KERNEL_OFFSET                   0x10

; Protected mode memory offsets
%define PM_STACK_ADDRESS                0x4000

%define MEMORYMAP_MAXENTRIES            32

%define TO_SEGMENT(ADDR)                (ADDR / 16)

;**********************************************

; Do some validation on the data

%if ((BOOTSTUB_ADDRESS % 16) != 0)
%error "Bootstub address must start on 16 byte boundary"
%endif

;%if ((KERNEL_ADDRESS % 4096) != 0)
;%error "Kernel stack size must be a multiple of 4Kb"
;%endif

;**********************************************

; CPU
; CPU Types - Mostly for older CPU's (w/o CPUID function)
%define CPU_UNKNOWN                     0x0000
%define CPU_80286                       0x1000          ; 286 or less
%define CPU_80386                       0x2000          ; Unknown 386
%define CPU_80386SX                     0x2001
%define CPU_80386DX                     0x2002
%define CPU_80486                       0x3000          ; Unknown 486
%define CPU_80486SX                     0x3001
%define CPU_80486DX                     0x3002
%define CPU_Pentium                     0x4000          ; Pentium (or equilivent) or better

; Other CPU related constants
%define CPU_TSC_TICKCOUNT               2

;**********************************************

; Filesystem types
%define FILESYS_FAT12                   0
%define FILESYS_FAT16                   1
%define FILESYS_FAT32                   2

; Filesystem information
%define FAT_ROOTDIR_ENTRYSIZE           0x20
%define FAT_NYBBLES_PER_ENTRY           0x03
%define FAT_ROOTENTRY_START_CLUSTER     0x1A
%define FAT_ROOTENTRY_FILESIZE          0x1C
%define FAT_FILE_ENTRY_DELETED          0xE5
%define FAT_FILENAME_SIZE               0x0A            ; 8+3=11
%define FAT_END_OF_CLUSTER              0x0FF8

%define START_CLUSTER                   2

;**********************************************

; Executable file information

; ELF header information
%define ELF_FILE_TYPE                   0x10            ; ELF object type at ELF_header+16
%define ELF_ENTRY_POINT                 0x18            ; Entry point at ELF_header+24
%define ELF_PROGRAM_HEADER              0x1C            ; Program header at ELF_header+28
%define ELF_SECTION_HEADER              0x20            ; Section header at ELF_header+32
%define ELF_NUM_SEGMENTS                0x2C            ; Number of segments at ELF_header+44
%define ELF_SH_ENTRY_SIZE               0x2E            ; Section header entry size at ELF_header+46
%define ELF_NUM_SH_ENTRIES              0x30            ; Number of section header entries at ELF_header+48

; ELF section header information
%define ELF_SH_TYPE                     0x04            ; Section type
%define ELF_SH_FLAGS                    0x08            ; Section flags
%define ELF_SH_ADDRESS                  0x08            ; Section address
%define ELF_SH_SIZE                     0x0C            ; Section size

%define ELF_MAGIC_NUMBER                0x464C457F      ; 0x7F, 'ELF'
%define ELF_ET_EXECUTABLE               0x02
%define ELF_SHF_ALLOC                   0x02


;**********************************************

;*******************************************************************************
;*             80x86 Architecture Information.  Do not modify.                 *
;*******************************************************************************

; Memory info
%define PAGE_SIZE                       4096

; Video constants
%define TEXT_SCREENSTART                0x000B8000
%define TEXT_NUMROWS                    25
%define TEXT_NUMCOLUMNS                 80

%define TEXT_WHITE                      7
%define TEXT_ERROR                      6
%define msgCr                           0x0d
%define msgLf                           0x0a

;**********************************************

; BIOS data area
%define BIOSDATA_SEGMENT                0x40
%define BIOSDATA_TICKCOUNT              0x6c

%define BIOS_CURSOR_PORT1               0x3d4
%define BIOS_CURSOR_PORT2               0x3d5
