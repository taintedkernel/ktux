// io.c

#include <io.h>             // this source

__inline__ void outportb(unsigned short port, char value)
{
    __asm__ __volatile__ ("outb %%al,%%dx"::"d" (port), "a" (value));
}

__inline__ void outportw(unsigned short port, short value)
{
    __asm__ __volatile__ ("outw %%ax,%%dx"::"d" (port), "a" (value));
}

__inline__ unsigned char inportb(unsigned short port)
{
    unsigned char value;
    __asm__ __volatile__ ("inb %w1,%b0" : "=a"(value) : "d"(port));
    return value;
}
