#include <wchar.h>

#include "morello_helpers.h"

wchar_t *wcschr(const wchar_t *s, wchar_t c)
{
	if (!c) return (wchar_t *)s + wcslen(s);
	for (; *s && *s != c; s++);
	return *s ? RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ((wchar_t *)s) : 0;
}
