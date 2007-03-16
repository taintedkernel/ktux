/***************************************************************************
 *   Copyright (C) 2004 by Anthony DeChiaro                                *
 *   axd6491@njit.edu                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

// memory.h - memory manager

#ifndef __KTUX_MEMORY_H
#define __KTUX_MEMORY_H

#include <hardware.h>		// systemInfo*


#define NULL									(void *)0

#define MEMORY_PAGING							1
#define MEMORY_HEAP								2


// Kernel physical memory address - *MUST* be 4k aligned! //
// List these in ascending order //
#define KERNEL_PHYS_ADDR						0x00100000
#define KERNEL_STACK_SIZE						0x00001000
#define P_KERNEL_STACK_ADDR						0x00003000
#define P_KERNEL_PAGE_DIRECTORY					0x00004000
// page tables
#define P_PAGE_TABLE_FIRST_4MB					0x00005000			// WARNING: If kernel grows beyond 4mb (larger then 3mb) this will be a problem
#define P_PAGE_TABLE_KERNEL_1					0x00006000
#define P_PAGE_TABLE_HEAP_1						0x00007000
#define P_MAPPED_PTABLE_1						0x00008000
// start of free ram
#define P_START_1MB_FREE_RAM					0x00009000


// Kernel virtual memory addresses - *MUST* be 4k aligned! //
#define KERNEL_VIRT_ADDR						0xC0000000

#define V_KERNEL_HEAP_ADDR						0xD0000000
#define V_KERNEL_DRIVER_HEAP					0xE0000000
#define V_KERNEL_MISC							0xF0000000
#define V_KERNEL_PAGE_TABLE_1					0xF0000000
#define V_KERNEL_PAGE_TABLE_TABLE				0xF0400000			// V_KERNEL_PAGE_TABLE_1 + 4mb
//#define KERNEL_HEAP_ADDR						0xD0000000
//#define KERNEL_DRIVER_HEAP					0xE0000000
//#define KERNEL_MISC							0xF0000000

#define USER_STACK								0xFFFFD000
#define USER_PAGE_DIRECTORY						0xFFFFE000
#define USER_PAGE_TABLE_1						0xFFCFE000
//#define USER_PAGE_TABLE_1						0xFFCFE000
//#define USER_STACK							0xFFFFD000
//#define USER_PAGE_DIRECTORY					0xFFFFE000


// 80x86 arch memory management //

// page table conversion macros
#define virt2Phys(addr)							( addr - KERNEL_VIRT_ADDR + KERNEL_PHYS_ADDR )
#define phys2Virt(addr)							( addr + KERNEL_VIRT_ADDR - KERNEL_PHYS_ADDR )
#define virt2Dir(addr)							( addr >> DIR_ADDR_SHIFT )		// Divide by 4mb
#define virt2Page(addr)							(( addr >> TABLE_ADDR_SHIFT ) & TABLE_ADDR_MASK )		// Offset within a page
#define virt2Map(addr)							(( addr >> DIR_ADDR_SHIFT << TABLE_ADDR_SHIFT ) + V_KERNEL_PAGE_TABLE_1 )
//#define virt2PageTableDir(addr)					((( addr >> DIR_ADDR_SHIFT << TABLE_ADDR_SHIFT ) + V_KERNEL_PAGE_TABLE_1 ) >> DIR_ADDR_SHIFT )

// page table entry macros
#define makePDEphys(physAddr, flags)			((physAddr & PDE_ADDR_MASK) | (flags & (~PDE_ADDR_MASK)))
#define makePTEphys(physAddr, flags)			((physAddr & PTE_ADDR_MASK) | (flags & (~PTE_ADDR_MASK)))

#define setPDEaddr(PD, virt, phys, flags)		PD[virt2Dir(virt)] = makePDEphys(phys, flags)
#define setPDEindex(PD, index, phys, flags)		PD[index] = makePDEphys(phys, flags)
#define setPTEaddr(PT, virt, phys, flags)		setPDEindex(PT, virt2Page(virt), phys, flags)
#define setPTEindex(PT, index, phys, flags)		setPDEindex(PT, index, phys, flags)

// old //
//#define virt2Page(addr)						(( addr & (( 1 << DIR_ADDR_SHIFT ) - 1 )) >> PAGE_SIZE_SHIFT ) 
//#define makePTEvirt(linearAddr, flags)		((phys2Virt(linearAddr) & PTE_ADDR_MASK) | (flags & (~PTE_ADDR_MASK)))
//#define makePDEvirt(linearAddr, flags)		((phys2Virt(linearAddr) & PDE_ADDR_MASK) | (flags & (~PDE_ADDR_MASK)))
// old //

// paging constants
#define PAGING_ENTRY_SHIFT						10								// 1024 (2^10) entries per page table/directory
#define PDE_PER_PD								(1 << PAGING_ENTRY_SHIFT)
#define PTE_PER_PT								(1 << PAGING_ENTRY_SHIFT)
#define PAGE_SIZE_SHIFT							12								// 4096 (2^12) bytes per page
#define PAGE_SIZE								(1 << PAGE_SIZE_SHIFT)

#define PDE_ADDR_MASK							0xFFFFF000
#define PDE_FLAG_PRESENT						0x001
#define PDE_FLAG_RW								0x002
#define PDE_FLAG_USER							0x004

#define PTE_ADDR_MASK							PDE_ADDR_MASK
#define PTE_FLAG_PRESENT						PDE_FLAG_PRESENT
#define PTE_FLAG_RW								PDE_FLAG_RW
#define PTE_FLAG_USER							PDE_FLAG_USER

#define DIR_ADDR_SHIFT							22
#define DIR_ADDR_MASK							0x3FF
#define TABLE_ADDR_SHIFT						12
#define TABLE_ADDR_MASK							0x3FF

// page table entry test macros
#define isPresent(flag)							(flag & (PDE_FLAG_PRESENT))
#define isReadWrite(flag)						(flag & PDE_FLAG_RW)
#define isUser(flag)							(flag & PDE_FLAG_USER)
#define regionOverlap(addr, size, regBeg, regEnd)		(regBeg < addr+size-1 && regEnd-1 > addr)

// 80x86 reserved memory regions (physical)
#define BDA_RSVD_MEM_START						0x00000000
#define BDA_RSVD_MEM_END						0x000004FF
#define	EBDA_RSVD_MEM_START						0x0009FC00
#define EBDA_RSVD_MEM_END						0x0009FFFF
#define VIDEO_RSVD_MEM_START					0x000A0000
#define VIDEO_RSVD_MEM_END						0x000BFFFF
#define VIDEO_BIOS_RSVD_MEM_START				0x000C0000
#define VIDEO_BIOS_RSVD_MEM_END					0x000C7FFF
#define ROMBIOS_SHADOW_MEM_START				0x000C8000
#define ROMBIOS_SHADOW_MEM_END					0x000EFFFF
#define MB_BIOS_RSVD_MEM_START					0x000F0000
#define MB_BIOS_RSVD_MEM_END					0x000FFFFF
// end x86 stuff


typedef volatile struct
{
	unsigned char title[20];
	unsigned int start;
	unsigned int end;
} rsvd_mem_struct;


// standard functions
void print_mem_info(void);
void init_phys_mem(systemInfo*);
void init_virt_mem(void);
void page_fault_handler(unsigned int, unsigned int, unsigned int);

// virtual memory management
void *kmalloc(unsigned int nbytes);
void *sbrk(unsigned int nbytes);
//void set_mapped_table_entry(unsigned int virt);
void unmap_kpage(unsigned int virt);
//void unmap_page(unsigned int pid, unsigned int virt);
void unmap_page_helper(unsigned int *pageDir, unsigned int virt);
void map_kpage(unsigned int virt, unsigned int phys, unsigned int flags);
//void map_page(unsigned int pid, unsigned int virt, unsigned int phys, unsigned int flags);
void map_page_helper(unsigned int *pageDir, unsigned int virt, unsigned int phys, unsigned int flags);
void map_page_range(unsigned int *pageDir, unsigned int virt, unsigned int phys, unsigned int size, unsigned int flags);
void map_ktables(unsigned int *pageDir);
unsigned int *create_new_table(unsigned int *pageDir, unsigned int virt);

// physical memory management
unsigned int alloc_page(void);
unsigned int free_page(unsigned int);

#endif





















/*
#define PDE_FLAG_PWT							0x008
#define PDE_FLAG_PCD							0x010
#define PDE_FLAG_A								0x020
#define PDE_FLAG_PS								0x080
#define PDE_FLAG_G								0x100
*/

/*
#define PTE_FLAG_PWT							PDE_FLAG_PWT
#define PTE_FLAG_PCD							PDE_FLAG_PCD
#define PTE_FLAG_A								PDE_FLAG_A
#define PTE_FLAG_D								0x040
#define PTE_FLAG_PAT							0x080
#define PTE_FLAG_G								PDE_FLAG_G
*/

/*
struct pde_s {
    u32 flags		: 9;
    u8  avail		: 3;
    u32 pte_addr	: 20;
};

struct pte_s {
    u32 flags		: 9;
    u8  avail		: 3;
    u32 pg_addr		: 20;
};

#define pde_v(x)						 		(struct pde_s)(x)
#define pte_v(x)						 		(struct pte_s)(x)
#define pde_p(x)						 		(struct pde_s *)(x)
#define pte_p(x) 								(struct pte_s *)(x)
*/
