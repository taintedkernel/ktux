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
#include <video.h>
#include <stdio.h>

//extern console *_curr_vc;
extern console _vc[];

console tty1;
console tty2;

//unsigned short buf_size=32;
//char *uf_ptr=(void *)0;

unsigned int t1;
unsigned int t2;

int volatile write(unsigned char *str)
{
	//asm volatile("xchg %bx, %bx");
	unsigned i=0;

	for(; *str!='\0'; str++)
	//for(i = 0; i<len; i++)
	{
		putch(*str);
	}

	return i;
}

volatile void thread_one()
{
	//asm volatile("xchg %bx, %bx");
	for (t1=0;;)
	{
		asm volatile ("movl %1, %%eax;"
		     "incl (%%eax);"
			:"=r"(t1)
			:"r"(&t1)
			:"%eax");
	}
}

volatile void thread_two()
{
	//asm volatile("xchg %bx, %bx");
	for (t2=0;;)
	{
		asm volatile ("movl %1, %%eax;"
				//"nop;"
		     "incl (%%eax);"
			:"=r"(t2)
			:"r"(&t2)
			:"%eax");

		/*asm volatile("movl %1, %%eax;"
				"incl (%%eax);" :: "r"(t2) : "%eax");*/
	}
}

void thread_monitor()
{
	unsigned short y;

	kprintf("counting threads:\n");
	y=get_csr_y();

	while(1) {
		move_cursor(0,y);
		kprintf("%u\t%u", t1, t2);
		asm volatile("int $0x20");
		//asm volatile("xchg %bx, %bx");
	}
}

void one_loop()
{
	//static unsigned int i=0;
	char progress[] = "\\1/1";
	//static unsigned short cursor=0;
	unsigned int p=0;

	while(1) {
		//asm volatile("xchg %bx, %bx");
		//move_cursor(tty.csr_x,tty.csr_y);
		putch_help(&tty1,'\b');
		//write(progress[p++%4], 1);
		putch_help(&tty1,progress[p++%4]);
	}
}

void two_loop()
{
	//static unsigned int i=0;
	char progress[] = "\\2/2";
	//static unsigned short cursor=0;
	unsigned int p=0;

	while(1) {
		//asm volatile("xchg %bx, %bx");
		//move_cursor(tty.csr_x,tty.csr_y);
		//write(progress[p++%4], 1);
		putch_help(&tty2,'\b');
		putch_help(&tty2,progress[p++%4]);
	}
}

void one_main(void)
{
	//asm volatile("xchg %bx, %bx");
	//kprintf("loop one : .");
	//move_cursor(0,0);
	/*tty1.csr_x=get_csr_x();
	tty1.csr_y=0;
	tty1.fb_adr=_vc[0].fb_adr;
	tty1.attrib=_vc[0].attrib;

	putch_help(&tty1, '1');
	putch_help(&tty1, ':');
	putch_help(&tty1, ' ');*/

	one_loop();
}

void two_main(void)
{
	//asm volatile("xchg %bx, %bx");
	//kprintf("loop two : .");
	//move_cursor(0,1);
	/*tty2.csr_x=get_csr_x();
	tty2.csr_y=1;
	tty2.fb_adr=_vc[0].fb_adr;
	tty2.attrib=_vc[0].attrib;

	putch_help(&tty2, '\n');
	putch_help(&tty2, '2');
	putch_help(&tty2, ':');
	putch_help(&tty2, ' ');*/

	two_loop();
}
