#ifndef __KTUX_STDIO_H
#define __KTUX_STDIO_H

#include <_va_list.h>

// Function pointers
typedef int (*fnptr_t)(unsigned c, void **helper);

// Function declarations
int do_printf(const char *fmt, va_list args, fnptr_t fn, void *ptr);
int kprintf_help(unsigned c, void **ptr);
void kprintf(const char *fmtStr, ...);
int vsprintf_help(unsigned c, void **ptr);
int vsprintf(char *buffer, const char *fmt, va_list args);
int sprintf(char *buffer, const char *fmt, ...);
void panic(int haltOn, const char *fmt, ...);

#endif
