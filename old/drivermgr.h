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

// drivermgr.h - driver manager

#ifndef __KTUX_DRIVERMGR_H
#define __KTUX_DRIVERMGR_H

#include <pic.h>
#include <keyboard.h>
#include <timer.h>

#define DRIVER_MAX_MEMORY_REGIONS		100
#define DRIVER_MAX_IO_PORTS				100


// typedefs
typedef enum {
	PIC_DRIVER, KEYBOARD_DRIVER, TIMER_DRIVER
} ktuxDriverType;

typedef struct
{
	ktuxPicDriver *picDriver;
	ktuxKeyboardDriver *keyboardDriver;
	ktuxTimerDriver *timerDriver;
} ktuxDriverManager;


// function prototypes
int ktuxRegisterDriver(ktuxDriverType driverType, void *driver);
int ktuxInitializeDrivers(void);

#endif
