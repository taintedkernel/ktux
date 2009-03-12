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

// keyboard.c - keyboard driver

#include <ktux.h>
#include <keyboard.h>
#include <video.h>

// dependencies
#include <io.h>							// inportb()
#include <stdio.h>						// putch()
#include <pic.h>
#include <interrupts.h>                 // reboot()
#include <sh.h>							// processCommand()
#include <string.h>						// memsetd()


char scancode_to_ascii[0x100] = {
    SPECIAL, SPECIAL,
    '1','2','3','4','5','6','7','8','9','0','-','=',
    '\b', '\t',
    'q','w','e','r','t','y','u','i','o','p','[',']',ENTER,SPECIAL,
    'a','s','d','f','g','h','j','k','l',';','\'','`','\\',
    '\\','z','x','c','v','b','n','m',',','.','/',SPECIAL,SPECIAL,SPECIAL,
    ' ',SPECIAL,RAW1_F1,RAW1_F2,RAW1_F3,RAW1_F4,RAW1_F5,RAW1_F6,RAW1_F7,RAW1_F8,RAW1_F9,RAW1_F10,SPECIAL,SPECIAL,PAUSE,SPECIAL,SPECIAL,
};

char scancode_to_ascii_caps[0x100] = {
    SPECIAL, SPECIAL,
    '1','2','3','4','5','6','7','8','9','0','-','=',
    '\b', '\t',
    'Q','W','E','R','T','Y','U','I','O','P','[',']',ENTER,SPECIAL,
    'A','S','D','F','G','H','J','K','L',';','\'','`','\\',
    '\\','Z','X','C','V','B','N','M',',','.','/',SPECIAL,SPECIAL,SPECIAL,
    ' ',SPECIAL,RAW1_F1,RAW1_F2,RAW1_F3,RAW1_F4,RAW1_F5,RAW1_F6,RAW1_F7,RAW1_F8,RAW1_F9,RAW1_F10,PAUSE,SPECIAL,SPECIAL,
};

char scancode_to_ascii_shift[0x100] = {
    SPECIAL, SPECIAL,
    '!','@','#','$','%','^','&','*','(',')','_','+',
    '\b', '\t',
    'Q','W','E','R','T','Y','U','I','O','P','{','}',ENTER,SPECIAL,
    'A','S','D','F','G','H','J','K','L',':','"','~','|',
    '|','Z','X','C','V','B','N','M','<','>','?',SPECIAL,SPECIAL,SPECIAL,
    ' ',SPECIAL,RAW1_F1,RAW1_F2,RAW1_F3,RAW1_F4,RAW1_F5,RAW1_F6,RAW1_F7,RAW1_F8,RAW1_F9,RAW1_F10,PAUSE,SPECIAL,SPECIAL,
};

char echo=1;
unsigned int kbdBufPos;
unsigned int cmdBufPos;

char keyboardBuffer[KBD_BUF_SIZE];
char commandBuffer[CMD_BUF_SIZE];


int init_keyboard(void)
{
	kbdBufPos = 0;
	cmdBufPos = 0;
	return 0;
}

void kbd_irq_handler(void)
{
	static unsigned char keycode;

	// get scancode from port 0x60
	keycode = inportb(0x60);
	keyboardBuffer[kbdBufPos++ % KBD_BUF_SIZE] = keycode;

	// *TODO* defer this to a bottom half
	process_kbd(keycode);
}

