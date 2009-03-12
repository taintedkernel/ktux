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

// memory.c - memory manager

#include <memory.h>
#include <paging.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <pic.h>
#include <math.h>
#include <ktux.h>

// Until paging enabled, use virtual pointers with our temporary GDT segments (base=0x40100000)
// Keep these pointers private to this source
static unsigned int *kernelPageDirectory = (unsigned int *) phys2Virt(P_KERNEL_PAGE_DIRECTORY);
static unsigned int *kernelHeapTable;
static unsigned int *kernelPageTable = (unsigned int *) phys2Virt(P_PAGE_TABLE_KERNEL_1);
static unsigned int *lower4MBPageTable = (unsigned int *) phys2Virt(P_PAGE_TABLE_FIRST_4MB);
static unsigned int *kernelMappedTables;
static unsigned char *kernelHeapEnd = (char *)V_KERNEL_HEAP_ADDR;

static unsigned int *freePageList = (void *)0;
static unsigned int freePageListSize = 0;
static unsigned int freePageListEntries;

unsigned int vKernelStartAddress = KERNEL_VIRT_ADDR;
unsigned int vKernelEndAddress = KERNEL_VIRT_ADDR;
unsigned int vHeapMappedLimit = V_KERNEL_HEAP_ADDR;
unsigned int numKernelPages = 0, numKernelTables = 0;
unsigned int freeMemory = 0, usedMemory = 0;
unsigned int pFreeMemoryStart = 0, pTotalMemory = 0;

//unsigned int master_pagetables_init = 0;
//void *freeHeadNode = NULL;

// Hardware-defined reserved memory regions
// *MUST* be in ascending address order
static rsvd_mem_struct reservedX86Regions[] =
{
	{ "BDA", BDA_RSVD_MEM_START, BDA_RSVD_MEM_END },
	{ "EBDA", EBDA_RSVD_MEM_START, EBDA_RSVD_MEM_END },
	{ "video memory", VIDEO_RSVD_MEM_START, VIDEO_RSVD_MEM_END },
	{ "video BIOS", VIDEO_BIOS_RSVD_MEM_START, VIDEO_BIOS_RSVD_MEM_END },
	{ "ROM BIOS shadow", ROMBIOS_SHADOW_MEM_START, ROMBIOS_SHADOW_MEM_END },
	{ "ROM BIOS", MB_BIOS_RSVD_MEM_START, MB_BIOS_RSVD_MEM_END },
	{ "", 0, 0 }
};


void print_mem_info(systemInfo *sysInfo)
{
	e820MemoryMap *e820Entry = sysInfo->memoryInfo;

	kprintf("BIOS E820 memory map (0x%08X):\n", e820Entry);
	while (e820Entry->base != MEM_E820_END_MAP)
	{
		kprintf("  0x%08X - 0x%08X ", (unsigned long)e820Entry->base,
				max(e820Entry->base+e820Entry->length, 0xFFFFFFFF));
		if (e820Entry->type == MEM_E820_ADDRESS_MEMORY)
			kprintf("(avail)\n");
		else
			kprintf("(rsvd)\n");

		e820Entry++;
	}

	kprintf("%ukb memory detected ", bytes2KB(pTotalMemory));
	kprintf("(%ukb used/%ukb free)\n", bytes2KB(usedMemory), bytes2KB(freeMemory));
	kprintf("virtual kernel memory map:\n");
	kprintf("  .text : 0x%X - 0x%X\n", vKernelStartAddress, vKernelEndAddress);
	kprintf("[dbg] freePageList : 0x%08X - size : 0x%X (%u)\n", freePageList, freePageListSize, freePageListSize);
	kprintf("physical free memory start : 0x%08X\n", pFreeMemoryStart);
	kprintf("memory manager initialized!\n");
}

