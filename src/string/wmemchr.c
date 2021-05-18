#include <wchar.h>

#include "morello_helpers.h"

wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n)
{
	for (; n && *s != c; n--, s++);
	return n ? RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ((wchar_t *)s) : 0;
}
