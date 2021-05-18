#include <wchar.h>

#include "morello_helpers.h"

wchar_t *wcsrchr(const wchar_t *s, wchar_t c)
{
	const wchar_t *p;
	for (p=s+wcslen(s); p>=s && *p!=c; p--);
	return p>=s ? RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ((wchar_t *)p) : 0;
}
