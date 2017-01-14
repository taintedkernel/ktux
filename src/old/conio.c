// conio.c

/*
#include <conio.h>
#include <io.h>



void print(const char *_message)
{
	byte *vidmem = ((byte *)0xB8000);
	word offset;
	dword i;

	outb(0x3D4, 14);
	offset = inb(0x3D5) << 8;
	outb(0x3D4, 15);
	offset |= inb(0x3D5);

	vidmem += offset*2;

	i = 0;
	while (_message[i] != 0) {
		*vidmem = _message[i++];
		vidmem += 2;
	}

	offset += i;
	outb(0x3D5, (byte)(offset));
	outb(0x3D4, 14);
	outb(0x3D5, (byte)(offset >> 8));
}
*/
