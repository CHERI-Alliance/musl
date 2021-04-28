#include <string.h>

#include "morello_helpers.h"

char *strpbrk(const char *s, const char *b)
{
	s += strcspn(s, b);
	return *s ? RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ((char *)s) : 0;
}
