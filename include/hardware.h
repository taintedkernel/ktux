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

// hardware.h - hardware manager

#ifndef __KTUX_HARDWARE_H
#define __KTUX_HARDWARE_H

typedef volatile struct {
	unsigned short type;
	unsigned int maxFunc;					// Zero if does not support CPUID
	unsigned char vendorString[13];			// 0000_0000: EBX-EDX (vendor id string)
	unsigned int signature;					// 0000_0001: EAX (processor family/model/stepping)
	unsigned int brand;						// 0000_0001: EBX (brand)
	unsigned int features2;					// 0000_0001: ECX (feature flags 2)
	unsigned int features;					// 0000_0001: EDX (feature flags)
	unsigned int maxExtFunc;				// Mostly on non-Intel chips only
	unsigned int extSignature;				// 8000_0001: EAX (processor family/model/stepping)
	unsigned int extBrand;					// 8000_0001: EBX (brand ID)
	unsigned int extFeatures2;				// 8000_0001: ECX (ext feature flags 2)
	unsigned int extFeatures;				// 8000_0001: EDX (ext feature flags)
	unsigned char extName[48];				// 8000_0002-4 EAX-EDX (processor name string)
	unsigned int extTLB_Info;				// 8000_0005: EBX (4 KB L1 TLB configuration descriptor)
	unsigned int extL1D_Cache;				// 8000_0005: ECX (data L1 cache configuration descriptor)
	unsigned int extL1I_Cache;				// 8000_0005: EDX (code L1 cache configuration descriptor)
	unsigned int extL2_Cache;				// 8000_0006: ECX (unified L2 cache configuration descriptor)
	unsigned int TSCLoWord;					
	unsigned int TSCHiWord;
	unsigned int freq;
} __attribute__ ((packed)) cpuInfo;

typedef volatile struct {
	unsigned long long base;
	unsigned long long length;
	unsigned int type;
} __attribute__ ((packed)) e820MemoryMap;

typedef volatile struct {
	unsigned short bootDrive;				// As passed by BIOS: 0x0=floppy,0x80=hd,etc
	unsigned short bootFileSystem;
	void *bootFATinfo;						// Pointer to FAT BPB
	unsigned int kernelEnd;					// In Kb
	cpuInfo *cpuInfo;
	unsigned long totalMemory;				// In bytes
	e820MemoryMap *memoryInfo;				// match ../osloader/kloader.h
} __attribute__ ((packed)) systemInfo;


void displayCPUInfo(cpuInfo*);

#endif
