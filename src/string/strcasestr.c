#define _GNU_SOURCE
#include <string.h>

#include "morello_helpers.h"

char *strcasestr(const char *h, const char *n)
{
	size_t l = strlen(n);
	for (; *h; h++) if (!strncasecmp(h, n, l)) return RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ((char *)h);
	return 0;
}