void init_virt_mem(systemInfo *sysInfo)
{
	unsigned int i=0, j=0, address;
	unsigned int used, lastReservedRegion;

	// Get total amount of system memory
	pTotalMemory = sysInfo->totalMemory;

	// Mark end of kernel
	vKernelEndAddress = sysInfo->kernelEnd;

	// Mark free page list
	freePageList = (unsigned int *) virt2Phys(vKernelEndAddress);
	freePageListSize = pTotalMemory >> PAGING_ENTRY_SHIFT;		// 1/1024th of total RAM (total RAM / 4kb pages * 4 bytes/entry (unsigned int))

	//kprintf("[dbg] vKernelEnd=0x%p, freepageList=0x%p, size=0x%p\n", vKernelEndAddress, freePageList, freePageListSize);

	// Calculate start of free memory
	pFreeMemoryStart = virt2Phys(vKernelEndAddress) + freePageListSize;

	/*** Map our page tables ***/
	// Start off with lower 1mb, kernel code address space

	/* Identity map reserved regions/lower memory (first 1 mb in RAM)
	 * Also identity map physical kernel address space
	 * This is needed right after paging is enabled,
	 * before CS can be updated with new segment of base=0
	 */
	address = 0;
	while (address < pFreeMemoryStart) {
		setPTEindex(lower4MBPageTable, i++, address, (PDE_FLAG_PRESENT | PDE_FLAG_RW | PDE_FLAG_KERNEL));
		//lower4MBPageTable[i++] = makePTEphys(address, (PDE_FLAG_PRESENT | PDE_FLAG_RW));	// Supervisor, read/write, present (011 b)
		address += PAGE_SIZE;
	}

	// Mark rest of page table not present
	while (i % PTE_PER_PT > 0) {
		setPTEindex(lower4MBPageTable, i++, 0, (PDE_FLAG_RW | PDE_FLAG_KERNEL));
		//lower4MBPageTable[i++] = makePTEphys(0, PDE_FLAG_RW);								// Supervisor, read/write, not present (010 b)
		address += PAGE_SIZE;
	}

	// Map kernel [code] virtual address space
	i = 0; address = KERNEL_PHYS_ADDR;
	while (address < pFreeMemoryStart) {
		setPTEindex(kernelPageTable, i++, address, (PDE_FLAG_PRESENT | PDE_FLAG_RW | PDE_FLAG_KERNEL));
		//kernelPageTable[i++] = makePTEphys(address, (PDE_FLAG_PRESENT | PDE_FLAG_RW));		// Supervisor, read/write, present (011 b)
		address += PAGE_SIZE;
	}

	// Calculate number of kernel page tables needed/used
	numKernelPages = i - 1;
	numKernelTables = (i - 1) >> PAGING_ENTRY_SHIFT;

	// Mark rest of kernel page table not present
	while (i % PTE_PER_PT > 0) {
		setPTEindex(kernelPageTable, i++, 0, PDE_FLAG_RW);
		//kernelPageTable[i++] = makePTEphys(0, PDE_FLAG_RW);								// Supervisor, read/write, not present (010 b)
		address += PAGE_SIZE;
	}

	/*** Map our page directories ***/
	// Map entry 0 to lower 1mb page directory (identity mapped)
	// Map entry for kernel virtual address space
	// Map all others not present

	// First our lower 1mb
	i=0;
	setPDEindex(kernelPageDirectory, i++, P_PAGE_TABLE_FIRST_4MB, (PDE_FLAG_PRESENT | PDE_FLAG_RW | PDE_FLAG_KERNEL));
	//kernelPageDirectory[i++] = makePDEphys(P_PAGE_TABLE_FIRST_4MB, (PDE_FLAG_PRESENT | PDE_FLAG_RW));

	// Mark rest of page directory not present (initalize them)
	while (i % PDE_PER_PD > 0) {
		setPDEindex(kernelPageDirectory, i++, 0, PDE_FLAG_RW);
		//kernelPageDirectory[i++] = makePDEphys(0, PDE_FLAG_RW);
	}

	// Map page tables for the kernel in the directory
	map_ktables(kernelPageDirectory);

	/*** Enable paging! ***/
	enable_paging((unsigned int *)P_KERNEL_PAGE_DIRECTORY);

	// Update our paging data structure pointers
	kernelPageDirectory = (unsigned int *) P_KERNEL_PAGE_DIRECTORY;
	kernelPageTable = (unsigned int *) P_PAGE_TABLE_KERNEL_1;
	lower4MBPageTable = (unsigned int *) P_PAGE_TABLE_FIRST_4MB;
	//kernelTables = (unsigned int *) V_KERNEL_PAGE_TABLE_1;

	// Find number of entries in reserved region table
	for (i=0; reservedX86Regions[i].end > 0; i++);
	lastReservedRegion = i - 1;

	// Create our free page stack list //
	// This can probably be heavily optimized but its good for now
	for ( i=0,address=0; address<pTotalMemory; address+=PAGE_SIZE )
	{
		// Start comparing address to all known reserved regions //
		// Add only if not found used anywhere //
		used = 0;

		// Address in the last "e820 available" entry?
		if ( pFreeMemoryStart < address && address < pTotalMemory - 1 ) {
			freePageList[i++] = address;
			continue;
		}

		// See if it's in use by our kernel (static, not heap)
		/*if (vKernelStartAddress < address + PAGE_SIZE - 1 &&
			pFreeMemoryStart-1 > address)
			{ continue; }
			???
			*/

		// Kernel stack
		if ( P_KERNEL_STACK_ADDR < address &&
				address < P_KERNEL_STACK_ADDR + KERNEL_STACK_SIZE - 1 )
			{ continue; }

		// Scan list of reserved memory regions
		if ( address < reservedX86Regions[lastReservedRegion].end )
		{
			for ( j=0; reservedX86Regions[j].end > 0; j++ ) {
				if ( reservedX86Regions[j].start < address &&
						address < reservedX86Regions[j].end - 1 ) {
					//kprintf("in use (reserved), skipping\n");
					used=1; break;
				}
			}
		}

		// Kernel page tables/directories
		if ( address < P_START_FREE_RAM ) { continue; }

		// Add to list
		if (used == 0) {
			//kprintf("not in use, adding to list\n");
			freePageList[i++] = address;
		}
	}

	// Mark number of free pages and calculate memory usage
	freePageListEntries = i-1;
	freeMemory = freePageListEntries << PAGE_SIZE_SHIFT;
	usedMemory = pTotalMemory - freeMemory;
}

