#define _GNU_SOURCE
#include <string.h>

#include "morello_helpers.h"

char *strsep(char **str, const char *sep)
{
	char *s = *str, *end;
	if (!s) return NULL;
	end = s + strcspn(s, sep);
	if (*end) *end++ = 0;
	else end = 0;
	*str = end;
	return RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ(s);
}
