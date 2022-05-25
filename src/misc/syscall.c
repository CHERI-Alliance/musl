#define _BSD_SOURCE
#include <unistd.h>
#include "syscall.h"
#include <stdarg.h>

#undef syscall

#if defined(__CHERI_PURE_CAPABILITY__)
#undef va_arg
#define va_arg(v, l) ( \
	((v) != ((void*)0) && __builtin_cheri_length_get(v) >= (__builtin_cheri_offset_get(v) + sizeof(l))) \
	? __builtin_va_arg(v, l) : ((l){0})    \
)
#endif

intptr_t syscall(long n, ...)
{
	va_list ap;
	syscall_arg_t a,b,c,d,e,f;
	va_start(ap, n);
	a=va_arg(ap, syscall_arg_t);
	b=va_arg(ap, syscall_arg_t);
	c=va_arg(ap, syscall_arg_t);
	d=va_arg(ap, syscall_arg_t);
	e=va_arg(ap, syscall_arg_t);
	f=va_arg(ap, syscall_arg_t);
	va_end(ap);
	return __syscall_ret(__syscall(n,a,b,c,d,e,f));
}
