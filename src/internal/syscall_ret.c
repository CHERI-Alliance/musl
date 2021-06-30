#include <errno.h>
#include "syscall.h"

intptr_t __syscall_ret(uintptr_t r)
{
	if (r > -4096UL) {
		errno = -r;
		return -1;
	}
	return r;
}
