#if defined(__CHERI_PURE_CAPABILITY__)

#include <string.h>
#include <stdint.h>

#define __CAP_SIZE (sizeof(intcap_t))

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;
	for (; (unsigned long)s % __CAP_SIZE && n; n--) *d++ = *s++;
	if ((unsigned long)d % __CAP_SIZE == 0) {
		// source and destination are aligned
		for (; n >= __CAP_SIZE;  s += __CAP_SIZE, d += __CAP_SIZE, n -= __CAP_SIZE) {
			*(void**)d = *(void**)s;
		}
	}
	for (; n; n--) *d++ = *s++;
	return dest;
}

#else
#include "../memcpy.c"
#endif
