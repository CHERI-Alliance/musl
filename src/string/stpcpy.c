#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>

#include "morello_helpers.h"

#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

char *__stpcpy(char *restrict d, const char *restrict s)
{
#ifdef __GNUC__
	size_t i = 0;
	size_t max_i = CAP_TAIL_LENGTH(s);

	typedef size_t __attribute__((__may_alias__)) word;
	word *wd;
	const word *ws;
	if ((uintptr_t)s % ALIGN == (uintptr_t)d % ALIGN) {
		for (; (uintptr_t)s % ALIGN; s++, d++, i++)
			if (!(*d=*s)) return RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ(d);
		if (LT_IF_MORELLO_ELSE(i + sizeof(word) - 1, max_i, true)) {
			wd=(void *)d; ws=(const void *)s;
			for (; LT_IF_MORELLO_ELSE(i + sizeof(word) - 1, max_i, true) &&
			       !HASZERO(*ws);
			     *wd++ = *ws++)
			  ;
			d=(void *)wd; s=(const void *)ws;
		}
	}
#endif
	for (; (*d=*s); s++, d++);

	return RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ(d);
}

weak_alias(__stpcpy, stpcpy);
