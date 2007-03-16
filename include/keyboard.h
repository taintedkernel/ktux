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

// keyboard.h - keyboard handler

#ifndef __KTUX_KEYBOARD_H
#define __KTUX_KEYBOARD_H

//#include <ktux.h>					// unknown
//#include <interrupts.h>				// unknown

/* "ASCII" values for non-ASCII keys. All of these are user-defined.
Hrrm. Unicode defines code pages for pseudographics (e.g. box-drawing
characters). I wonder it defines anything for keys like these?
function keys: */
#define SPECIAL_KEY_START	0x80
#define	KEY_F1				0x80
#define	KEY_F2				(KEY_F1 + 1)
#define	KEY_F3				(KEY_F2 + 1)
#define	KEY_F4				(KEY_F3 + 1)
#define	KEY_F5				(KEY_F4 + 1)
#define	KEY_F6				(KEY_F5 + 1)
#define	KEY_F7				(KEY_F6 + 1)
#define	KEY_F8				(KEY_F7 + 1)
#define	KEY_F9				(KEY_F8 + 1)
#define	KEY_F10				(KEY_F9 + 1)
#define	KEY_F11				(KEY_F10 + 1)
#define	KEY_F12				(KEY_F11 + 1)

// cursor keys
#define	KEY_INS				0x90
#define	KEY_DEL				(KEY_INS + 1)
#define	KEY_HOME			(KEY_DEL + 1)
#define	KEY_END				(KEY_HOME + 1)
#define	KEY_PGUP			(KEY_END + 1)
#define	KEY_PGDN			(KEY_PGUP + 1)
#define	KEY_LFT				(KEY_PGDN + 1)
#define	KEY_UP				(KEY_LFT + 1)
#define	KEY_DN				(KEY_UP + 1)
#define	KEY_RT				(KEY_DN + 1)

// print screen/sys rq and pause/break
#define	KEY_PRNT			(KEY_RT + 1)
#define	KEY_PAUSE			(KEY_PRNT + 1)

// these return a value but they could also act as additional bucky keys
#define	KEY_LWIN			(KEY_PAUSE + 1)
#define	KEY_RWIN			(KEY_LWIN + 1)
#define	KEY_MENU			(KEY_RWIN + 1)
#define SPECIAL_KEY_END		(KEY_MENU + 1)

// "bucky bits" - 0x0100 is reserved for non-ASCII keys, so start with 0x200
#define	KBD_META_ALT		0x0200			// Alt is pressed
#define	KBD_META_CTRL		0x0400			// Ctrl is pressed
#define	KBD_META_SHIFT		0x0800			// Shift is pressed
#define	KBD_META_ANY		(KBD_META_ALT | KBD_META_CTRL | KBD_META_SHIFT)
#define	KBD_META_CAPS		0x1000			// CapsLock is on
#define	KBD_META_NUM		0x2000			// NumLock is on
#define	KBD_META_SCRL		0x4000			// ScrollLock is on

#define	RAW1_LEFT_CTRL		0x1D
#define	RAW1_RIGHT_CTRL		0x1D			// same as left
#define	RAW1_LEFT_SHIFT		0x2A
#define	RAW1_RIGHT_SHIFT	0x36
#define	RAW1_LEFT_ALT		0x38
#define	RAW1_RIGHT_ALT		0x38			// same as left
#define	RAW1_CAPS_LOCK		0x3A
#define	RAW1_F1				0x3B
#define	RAW1_F2				0x3C
#define	RAW1_F3				0x3D
#define	RAW1_F4				0x3E
#define	RAW1_F5				0x3F
#define	RAW1_F6				0x40
#define	RAW1_F7				0x41
#define	RAW1_F8				0x42
#define	RAW1_F9				0x43
#define	RAW1_F10			0x44
#define	RAW1_NUM_LOCK		0x45
#define	RAW1_SCROLL_LOCK	0x46
#define	RAW1_F11			0x57
#define	RAW1_F12			0x58

#define SPECIAL				0
#define ENTER				10
#define PAUSE				'P'

#define	KBD_BUF_SIZE		64


int init_keyboard(void);
void write_kbd(unsigned adr, unsigned char data);
unsigned convert(unsigned key, unsigned short kbd_status);
void kbd_irq_handler(void);
void process_kbd(unsigned char keycode);

#endif
