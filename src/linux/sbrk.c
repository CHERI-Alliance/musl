#define _BSD_SOURCE
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#ifdef __CHERI_PURE_CAPABILITY__
#include "syscall.h"

void *sbrk(intptr_t inc)
{
	if (inc) return (void *)__syscall_ret(-ENOMEM);
	return (void *)__syscall(SYS_brk, 0);
}
#else
void *sbrk(intptr_t inc)
{
	__syscall_ret(-ENOMEM);
}
#endif
