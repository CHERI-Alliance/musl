#define _BSD_SOURCE
#include <string.h>

void explicit_bzero(void *d, size_t n)
{
	d = memset(d, 0, n);
#if 0
	__asm__ __volatile__ ("" : : "r"(d) : "memory");
#else
	__asm__ __volatile__ ("" : : "C"(d) : "memory");
#endif
}
