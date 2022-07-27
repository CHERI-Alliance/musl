#include <string.h>
#include <stdint.h>

#if defined(__CHERI_PURE_CAPABILITY__) && defined(__SANITIZE_CHERISEED__)

// FIXME: please update to support tags
void *memmove(void *dest, const void *src, size_t n)
{
	if (dest == src) return dest;

	char *d = dest;
	const char *s = src;
	if (d < s) {
		// s:    |...|
		// d:  |...|
		for (size_t i = 0; i < n; ++i)
			d[i] = s[i];
	} else {
		// s:  |...|
		// d:   |...|
		// Avoiding underflow of size_t.
		while (n > 0) {
			--n;
			d[n] = s[n];
		}
	}
	return dest;
}

#else  // __CHERI_PURE_CAPABILITY__ && __SANITIZE_CHERISEED__

#ifdef __GNUC__
typedef __attribute__((__may_alias__)) void* WT;
#define WS (sizeof(WT))
#endif

void *memmove(void *dest, const void *src, size_t n)
{
	char *d = dest;
	const char *s = src;

	if (d==s) return d;
	if ((uintptr_t)s-(uintptr_t)d-n <= -2*n) return memcpy(d, s, n);

	if (d<s) {
#ifdef __GNUC__
		if ((uintptr_t)s % WS == (uintptr_t)d % WS) {
			while ((uintptr_t)d % WS) {
				if (!n--) return dest;
				*d++ = *s++;
			}
			for (; n>=WS; n-=WS, d+=WS, s+=WS) *(WT *)d = *(WT *)s;
		}
#endif
		for (; n; n--) *d++ = *s++;
	} else {
#ifdef __GNUC__
		if ((uintptr_t)s % WS == (uintptr_t)d % WS) {
			while ((uintptr_t)(d+n) % WS) {
				if (!n--) return dest;
				d[n] = s[n];
			}
			while (n>=WS) n-=WS, *(WT *)(d+n) = *(WT *)(s+n);
		}
#endif
		while (n) n--, d[n] = s[n];
	}

	return dest;
}

#endif  // __CHERI_PURE_CAPABILITY__ && __SANITIZE_CHERISEED__