void init_heap()
{
	/* Initialize a kernel heap
	 *
	 * First set kernel page directory entry for virtual heap address to a
	 * pre-reserved physical memory region for initial page table.
	 *
	 * This lies < 4mb, therefore is identity-mapped in memory for easy access.
	 */
	kprintf("init_heap() : ");

	// First, initialize dynamic memory //
	// Create kernel page directory entry with our new initial 4mb heap table
	//kernelPageDirectory[virt2Dir(V_KERNEL_HEAP_ADDR)] = makePDEphys(P_PAGE_TABLE_HEAP_1, (PDE_FLAG_PRESENT | PDE_FLAG_RW));
	setPDEaddr(kernelPageDirectory, V_KERNEL_HEAP_ADDR, P_PAGE_TABLE_HEAP_1, (PDE_FLAG_PRESENT | PDE_FLAG_RW | PDE_FLAG_KERNEL));

	// Allocate and initialize the table with one page of data
	// Address < 4mb, identity mapped -> physical = virtual
	kernelHeapTable = (unsigned int *) P_PAGE_TABLE_HEAP_1;
	setPTEindex(kernelHeapTable, 0, alloc_page(), (PDE_FLAG_PRESENT | PDE_FLAG_RW | PDE_FLAG_KERNEL));
	//kernelHeapEnd += PAGE_SIZE;
	//kernelHeapTable[0] = makePTEphys(alloc_page(), (PDE_FLAG_PRESENT | PDE_FLAG_RW));

	kprintf("heap table mapped at 0x%X, page 0 mapped to physical 0x%X\n", kernelHeapTable, kernelHeapTable[0]);

	// Keep a page table for our mapped page tables //
	// Initialize page directory with new table at physical address P_MAPPED_PTABLE_1 (identity mapped 1st 4mb)
	setPDEaddr(kernelPageDirectory, V_KERNEL_PAGE_TABLE_1, (unsigned int)kernelPageDirectory, (PDE_FLAG_PRESENT | PDE_FLAG_RW | PDE_FLAG_KERNEL));

/// check it
/*	int i
	unsigned int *tmp;
	tmp = (unsigned int *)virt2Map(KERNEL_VIRT_ADDR);
	for (i=0; i<10; i++)
	kprintf("k:0x%p ", tmp[i]);
	kprintf("\n");

	tmp = (unsigned int *)virt2Map(V_KERNEL_HEAP_ADDR);
	for (i=0; i<10; i++)
	kprintf("h:0x%p ", tmp[i]);
	kprintf("\n");*/

	/*kernelMappedTables = (unsigned int *) P_MAPPED_PTABLE_1;
	set_mapped_table_entry(0);
	set_mapped_table_entry(KERNEL_VIRT_ADDR);
	set_mapped_table_entry(V_KERNEL_HEAP_ADDR);*/
}

