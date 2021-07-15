#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

#ifdef MUSL_USE_LIBSHIM

void *__shim_syscall(intptr_t n, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f);

#include <bits/alltypes.h>

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

#else /* MUSL_USE_LIBSHIM */

#define __asm_syscall(...) do { \
	__asm__ __volatile__ ( "svc 0" \
	: "=r"(x0) : __VA_ARGS__ : "memory", "cc"); \
	return x0; \
	} while (0)

static inline long __syscall0(long n)
{
	register long x8 __asm__("x8") = n;
	register long x0 __asm__("x0");
	__asm_syscall("r"(x8));
}

static inline long __syscall1(long n, long a)
{
	register long x8 __asm__("x8") = n;
	register long x0 __asm__("x0") = a;
	__asm_syscall("r"(x8), "0"(x0));
}

static inline long __syscall2(long n, long a, long b)
{
	register long x8 __asm__("x8") = n;
	register long x0 __asm__("x0") = a;
	register long x1 __asm__("x1") = b;
	__asm_syscall("r"(x8), "0"(x0), "r"(x1));
}

static inline long __syscall3(long n, long a, long b, long c)
{
	register long x8 __asm__("x8") = n;
	register long x0 __asm__("x0") = a;
	register long x1 __asm__("x1") = b;
	register long x2 __asm__("x2") = c;
	__asm_syscall("r"(x8), "0"(x0), "r"(x1), "r"(x2));
}

static inline long __syscall4(long n, long a, long b, long c, long d)
{
	register long x8 __asm__("x8") = n;
	register long x0 __asm__("x0") = a;
	register long x1 __asm__("x1") = b;
	register long x2 __asm__("x2") = c;
	register long x3 __asm__("x3") = d;
	__asm_syscall("r"(x8), "0"(x0), "r"(x1), "r"(x2), "r"(x3));
}

static inline long __syscall5(long n, long a, long b, long c, long d, long e)
{
	register long x8 __asm__("x8") = n;
	register long x0 __asm__("x0") = a;
	register long x1 __asm__("x1") = b;
	register long x2 __asm__("x2") = c;
	register long x3 __asm__("x3") = d;
	register long x4 __asm__("x4") = e;
	__asm_syscall("r"(x8), "0"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4));
}

static inline long __syscall6(long n, long a, long b, long c, long d, long e, long f)
{
	register long x8 __asm__("x8") = n;
	register long x0 __asm__("x0") = a;
	register long x1 __asm__("x1") = b;
	register long x2 __asm__("x2") = c;
	register long x3 __asm__("x3") = d;
	register long x4 __asm__("x4") = e;
	register long x5 __asm__("x5") = f;
	__asm_syscall("r"(x8), "0"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5));
}

#endif // MUSL_USE_LIBSHIM

/*
 TODO: eventually we want to support VDSO in musl, however while there is no
 support for this in the kernel we will just use normal syscalls
*/
#ifndef MORELLO
#define VDSO_USEFUL
#define VDSO_CGT_SYM "__kernel_clock_gettime"
#define VDSO_CGT_VER "LINUX_2.6.39"
#endif // !defined(MORELLO)

#define IPC_64 0
