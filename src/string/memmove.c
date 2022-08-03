#include <string.h>
#include <stdint.h>

#if defined(__CHERI_PURE_CAPABILITY__) && defined(__SANITIZE_CHERISEED__)

void *memmove(void *dest, const void *src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

	if (d == s)
		return dest;

	// s:    |...|
	// d:  |...|
	if (d < s)
		return memcpy(dest, src, n);

	// s:  |...|
	// d:   |...|
	d += n;
	s += n;

	// Preserve tags if size is at least the size of a capability and the
	// alignment of dest and src, with respect to capability size, are the same.
	if ((n >= sizeof(void*))
	    && (((ptraddr_t)d % sizeof(void*)) == ((ptraddr_t)s % sizeof(void*)))) {
		// There can be some tail to move without preserving tags.
		while (((ptraddr_t)s % sizeof(void*)) != 0) {
			*--d = *--s;
			--n;
		}

		// Calculate the number of capabilities to move.
		size_t cap_count = n / sizeof(void*);
		n -= cap_count * sizeof(void*);

		// Copy capabilities, if any.
		uintcap_t *d_cap = (uintcap_t*)d;
		const uintcap_t *s_cap = (uintcap_t*)s;
		while (cap_count-- > 0)
			*--d_cap = *--s_cap;

		d = (unsigned char *)d_cap;
		s = (const unsigned char *)s_cap;

		// Move the head, if any, without preserving tags.
	}

	// Move without preserving tags.
	while (n-- > 0)
		*--d = *--s;

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