void page_fault_handler(unsigned int address, unsigned int errorCode, unsigned int eip)
{
	unsigned int debug=1;
	unsigned int pageDir, *pageTable;

	pageDir = kernelPageDirectory[virt2Dir(address)];
	pageTable = (unsigned int *)(pageDir & PDE_ADDR_MASK);

	if (debug) kprintf("page fault: address=%p referenced at kernel EIP 0x%p\n", address, eip);
	if (debug) kprintf("  kernel page directory entry=0x%p\n", pageDir);
	if (debug) kprintf("  kernel page table @ 0x%p; page_table[0x%p]=0x%p\n", pageTable, virt2Page(address), pageTable[virt2Page(address)]);

	if (errorCode & PDE_FLAG_PRESENT)
	{
		if (debug) kprintf("  fault caused by page-level protection violation.  kill thread not implemented, halting\n");
		while(1);
	}

	if (address < KERNEL_VIRT_ADDR) {
		if (debug) kprintf("  address below kernel\n");
	}
	else if (address < V_KERNEL_HEAP_ADDR) {
		if (debug) kprintf("  address in kernel space!\n");
	}
	else if (address < V_KERNEL_DRIVER_HEAP) {
		// Is request valid?  Must be below end of heap.
		if (address < (unsigned int)kernelHeapEnd) {
			//kprintf("write code to allocate and map new page here\n");
			setPTEaddr(pageTable, address, alloc_page(), (PDE_FLAG_PRESENT | PDE_FLAG_RW | PDE_FLAG_KERNEL));

			if (debug) kprintf("  updated: kernel page table @ 0x%p; page_table[0x%p]=0x%p\n", pageTable, virt2Page(address), pageTable[virt2Page(address)]);
			return;
		}
		else {
			if (debug) kprintf("  heap request invalid.  kill thread.\n");
			//KILL_THREAD
		}
	}

/*	if ((V_KERNEL_PAGE_TABLE_1 <= address) && (address < V_KERNEL_PAGE_TABLE_TABLE))
	{
	}*/

	while(1);
}


/************** VIRTUAL MEMORY MANAGEMENT *******************/
/* This code basically maintains our virtual address space for
 * applications through the page tables.  These functions map
 * virtual addresses to physical pages in memory. */

// Nice and simple for now
void kfree(void *address)
{
	return;
}

