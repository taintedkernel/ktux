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

// drivermgr.c - driver manager 

//#include <ktux.h>
#include <error.h>
#include <stdio.h>
#include <drivermgr.h>

// dependencies
#include <io.h>

static void *ktuxDriverInits[] = {
	PicDriverInitialize,
	TimerDriverInitialize,
	KeyboardDriverInitialize,
	(void *) -1
};

ktuxDriverManager driverManager = {
	NULL,
	NULL,
	NULL
};

int ktuxInitializeDrivers(void)
{
	int i, initStatus;
	int numErrors = 0;
	int (*driverInit)(void) = NULL;

	for (i=0; ; i++) 
	{
		driverInit = ktuxDriverInits[i];

		if (driverInit == (void *) -1)
			break;

		if (driverInit == NULL)
			continue;

		initStatus = driverInit();

		if (initStatus < 0)
			numErrors++;
	}

	//if (numErrors > 0)
	//	panic(false, "kernel driver error initializing drivers");

	return (numErrors == 0);
}

int ktuxRegisterDriver(ktuxDriverType driverType, void *driver)
{
	if (driver == NULL) {
		//ktuxError(kernel_error, "ktuxRegisterDriver() called with NULL");
		return ERR_NULLPOINTER;
	}

	switch (driverType)	{
	case PIC_DRIVER:
		driverManager.picDriver = (ktuxPicDriver *) driver;
		break;
	case KEYBOARD_DRIVER:
		driverManager.keyboardDriver = (ktuxKeyboardDriver *) driver;
		break;
	case TIMER_DRIVER:
		driverManager.timerDriver = (ktuxTimerDriver *) driver;
		break;
	}

	return 0;
}

int ktuxInitializeHardware(void)
{
	// Initialize PIC
	kprintf("Initializing PIC...");
	driverManager.picDriver->driverBase->InitDriver();
	kprintf("done\n");

	// Initialize timer
	kprintf("Initializing timer...");
	driverManager.timerDriver->driverBase->InitDriver();
	kprintf("done\n");

	// Initialize keyboard
	kprintf("Initializing keyboard...");
	driverManager.keyboardDriver->driverBase->InitDriver();
	kprintf("done\n");
}

/*

Driver initialization routines must do 3 things:
	-Set the driverBase pointer
	-Register the driver with drivermgr.c
	-Set driverInitialized to true

Develop a standard driver interface.

Design in such a way to allow for cross-platform abilitity easily, even though I
do not plan to implement others.

xDrivers should register IO addresses and memory ranges (if static)
-Minimal functions/events drivers should implement:
	xInitialization
	-Loading (implemented in init?)
	-InterruptHandling (if needed)
	-Unloading (needed?)
	xFunctions within their own driver specific to hardware

-Create standard API for registering events

Driver source layout for driver functions:
1. Inline functions
2. Implementations of standard driver functions
3. Extra functions driver implements

*/

