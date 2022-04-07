// memmove_c is only meaningful if targeting capability architectures.
#if defined(__CHERI__)

#if !defined(__SANITIZE_CHERISEED__)
#error "FIXME: please update to support tags."
#endif

#include <string.h>

void *__capability memmove_c(void *__capability dest,
	const void *__capability src, size_t n)
{
	if (dest == src) return dest;

	char *__capability d = dest;
	const char *__capability s = src;
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

// Cannot alias the base version in hybrid : the types do not match.
#if defined(__CHERI_PURE_CAPABILITY__)
weak_alias(memmove_c, memmove);
#endif

#endif /* __CHERI__ */