/* Ah, yes.  Can't have much of a memory manager without malloc() */
void *kmalloc(unsigned int nbytes)
{
	unsigned int debug = 1;
	static unsigned int freeBytes = 0; //PAGE_SIZE;
	//union align { double d; unsigned u; void (*f)(void); } align;

	//if (debug) kprintf("[dbg] kmalloc(0x%X) : free=0x%X, heapEnd/Limit=0x%X/0x%X\n", nbytes, freeBytes, kernelHeapEnd, vHeapMappedLimit);

	//nbytes = (nbytes +
	if (nbytes <= freeBytes)
	{
		//if (debug) kprintf("[dbg] kmalloc1.pre called\n");
		void *p = kernelHeapEnd;
		kernelHeapEnd += nbytes;
		freeBytes -= nbytes;
		memsetd(p, 0, nbytes >> 2);
		if (debug) kprintf("[dbg] kmalloc1(0x%X).post return(0x%X) : heapEnd/Limit=0x%X/0x%X, freeBytes=%d\n", nbytes, p, kernelHeapEnd, vHeapMappedLimit, freeBytes);
		return p;
	}
	else if (nbytes > PAGE_SIZE)
	{
		void *p = sbrk(nbytes);
		if (p == (void *)-1)
			return 0;
		kernelHeapEnd = (char*)(p + nbytes);
		memsetd(p, 0, nbytes >> 2);
		if (debug) kprintf("[dbg] kmalloc2(0x%X) return(0x%X) : heapEnd/Limit=0x%X/0x%X, freeBytes=%d\n", nbytes, p, kernelHeapEnd, vHeapMappedLimit, freeBytes);
		return p;
	}
	else
	{
		//if (debug) kprintf("[dbg] kmalloc3.pre called\n");
		void *p = sbrk(nbytes);
		if (p == (void *)-1)
			return 0;
		freeBytes += nbytes;
		kernelHeapEnd = p;
		if (debug) kprintf("[dbg] kmalloc3(0x%X) : heapEnd/Limit=0x%X/0x%X, freeBytes=%d\n", nbytes, kernelHeapEnd, vHeapMappedLimit, freeBytes);
		return (void *)kmalloc(nbytes);
	}
}

void *sbrk(unsigned int nbytes)
{
	void *p;
	static unsigned int i, free;
	static unsigned int newHeapPage, newPageVirt;

	unsigned int debug=1;

	if ((vHeapMappedLimit & (PAGE_SIZE - 1)) == 0)
		free = 0;
	else
		free = (vHeapMappedLimit & ~(PAGE_SIZE - 1)) + PAGE_SIZE - vHeapMappedLimit;

	if (debug) kprintf("[dbg] sbrk(0x%X) : 0x%X until EOP (heapMapLimit=0x%X)\n", nbytes, free, vHeapMappedLimit);

	// TEST THROUGHLY
	if (free < nbytes) {
		i = 0;
		while (free < nbytes) {
			newHeapPage = alloc_page();
			newPageVirt = (vHeapMappedLimit & ~(PAGE_SIZE-1)) + PAGE_SIZE*(i++);

			//kprintf("calling map_kpage(0x%p, 0x%p)\n", tmpVirt, tmpPage);
			map_page_helper(kernelPageDirectory, newPageVirt, newHeapPage, (PDE_FLAG_PRESENT | PDE_FLAG_RW | PDE_FLAG_KERNEL));
			free += PAGE_SIZE;
		}
	}

	p = (void *)vHeapMappedLimit;
	vHeapMappedLimit += nbytes;
	free -= nbytes;

	if (debug) kprintf("[dbg] sbrk return(0x%X) : heapMapLimit=0x%X, 0x%X until EOP\n", p, vHeapMappedLimit, free);

	return p;
}

/*void set_mapped_table_entry(unsigned int virt)
{
	if (master_pagetables_init)
		kernelMappedTables[virt2Dir(virt)] = kernelPageDirectory[virt2Dir(virt)];
	else
		kprintf("init_master_pagetables not initialized!");
}*/

