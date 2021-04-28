#include <string.h>

#include "morello_helpers.h"

void *__memrchr(const void *m, int c, size_t n)
{
	const unsigned char *s = m;
	c = (unsigned char)c;
	while (n--) if (s[n]==c) return RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ((void *)(s+n));
	return 0;
}

weak_alias(__memrchr, memrchr);
