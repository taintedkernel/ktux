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

// video.c

#include <ktux.h>
#include <string.h>			// memcpy(), memsetw()
#include <stdio.h>			// kprintf()
#include <io.h>				// outb(), inb()
#include <vga.h>			// VGA_MISC_READ
#include <video.h>			// this source
#include <ctype.h>			// isdigit


console _vc[MAX_VC];
static unsigned short _num_vcs;
static console *_curr_vc;

static unsigned short *_vga_fb_adr = (void *)0;
static unsigned _crtc_io_adr, _vc_width, _vc_height;

void blink(void)
{
	(*(unsigned char *)_vga_fb_adr)++;
}

/*
void resetCursor(void)
{
	_csr_x = 0;
	_csr_y = 0;
	moveCSR();
}
*/
void move_cursor(unsigned int x, unsigned int y)
{
	_curr_vc->csr_x = x;
	_curr_vc->csr_y = y;
	move_csr();
}

void scroll(console *con)
{
	unsigned short *fb_adr;
	unsigned blank, temp;

	blank = 0x20 | ((unsigned)con->attrib << 8);
	fb_adr = con->fb_adr;
/* scroll up */
	if(con->csr_y >= _vc_height)
	{
		temp = con->csr_y - _vc_height + 1;
		memcpy(fb_adr, fb_adr + temp * _vc_width,
			(_vc_height - temp) * _vc_width * 2);
/* blank the bottom line of the screen */
		memsetw(fb_adr + (_vc_height - temp) * _vc_width,
			blank, _vc_width);
		con->csr_y = _vc_height - 1;
	}
}

/*****************************************************************************
*****************************************************************************/
void set_attrib(console *con, unsigned att)
{
	static const unsigned ansi_to_vga[] =
	{
		0, 4, 2, 6, 1, 5, 3, 7
	};
/**/
	unsigned new_att;

	new_att = con->attrib;
	if(att == 0)
		new_att &= ~0x08;		/* bold off */
	else if(att == 1)
		new_att |= 0x08;		/* bold on */
	else if(att >= 30 && att <= 37)
	{
		att = ansi_to_vga[att - 30];
		new_att = (new_att & ~0x07) | att;/* fg color */
	}
	else if(att >= 40 && att <= 47)
	{
		att = ansi_to_vga[att - 40] << 4;
		new_att = (new_att & ~0x70) | att;/* bg color */
	}
	con->attrib = new_att;
}
/*****************************************************************************
*****************************************************************************/
void move_csr(void)
{
	unsigned short temp;

	temp = (_curr_vc->csr_y * _vc_width + _curr_vc->csr_x) +
		(_curr_vc->fb_adr - _vga_fb_adr);
	outportb(_crtc_io_adr + 0, 14);
	outportb(_crtc_io_adr + 1, temp >> 8);
	outportb(_crtc_io_adr + 0, 15);
	outportb(_crtc_io_adr + 1, temp);
}
/*****************************************************************************
*****************************************************************************/
void select_vc(unsigned which_vc)
{
	unsigned i;

	if(which_vc >= _num_vcs)
		return;
	_curr_vc = _vc + which_vc;
	i = _curr_vc->fb_adr - _vga_fb_adr;
	outportb(_crtc_io_adr + 0, 12);
	outportb(_crtc_io_adr + 1, i >> 8);
	outportb(_crtc_io_adr + 0, 13);
	outportb(_crtc_io_adr + 1, i);
/* oops, forgot this: */
	move_csr();
}
/*****************************************************************************
*****************************************************************************/
void putch_help(console *con, unsigned c)
{
	unsigned short *fb_adr;
	unsigned att;

	att = (unsigned)con->attrib << 8;
	fb_adr = con->fb_adr;
/* state machine to handle the escape sequences
ESC */
	if(con->esc == 1)
	{
		if(c == '[')
		{
			con->esc++;
			con->esc1 = 0;
			return;
		}
		/* else fall-through: zero esc and print c */
	}
/* ESC[ */
	else if(con->esc == 2)
	{
		if(isdigit(c))
		{
			con->esc1 = con->esc1 * 10 + c - '0';
			return;
		}
		else if(c == ';')
		{
			con->esc++;
			con->esc2 = 0;
			return;
		}
/* ESC[2J -- clear screen */
		else if(c == 'J')
		{
			if(con->esc1 == 2)
			{
				memsetw(fb_adr, ' ' | att,
					_vc_height * _vc_width);
				con->csr_x = con->csr_y = 0;
			}
		}
/* ESC[num1m -- set attribute num1 */
		else if(c == 'm')
			set_attrib(con, con->esc1);
		con->esc = 0;	/* anything else with one numeric arg */
		return;
	}
/* ESC[num1; */
	else if(con->esc == 3)
	{
		if(isdigit(c))
		{
			con->esc2 = con->esc2 * 10 + c - '0';
			return;
		}
		else if(c == ';')
		{
			con->esc++;	/* ESC[num1;num2; */
			con->esc3 = 0;
			return;
		}
/* ESC[num1;num2H -- move cursor to num1,num2 */
		else if(c == 'H')
		{
			if(con->esc2 < _vc_width)
				con->csr_x = con->esc2;
			if(con->esc1 < _vc_height)
				con->csr_y = con->esc1;
		}
/* ESC[num1;num2m -- set attributes num1,num2 */
		else if(c == 'm')
		{
			set_attrib(con, con->esc1);
			set_attrib(con, con->esc2);
		}
		con->esc = 0;
		return;
	}
/* ESC[num1;num2;num3 */
	else if(con->esc == 4)
	{
		if(isdigit(c))
		{
			con->esc3 = con->esc3 * 10 + c - '0';
			return;
		}
/* ESC[num1;num2;num3m -- set attributes num1,num2,num3 */
		else if(c == 'm')
		{
			set_attrib(con, con->esc1);
			set_attrib(con, con->esc2);
			set_attrib(con, con->esc3);
		}
		con->esc = 0;
		return;
	}
	con->esc = 0;

/* escape character */
	if(c == 0x1B)
	{
		con->esc = 1;
		return;
	}
/* backspace */
	if(c == 0x08)
	{
		if(con->csr_x != 0)
			con->csr_x--;
	}
/* tab */
	else if(c == 0x09)
		con->csr_x = (con->csr_x + 8) & ~(8 - 1);
/* carriage return */
	else if(c == '\r')	/* 0x0D */
		con->csr_x = 0;
/* line feed */
//	else if(c == '\n')	/* 0x0A */
//		con->csr_y++;
/* CR/LF */
	else if(c == '\n')	/* ### - 0x0A again */
	{
		con->csr_x = 0;
		con->csr_y++;
	}
/* printable ASCII */
	else if(c >= ' ')
	{
		unsigned short *where;

		where = fb_adr + (con->csr_y * _vc_width + con->csr_x);
		*where = (c | att);
		con->csr_x++;
	}
	if(con->csr_x >= _vc_width)
	{
		con->csr_x = 0;
		con->csr_y++;
	}
	scroll(con);
/* move cursor only if the VC we're writing is the current VC */
	if(_curr_vc == con)
		move_csr();
}
/*****************************************************************************
*****************************************************************************/
void putch(unsigned c)
{
/* all kernel messages to VC #0 */
//	putch_help(_vc + 0, c);
/* all kernel messages to current VC */
	putch_help(_curr_vc, c);
}

