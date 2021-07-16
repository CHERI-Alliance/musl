#include <sys/auxv.h>
#include <errno.h>
#include "libc.h"

void *__getauxptr(unsigned long item)
{
	uintptr_t *auxv = libc.auxv;
	if (item == AT_SECURE) return libc.secure;
	for (; *auxv; auxv+=2)
		if (*auxv==item) return auxv[1];
	errno = ENOENT;
	return 0;
}

weak_alias(__getauxptr, getauxptr);