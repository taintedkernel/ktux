#ifndef __KTUX_STRING_H
#define	__KTUX_STRING_H

// String defines and typedefs
typedef unsigned int size_t;

// Function declarations
void *memcpy(void *dst_ptr, const void *src_ptr, size_t count);
void *memsetw(void *dst, int val, size_t count);
void *memsetd(void *dst, int val, size_t count);
unsigned strlen(const char *str);
int strcmp(const char *s1, const char *s2);

#define offsetof(str, mbr) (&(str.mbr) - &str)

#endif