/* void mapPage(unsigned int *pageDir, unsigned int phys, unsigned int virt, unsigned int flags)
 *
 * This funtion takes one page (4k) of data at physical address 'phys' and
 * maps it to the page directory 'pageDir' at virtual address 'virt'.  If
 * the corresponding page table is not already present, a new one will be
 * allocated and initialized. */

/* These should really be hand-rolled into assembly for speed */
inline void unmap_kpage(unsigned int virt)
{
	unmap_page_helper(kernelPageDirectory, virt);
}

/*void unmap_page(unsigned int pid, unsigned int virt)
{
	unmap_page_helper((unsigned int *)process_get_page_directory(pid), virt);
}*/

void unmap_page_helper(unsigned int *pageDir, unsigned int virt)
{
	unsigned int *pageTable;

	pageTable = (unsigned int *)(pageDir[virt2Dir(virt)] & PDE_ADDR_MASK);
	pageTable[virt2Page(virt)] = makePTEphys(0, PDE_FLAG_RW);
}

inline void map_kpage(unsigned int virt, unsigned int phys, unsigned int flags)
{
	map_page_helper(kernelPageDirectory, virt, phys, flags);
}

/*void map_page(unsigned int pid, unsigned int virt, unsigned int phys, unsigned int flags)
{
	map_page_helper((unsigned int *)process_get_page_directory(pid), virt, phys, flags);
}*/

void map_page_helper(unsigned int *pageDir, unsigned int virt, unsigned int phys, unsigned int flags)
{
	unsigned int debug = 0;
	unsigned int pageDirEntry;
	unsigned int *pageTable;

	pageDirEntry = pageDir[virt2Dir(virt)];
	pageTable = (unsigned int *)virt2Map(virt);

	if (debug) kprintf("[dbg] map_page_helper(page_dir=0x%p, virt=0x%p, phys=0x%p, flags=0x%p)\n", pageDir, virt, phys, flags);
	if (debug) kprintf("[dbg]     page table @ 0x%p (w/flags), virtual @ 0x%p\n", pageDirEntry, pageTable);

	cli();

	// Page table we need doesn't exist, create it
	if ( !(pageDirEntry & PDE_FLAG_PRESENT) )
	{
		pageTable = create_new_table(pageDir, virt);
		pageDirEntry = pageDir[virt2Dir(virt)];

/*		// Temporarily map new table to virt
		map_page_helper(pageDir, virt, pageTable, (PDE_FLAG_PRESENT | PDE_FLAG_RW));
		pageTable = (unsigned int *)virt;
		pageTable[virt2Page(virt)] = makePTEphys(alloc_page(), flags);
*/
		if (debug) kprintf("[dbg] created new table @ 0x%p for virt 0x%p\n", pageTable, virt);
		if (debug) kprintf("[dbg] new pageDirEntry=0x%p\n", pageDirEntry);
		map_page_helper(pageDir, virt, phys, flags);
	}
	else
	{
		/*if ((pageDirEntry & PDE_ADDR_MASK) == 0) {
			kprintf("loop %d (0x%p)", pageDirEntry);
			while (1);
		}*/
		setPTEaddr(pageTable, virt, phys, flags);
		//pageTable[virt2Page(virt)] = makePTEphys(phys, flags);

		if (pageDir == (unsigned int *)P_KERNEL_PAGE_DIRECTORY)
			invalidate(virt);
	}

	sti();
}

// This only works with whole pages (size 4k aligned)
void map_page_range(unsigned int *pageDir, unsigned int virt, unsigned int phys, unsigned int size, unsigned int flags)
{
	unsigned int currentPage;
	unsigned int numPages = size >> PAGE_SIZE_SHIFT;

	if ((size & 0xFFF) != 0)
		return;

	for (currentPage=0; currentPage<numPages; currentPage++) {
		map_page_helper(pageDir, virt+(currentPage << PAGE_SIZE_SHIFT),
			phys+(currentPage << PAGE_SIZE_SHIFT), flags);
	}
}

