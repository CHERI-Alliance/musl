#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>

#include "morello_helpers.h"

#define ALIGN (sizeof(size_t)-1)
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

char *__stpncpy(char *restrict d, const char *restrict s, size_t n)
{
#ifdef __GNUC__
	size_t i = 0;
	size_t max_i = CAP_TAIL_LENGTH(s);

	typedef size_t __attribute__((__may_alias__)) word;
	word *wd;
	const word *ws;
	if (((uintptr_t)s & ALIGN) == ((uintptr_t)d & ALIGN)) {
		for (; ((uintptr_t)s & ALIGN) && n && (*d=*s); n--, s++, d++, i++);
		if (!n || !*s) goto tail;
		if (LT_IF_MORELLO_ELSE(i + sizeof(word) - 1, max_i, true)) {
			wd=(void *)d; ws=(const void *)s;
                        for (; LT_IF_MORELLO_ELSE(i + sizeof(word) - 1, max_i, true) &&
                               n >= sizeof(size_t) && !HASZERO(*ws);
                             n -= sizeof(size_t), ws++, wd++)
                          *wd = *ws;
                        d=(void *)wd; s=(const void *)ws;
		}
	}
#endif
	for (; n && (*d=*s); n--, s++, d++);
tail:
	memset(d, 0, n);
	return RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ(d);
}

weak_alias(__stpncpy, stpncpy);

