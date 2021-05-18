#include <wchar.h>

#include "morello_helpers.h"

wchar_t *wcpcpy(wchar_t *restrict d, const wchar_t *restrict s)
{
	return RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ(wcscpy(d, s) + wcslen(s));
}