void process_kbd(unsigned char keycode)
{
	static unsigned int i;
	static unsigned short kbd_status, saw_break_code;
	static unsigned short temp;

	// check for break keycode (i.e. a keycode is released)
	if(keycode >= 0x80)
	{
		saw_break_code = 1;
		keycode &= 0x7F;
	}
	// the only break codes we're interested in are Shift, Ctrl, Alt
	if(saw_break_code)
	{
		if(keycode == RAW1_LEFT_ALT || keycode == RAW1_RIGHT_ALT)
			kbd_status &= ~KBD_META_ALT;
		else if(keycode == RAW1_LEFT_CTRL || keycode == RAW1_RIGHT_CTRL)
			kbd_status &= ~KBD_META_CTRL;
		else if(keycode == RAW1_LEFT_SHIFT || keycode == RAW1_RIGHT_SHIFT)
			kbd_status &= ~KBD_META_SHIFT;
		saw_break_code = 0;
		return;
	}
	// it's a make keycode: check the "meta" keycodes, as above
	if(keycode == RAW1_LEFT_ALT || keycode == RAW1_RIGHT_ALT)
	{
		kbd_status |= KBD_META_ALT;
		return;
	}
	if(keycode == RAW1_LEFT_CTRL || keycode == RAW1_RIGHT_CTRL)
	{
		kbd_status |= KBD_META_CTRL;
		return;
	}
	if(keycode == RAW1_LEFT_SHIFT || keycode == RAW1_RIGHT_SHIFT)
	{
		kbd_status |= KBD_META_SHIFT;
		return;
	}
	// Scroll Lock, Num Lock, and Caps Lock set the LEDs. These keycodes
	// have on-off (toggle or XOR) action, instead of momentary action
	if(keycode == RAW1_SCROLL_LOCK)
	{
		kbd_status ^= KBD_META_SCRL;
		goto LEDS;
	}
	if(keycode == RAW1_NUM_LOCK)
	{
		kbd_status ^= KBD_META_NUM;
		goto LEDS;
	}
	if(keycode == RAW1_CAPS_LOCK)
	{
		kbd_status ^= KBD_META_CAPS;
LEDS:		write_kbd(0x60, 0xED);	// "set LEDs" command
		temp = 0;
		if(kbd_status & KBD_META_SCRL)
			temp |= 1;
		if(kbd_status & KBD_META_NUM)
			temp |= 2;
		if(kbd_status & KBD_META_CAPS)
			temp |= 4;
		write_kbd(0x60, temp);	// bottom 3 bits set LEDs
		return;
	}

	// if it's F1, F2 etc. switch to the appropriate virtual console
	if (keycode >= RAW1_F1 && keycode <= RAW1_F10) // && (kbd_status & KBD_META_ALT) && (kbd_status & KBD_META_CTRL))
	{
		switch(keycode)
		{
		case RAW1_F1:
			i = 0;
			goto SWITCH_VC;
		case RAW1_F2:
			i = 1;
			goto SWITCH_VC;
		case RAW1_F3:
			i = 2;
			goto SWITCH_VC;
		case RAW1_F4:
			i = 3;
			goto SWITCH_VC;
		case RAW1_F5:
			i = 4;
			goto SWITCH_VC;
		case RAW1_F6:
			i = 5;
			goto SWITCH_VC;
		case RAW1_F7:
			i = 6;
			goto SWITCH_VC;
		case RAW1_F8:
			i = 7;
			goto SWITCH_VC;
		case RAW1_F9:
			i = 8;
			goto SWITCH_VC;
		case RAW1_F10:
			i = 9;
			goto SWITCH_VC;
		case RAW1_F11:
			i = 10;
			goto SWITCH_VC;
		case RAW1_F12:
			i = 11;
	SWITCH_VC:
			select_vc(i);
			break;
		}
	}
	else
	{
		i = convert(keycode, kbd_status);
		if(i != 0)
		{
			if (cmd_buf_add(i) == OK && echo)
			{
				putch(i);
				if (i == 's'){
					cli();
					ps();
					sti();
				}
			}
		}
	}

	// NO PRINTF IN INTERRUPT HANDLERS - NON RE-ENTRANT! //
	//kprintf("IRQ1 handled, keycode=%04X, char='%c'\n", keycode, convert(keycode));
}

void write_kbd(unsigned adr, unsigned char data)
{
	unsigned long timeout;
	unsigned char stat;

/* Linux code didn't have a timeout here... */
	for(timeout = 500000L; timeout != 0; timeout--)
	{
		stat = inportb(0x64);
/* loop until 8042 input buffer empty */
		if((stat & 0x02) == 0)
			break;
	}
	if(timeout != 0)
		outportb(adr, data);
}

unsigned convert(unsigned key, unsigned short kbd_status)
{
	static unsigned short temp;

/* ignore invalid scan codes */
	if(key >= sizeof(scancode_to_ascii) / sizeof(scancode_to_ascii[0]))
		return 0;

	if(key >= RAW1_F1 && key <= RAW1_SCROLL_LOCK)
		return 0;

/* convert raw scancode in key to unshifted ASCII in temp */
	if (kbd_status & KBD_META_SHIFT)
		temp = scancode_to_ascii_shift[key];
	else if (kbd_status & KBD_META_CAPS)
		temp = scancode_to_ascii_caps[key];
	else
		temp = scancode_to_ascii[key];

/* defective keyboard? non-US keyboard? more than 104 keys? */
	if(temp == 0)
		return temp;

/* handle the three-finger salute */
	if((kbd_status & KBD_META_CTRL) && (kbd_status & KBD_META_ALT) &&
		(temp == KEY_DEL))
	{
		kprintf("\n""\x1B[42;37;1m""*** rebooting!");
		reboot();
	}

/* I really don't know what to do yet with Alt, Ctrl, etc. -- punt */
	return temp;
}

int cmd_buf_add(char key)
{
	unsigned int end;

	if (key == '\b')
	{
		if (cmdBufPos > 0) {
			commandBuffer[--cmdBufPos] = '\0';
			return OK;
		}
		else
			return ERROR;
	}
	else if (key == '\t')
	{
		for (end=(cmdBufPos+8) & 7; cmdBufPos < end;
			commandBuffer[cmdBufPos++]=' ' )
			;
		return OK;
	}
	else if (key == '\n')
	{
		//commandBuffer[cmdBufPos] = '\n';
		processCommand(commandBuffer);
		memsetd(commandBuffer, 0, CMD_BUF_SIZE >> 2);
		cmdBufPos = 0;
		return OK;
	}
	else {
		commandBuffer[cmdBufPos++ % CMD_BUF_SIZE] = key;
		return OK;
	}

	// never reached
	return ERROR;
}
