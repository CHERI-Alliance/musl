#include <sys/prctl.h>
#include <stdarg.h>
#include "cheri_helpers.h"
#include "syscall.h"

#ifndef __CHERI_PURE_CAPABILITY__
typedef unsigned long prctl_arg_t;
int prctl(int op, ...)
#else
typedef uintptr_t prctl_arg_t;
int __real_prctl(int op, ...)
#endif
{
	prctl_arg_t x[4];
	int i;
	va_list ap;
	va_start(ap, op);
	for (i=0; i<4; i++) x[i] = VA_ARG_IF_IN_BOUNDS(ap, prctl_arg_t);
	va_end(ap);
	return syscall(SYS_prctl, op, x[0], x[1], x[2], x[3]);
}
