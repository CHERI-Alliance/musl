// memset_c is only meaningful if targeting capability architectures.
#if defined(__CHERI__)

#include <string.h>

void *__capability memset_c(void *__capability dest, int c, size_t n)
{
	unsigned char *__capability d = dest;
	for (size_t i = 0; i < n; ++i)
		d[i] = (unsigned char)c;
	return dest;
}

// Cannot alias the base version in hybrid : the types do not match.
#if defined(__CHERI_PURE_CAPABILITY__)
weak_alias(memset_c, memset);
#endif

#endif /* __CHERI__ */
