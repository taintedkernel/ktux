#ifndef __KTUX_VGA_H
#define __KTUX_VGA_H

// x86 Video Constants
#define TEXT_SCREENSTART	0x000B8000
#define TEXT_NUMROWS		25
#define TEXT_NUMCOLUMNS		80
#define TEXT_SCREENSIZE		TEXT_NUMROWS * TEXT_NUMCOLUMNS

#define BIOS_CURSOR_PORT1	0x3d4
#define BIOS_CURSOR_PORT2	0x3d5

#define	VGA_MISC_READ		0x3CC

#endif
