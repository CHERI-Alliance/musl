#include <bits/alltypes.h>

void *__shim_syscall(intptr_t n, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f);

static inline intptr_t __syscall0(intptr_t n)
{
	return __shim_syscall(n, 0, 0, 0, 0, 0, 0);
}

static inline intptr_t __syscall1(intptr_t n, intptr_t a)
{
	return __shim_syscall(n, a, 0, 0, 0, 0, 0);
}

static inline intptr_t __syscall2(intptr_t n, intptr_t a, intptr_t b)
{
	return __shim_syscall(n, a, b, 0, 0, 0, 0);
}

static inline intptr_t __syscall3(intptr_t n, intptr_t a, intptr_t b, intptr_t c)
{
	return __shim_syscall(n, a, b, c, 0, 0, 0);
}

static inline intptr_t __syscall4(intptr_t n, intptr_t a, intptr_t b, intptr_t c, intptr_t d)
{
	return __shim_syscall(n, a, b, c, d, 0, 0);
}

static inline intptr_t __syscall5(intptr_t n, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e)
{
	return __shim_syscall(n, a, b, c, d, e, 0);
}

static inline intptr_t __syscall6(intptr_t n, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f)
{
	return __shim_syscall(n, a, b, c, d, e, f);
}