void clrscr(console *con)
{
	unsigned char *vidmem = ((unsigned char *)_vga_fb_adr);	
	static unsigned short i;

	for(i=0; i<_vc_width*_vc_height; i++) {
		*vidmem++ = 0;
		*vidmem++ = 0xF;
	}

	con->csr_x = 0;
	con->csr_y = 0;
	move_csr();
}

void init_video(void)
{
	unsigned short i;

/* check for monochrome or color VGA emulation */
	if((inportb(VGA_MISC_READ) & 0x01) != 0)
	{
		_vga_fb_adr = (unsigned short *)0xB8000L;
		_crtc_io_adr = 0x3D4;
	}
	else
	{
		_vga_fb_adr = (unsigned short *)0xB0000L;
		_crtc_io_adr = 0x3B4;
	}
/* read current screen size from BIOS data segment (addresses 400-4FF) */
	_vc_width = *(unsigned short *)0x44A;
	_vc_height = *(unsigned char *)0x484 + 1;
/* figure out how many VCs we can have with 32K of display memory.
Use INTEGER division to round down. */
	_num_vcs = 32768L / (_vc_width * _vc_height * 2);
	if(_num_vcs > MAX_VC)
		_num_vcs = MAX_VC;
/* init VCs, with a different foreground color for each */
	for(i = 0; i < _num_vcs; i++)
	{
		_curr_vc = _vc + i;
		_curr_vc->attrib = i + 1;
		_curr_vc->fb_adr = _vga_fb_adr +
			_vc_width * _vc_height * i;
/* ESC[2J clears the screen */
		kprintf("\x1B[2J  this is VC#%u (of 0-%u)\n",
			i, _num_vcs - 1);
	}
	select_vc(0);
	kprintf("init_video: %s emulation, %u x %u, framebuffer at "
		"0x%lX\n", (_crtc_io_adr == 0x3D4) ? "color" : "mono",
		_vc_width, _vc_height, _vga_fb_adr);
}
