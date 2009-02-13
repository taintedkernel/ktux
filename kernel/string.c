// string.c

#include <string.h>			// this source

void *memcpy(void *dst_ptr, const void *src_ptr, size_t count)
{
	void *ret_val = dst_ptr;
	const char *src = (const char *)src_ptr;
	char *dst = (char *)dst_ptr;

/* copy up */
	for(; count != 0; count--)
		*dst++ = *src++;
	return ret_val;
}

void *memsetw(void *dst, int val, size_t count)
{
	unsigned short *temp = (unsigned short *)dst;

	for( ; count != 0; count--)
		*temp++ = val;
	return dst;
}

void *memsetd(void *dst, int val, size_t count)
{
	unsigned int *temp = (unsigned int *)dst;

	for( ; count != 0; count--)
		*temp++ = val;
	return dst;
}

size_t strlen(const char *str)
{
	size_t ret_val;

	for(ret_val = 0; *str != '\0'; str++)
		ret_val++;
	return ret_val;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 != '\0' && *s1 == *s2) {
		s1++;
		s2++;
	}

	return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}

int strncmp(const char *s1, const char *s2, unsigned long len)
{
	unsigned long i=0;

	while (*s1 != '\0' && *s1 == *s2 && i < len) {
		s1++; s2++; i++;
	}

	return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}
