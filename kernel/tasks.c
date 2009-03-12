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

// tasks.c - external tasks/threads

#include <tasks.h>
#include <process.h>
#include <video.h>
#include <stdio.h>
#include <debug.h>

//extern console _vc[];
console tty1, tty2;

/* we can get away without using volatile
 * because we are using inline asm and
 * do the inc with indirect addressing (ie:
 * not updating a local/register copy of the
 * variable)  using var++ in C would not work
 * without the volatile keyword
 */
static unsigned int foo;
static unsigned int bar;
static unsigned int gamma;

/*int volatile write(unsigned char *str)
{
	//DEBUG_BP
	unsigned i=0;

	for(; *str!='\0'; str++)
	//for(i = 0; i<len; i++)
	{
		putch(*str);
	}

	return i;
}*/

volatile void thread_foo()
{
	//DEBUG_BP
	for (foo=0;;)
	{
		asm volatile ("movl %1, %%eax;"
			"incl (%%eax);"
			: "=r"(foo)
			: "r"(&foo)
			: "%eax" );
	}
}

volatile void thread_bar()
{
	//nice(16);

	//DEBUG_BP
	for (bar=0;;)
	{
		asm volatile ("movl %1, %%eax;"
				//"nop;"
			"incl (%%eax);"
			: "=r"(bar)
			: "r"(&bar)
			: "%eax" );
	}
}

volatile void thread_gamma()
{
	nice(-63);			// we are important like the preccioussss

	//DEBUG_BP
	for (gamma=0;;)
	{
		asm volatile ("movl %1, %%eax;"
				//"nop;"
			"incl (%%eax);"
			: "=r"(gamma)
			: "r"(&gamma)
			: "%eax" );
	}
}

void thread_monitor()
{
	unsigned short y;

	kprintf("counting threads:\n");
	y = get_csr_y();

	while(1) {
		move_cursor(0,y);
		//kprintf("%u\t%u", foo, bar);
		kprintf("%u\t%u\t%u", foo, bar, gamma);
		yield();
	}
}
