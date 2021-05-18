#include <wchar.h>

#include "morello_helpers.h"

wchar_t *wcspbrk(const wchar_t *s, const wchar_t *b)
{
	s += wcscspn(s, b);
	return *s ? RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ((wchar_t *)s) : NULL;
}
