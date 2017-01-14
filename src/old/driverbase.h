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

/*** FILE NO LONGER IN USE.  REMOVED FROM MAKEFILE ***/
/*** FILE NO LONGER IN USE.  REMOVED FROM MAKEFILE ***/
/*** FILE NO LONGER IN USE.  REMOVED FROM MAKEFILE ***/

// driverbase.h

#ifndef __KTUX_DRIVERBASE_H
#define __KTUX_DRIVERBASE_H

typedef struct
{
	// Driver configuration
	int driverInitialized;			// Have we been initialized yet?
	int driverHandlesIRQ;			// Does it implement an IRQ handler?

	/*
	// Hardware resources this driver will manage
	irqNumber irq;
	ioPortNumber ioPorts[DRIVER_MAX_IO_PORTS];		// we'll do a linked list once malloc() is implemented
	void *memoryRegionLow[DRIVER_MAX_MEMORY_REGIONS];
	void *memoryRegionHigh[DRIVER_MAX_MEMORY_REGIONS];
	*/
	
	// Standard function pointers to event handlers
	int (*InitDriver) (void);
	int (*HandleIRQ) (void);

} ktuxStdDriverBase;

#endif
