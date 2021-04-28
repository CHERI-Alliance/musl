#include <string.h>

#include "morello_helpers.h"

char *strtok(char *restrict s, const char *restrict sep)
{
	static char *p;
	if (!s && !(s = p)) return NULL;
	s += strspn(s, sep);
	if (!*s) return p = 0;
	p = s + strcspn(s, sep);
	if (*p) *p++ = 0;
	else p = 0;
	return RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ(s);
}
