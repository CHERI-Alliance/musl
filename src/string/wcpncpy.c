#include <wchar.h>

#include "morello_helpers.h"

wchar_t *wcpncpy(wchar_t *restrict d, const wchar_t *restrict s, size_t n)
{
	return RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ(wcsncpy(d, s, n) + wcsnlen(s, n));
}