// Map the kernel's (static) page tables into a page directory
void map_ktables(unsigned int *pageDir)
{
	unsigned int i, address = P_PAGE_TABLE_KERNEL_1;

	//mapPageRange(pageDir, address

	for (i=0; i<=numKernelTables; i++) {
		setPDEindex(pageDir, i + virt2Dir(KERNEL_VIRT_ADDR), address, (PDE_FLAG_PRESENT | PDE_FLAG_RW | PDE_FLAG_KERNEL));
		//pageDir[i + virt2Dir(KERNEL_VIRT_ADDR)] = makePDEphys(address, (PDE_FLAG_PRESENT | PDE_FLAG_RW));
		//pageDir[virt2PageTableDir(address)] = makePDEphys(address, (PDE_FLAG_PRESENT | PDE_FLAG_RW));
		address += PAGE_SIZE;
	}

	//reload_cr3();
}

// Allocates new page table.  Returns virtual address of table.
unsigned int *create_new_table(unsigned int *pageDir, unsigned int virt)
{
	unsigned int *pageTableP, *pageTableV;

	pageTableP = (unsigned int *)alloc_page();
	pageTableV = (unsigned int *)virt2Map(virt);

	kprintf("alloc new pageTable at 0x%p for virt 0x%p\n", pageTableP, virt);
	kprintf("virt2PageTable(0x%p)=0x%p\n", virt, pageTableV);

	// Update page directory with new table
	setPTEaddr(pageDir, virt, (unsigned int)pageTableP, (PDE_FLAG_PRESENT | PDE_FLAG_RW | PDE_FLAG_KERNEL));
	//pageDir[virt2Dir(virt)] = makePDEphys((unsigned int)pageTableP, (PDE_FLAG_PRESENT | PDE_FLAG_RW));
	//pageDir[virt2PageTableDir(virt)] = makePDEphys((unsigned int)pageTableP, (PDE_FLAG_PRESENT | PDE_FLAG_RW));

/*		// Temporarily map new table to virt
		map_page_helper(pageDir, virt, pageTable, (PDE_FLAG_PRESENT | PDE_FLAG_RW));
		pageTable = (unsigned int *)virt;
		pageTable[virt2Page(virt)] = makePTEphys(alloc_page(), flags);
*/
	//pageTableV[virt2Page(virt)] = makePDEphys((unsigned int));
	kprintf("returning with 0x%p\n", pageTableV);
	invalidate(virt);

	return pageTableV;
}


/***** PHYSICAL MEMORY MANAGEMENT *****/
/* This section is brief, it mainly consists of a page allocate
 * and de-allocator (free-er).  Since we maintain page list structures
 * as a stack, the code is minimal
 */

// Physical page allocater
// NULL/zero is failure (no free RAM)
// Notice no page table handling code here
unsigned int alloc_page()
{
	static unsigned int address;

	// Running before initialization
	if (pTotalMemory == 0)
		return 0;

	// No free memory!
	if (freePageListEntries == 0)
		return 0;

	address = freePageList[freePageListEntries];

	if (address > 0)
	{
		freePageList[freePageListEntries--] = 0;
		usedMemory -= PAGE_SIZE;
	}

	return address;
}

// Physical page deallocater
unsigned int free_page(unsigned int address)
{
	// Running before initialization
	if (pTotalMemory == 0)
		return ERROR;

	// implement error
	if ((address & 0xFFF) != 0)
		return ERROR;

	// *TODO* : Implement error/bounds checking
	freePageList[freePageListEntries++] = address;
	usedMemory += PAGE_SIZE;

	return 0;
}

static void dumpFreePageList(unsigned int start, unsigned int count)
{
	unsigned int i;

	for (i=start; i<start+count; i++) {
		kprintf("freePageList[%d]=0x%p\n", i, freePageList[i]);
	}
}
