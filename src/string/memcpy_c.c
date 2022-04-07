// memcpy_c is only meaningful if targeting capability architectures.
#if defined(__CHERI__)

#if !defined(__SANITIZE_CHERISEED__)
#error "FIXME: please update to support tags."
#endif

#include <string.h>

void *__capability memcpy_c(void *__capability restrict dest,
	const void *__capability restrict src, size_t n)
{
	unsigned char *__capability d = dest;
	const unsigned char *__capability s = src;
	for (size_t i = 0; i < n; ++i)
		d[i] = s[i];
	return dest;
}

// Cannot alias the base version in hybrid : the types do not match.
#if defined(__CHERI_PURE_CAPABILITY__)
weak_alias(memcpy_c, memcpy);
#endif

#endif // !defined(__CHERI__)
