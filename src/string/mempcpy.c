#define _GNU_SOURCE
#include <string.h>

#include "morello_helpers.h"

void *mempcpy(void *dest, const void *src, size_t n)
{
	return RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ((char *)memcpy(dest, src, n) + n);
}
