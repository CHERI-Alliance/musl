#include <bits/alltypes.h>

void *__shim_syscall(intptr_t cg, long nr, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f);

static inline intptr_t __syscall0(long n)
{
	return __shim_syscall(0, n, 0, 0, 0, 0, 0, 0);
}

static inline intptr_t __syscall1(long n, intptr_t a)
{
	return __shim_syscall(0, n, a, 0, 0, 0, 0, 0);
}

static inline intptr_t __syscall2(long n, intptr_t a, intptr_t b)
{
	return __shim_syscall(0, n, a, b, 0, 0, 0, 0);
}

static inline intptr_t __syscall3(long n, intptr_t a, intptr_t b, intptr_t c)
{
	return __shim_syscall(0, n, a, b, c, 0, 0, 0);
}

static inline intptr_t __syscall4(long n, intptr_t a, intptr_t b, intptr_t c, intptr_t d)
{
	return __shim_syscall(0, n, a, b, c, d, 0, 0);
}

static inline intptr_t __syscall5(long n, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e)
{
	return __shim_syscall(0, n, a, b, c, d, e, 0);
}

static inline intptr_t __syscall6(long n, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f)
{
	return __shim_syscall(0, n, a, b, c, d, e, f);
}
