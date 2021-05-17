#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

#if MUSL_USE_LIBSHIM

static inline __INTPTR_TYPE__ __syscall0(__INTPTR_TYPE__ n)
{
	return __shim_syscall(n);
}

static inline __INTPTR_TYPE__ __syscall1(__INTPTR_TYPE__ n, __INTPTR_TYPE__ a)
{
	return __shim_syscall(n, a);
}

static inline __INTPTR_TYPE__ __syscall2(__INTPTR_TYPE__ n, __INTPTR_TYPE__ a, __INTPTR_TYPE__ b)
{
	return __shim_syscall(n, a, b);
}

static inline __INTPTR_TYPE__ __syscall3(__INTPTR_TYPE__ n, __INTPTR_TYPE__ a, __INTPTR_TYPE__ b, __INTPTR_TYPE__ c)
{
	return __shim_syscall(n, a, b, c);
}

static inline __INTPTR_TYPE__ __syscall4(__INTPTR_TYPE__ n, __INTPTR_TYPE__ a, __INTPTR_TYPE__ b, __INTPTR_TYPE__ c, __INTPTR_TYPE__ d)
{
	return __shim_syscall(n, a, b, c, d);
}

static inline __INTPTR_TYPE__ __syscall5(__INTPTR_TYPE__ n, __INTPTR_TYPE__ a, __INTPTR_TYPE__ b, __INTPTR_TYPE__ c, __INTPTR_TYPE__ d, __INTPTR_TYPE__ e)
{
	return __shim_syscall(n, a, b, c, d, e);
}

static inline __INTPTR_TYPE__ __syscall6(__INTPTR_TYPE__ n, __INTPTR_TYPE__ a, __INTPTR_TYPE__ b, __INTPTR_TYPE__ c, __INTPTR_TYPE__ d, __INTPTR_TYPE__ e, __INTPTR_TYPE__ f)
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

#endif MUSL_USE_LIBSHIM

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
