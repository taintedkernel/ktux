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

// timer.c - 8253 driver

#include <ktux.h>
#include <timer.h>
#include <interrupts.h>

// dependencies
#include <io.h>
#include <stdio.h>
#include <video.h>


static long int tick = 0;
//static unsigned long *oldeip = (void *)0;

/*static ktuxTimerDriver timerDriver;
static ktuxStdDriverBase timerDriverBase = {
	false,						// Driver initialized (always default to false)
	true,						// Driver implements IRQ?
	init_timerialize,			// InitDriver() function pointer
	TimerIRQHandler				// HandleIRQ() function pointer
};

int TimerDriverInitialize(void)
{
	// Our basic driver init function
	timerDriver.driverBase = &timerDriverBase;

	// Register driver with kernel
	ktuxRegisterDriver(TIMER_DRIVER, &timerDriver);

	// Driver init flag = true
	timerDriver.driverBase->driverInitialized = true;

	return 0;
}*/

int init_timer(void)
{
	return 0;
}

unsigned int TimerIRQHandler(unsigned int oldESP)
{
	tick++;
	putch('.');
	return oldESP;
}

void reprogram_timer(unsigned int freq)
{
/* I can remember the NTSC TV color burst frequency, but not the PC
peripheral clock. Fortunately, they are related: */
	unsigned short foo = (3579545L / 3) / freq;
/**/

/* reprogram the 8253 timer chip to run at 'HZ', instead of 18 Hz */
	outportb(0x43, 0x36);	/* channel 0, LSB/MSB, mode 3, binary */
	outportb(0x40, foo & 0xFF);	/* LSB */
	outportb(0x40, foo >> 8);	/* MSB */
}
