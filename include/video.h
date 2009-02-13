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

// video.h

#ifndef __KTUX_VIDEO_H
#define __KTUX_VIDEO_H

#define	MAX_VC	12

#define	VGA_MISC_READ	0x3CC


typedef struct
{
// virtual console input
//	queue_t keystrokes;
// virtual console output
	unsigned esc, attrib, csr_x, csr_y, esc1, esc2, esc3;
	unsigned short *fb_adr;
} console;


// Function declarations
void blink(void);
void scroll(console *con);
void move_csr(void);
unsigned short get_csr_x(void);
unsigned short get_csr_y(void);
void putch_help(console *con, unsigned c);
void putch(unsigned c);
void clrscr(console *con);
void init_console(void);
void set_attrib(console *con, unsigned att);
void select_vc(unsigned which_vc);
void move_cursor(unsigned int x, unsigned int y);
void print_video_info(void);

#endif
