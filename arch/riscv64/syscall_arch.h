#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

#ifdef __CHERI_PURE_CAPABILITY__
#define __asm_syscall(...) \
	__asm__ __volatile__ ("ecall\n\t" \
	: "=C"(a0) : __VA_ARGS__ : "memory"); \
	return a0; \

static inline intptr_t __syscall0(intptr_t n)
{
	register intptr_t a7 __asm__("ca7") = n;
	register intptr_t a0 __asm__("ca0");
	__asm_syscall("C"(a7))
}

static inline intptr_t __syscall1(intptr_t n, intptr_t a)
{
	register intptr_t a7 __asm__("ca7") = n;
	register intptr_t a0 __asm__("ca0") = a;
	__asm_syscall("C"(a7), "0"(a0))
}

static inline intptr_t __syscall2(intptr_t n, intptr_t a, intptr_t b)
{
	register intptr_t a7 __asm__("ca7") = n;
	register intptr_t a0 __asm__("ca0") = a;
	register intptr_t a1 __asm__("ca1") = b;
	__asm_syscall("C"(a7), "0"(a0), "C"(a1))
}

static inline intptr_t __syscall3(intptr_t n, intptr_t a, intptr_t b, intptr_t c)
{
	register intptr_t a7 __asm__("ca7") = n;
	register intptr_t a0 __asm__("ca0") = a;
	register intptr_t a1 __asm__("ca1") = b;
	register intptr_t a2 __asm__("ca2") = c;
	__asm_syscall("C"(a7), "0"(a0), "C"(a1), "C"(a2))
}

static inline intptr_t __syscall4(intptr_t n, intptr_t a, intptr_t b, intptr_t c, intptr_t d)
{
	register intptr_t a7 __asm__("ca7") = n;
	register intptr_t a0 __asm__("ca0") = a;
	register intptr_t a1 __asm__("ca1") = b;
	register intptr_t a2 __asm__("ca2") = c;
	register intptr_t a3 __asm__("ca3") = d;
	__asm_syscall("C"(a7), "0"(a0), "C"(a1), "C"(a2), "C"(a3))
}

static inline intptr_t __syscall5(intptr_t n, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e)
{
	register intptr_t a7 __asm__("ca7") = n;
	register intptr_t a0 __asm__("ca0") = a;
	register intptr_t a1 __asm__("ca1") = b;
	register intptr_t a2 __asm__("ca2") = c;
	register intptr_t a3 __asm__("ca3") = d;
	register intptr_t a4 __asm__("ca4") = e;
	__asm_syscall("C"(a7), "0"(a0), "C"(a1), "C"(a2), "C"(a3), "C"(a4))
}

static inline intptr_t __syscall6(intptr_t n, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f)
{
	register intptr_t a7 __asm__("ca7") = n;
	register intptr_t a0 __asm__("ca0") = a;
	register intptr_t a1 __asm__("ca1") = b;
	register intptr_t a2 __asm__("ca2") = c;
	register intptr_t a3 __asm__("ca3") = d;
	register intptr_t a4 __asm__("ca4") = e;
	register intptr_t a5 __asm__("ca5") = f;
	__asm_syscall("C"(a7), "0"(a0), "C"(a1), "C"(a2), "C"(a3), "C"(a4), "C"(a5))
}
#else
#define __asm_syscall(...)		  \
	__asm__ __volatile__ ("ecall\n\t" \
	: "=r"(a0) : __VA_ARGS__ : "memory"); \
	return a0; \

static inline long __syscall0(long n)
{
	register long a7 __asm__("a7") = n;
	register long a0 __asm__("a0");
	__asm_syscall("r"(a7))
}

static inline long __syscall1(long n, long a)
{
	register long a7 __asm__("a7") = n;
	register long a0 __asm__("a0") = a;
	__asm_syscall("r"(a7), "0"(a0))
}

static inline long __syscall2(long n, long a, long b)
{
	register long a7 __asm__("a7") = n;
	register long a0 __asm__("a0") = a;
	register long a1 __asm__("a1") = b;
	__asm_syscall("r"(a7), "0"(a0), "r"(a1))
}

static inline long __syscall3(long n, long a, long b, long c)
{
	register long a7 __asm__("a7") = n;
	register long a0 __asm__("a0") = a;
	register long a1 __asm__("a1") = b;
	register long a2 __asm__("a2") = c;
	__asm_syscall("r"(a7), "0"(a0), "r"(a1), "r"(a2))
}

static inline long __syscall4(long n, long a, long b, long c, long d)
{
	register long a7 __asm__("a7") = n;
	register long a0 __asm__("a0") = a;
	register long a1 __asm__("a1") = b;
	register long a2 __asm__("a2") = c;
	register long a3 __asm__("a3") = d;
	__asm_syscall("r"(a7), "0"(a0), "r"(a1), "r"(a2), "r"(a3))
}

static inline long __syscall5(long n, long a, long b, long c, long d, long e)
{
	register long a7 __asm__("a7") = n;
	register long a0 __asm__("a0") = a;
	register long a1 __asm__("a1") = b;
	register long a2 __asm__("a2") = c;
	register long a3 __asm__("a3") = d;
	register long a4 __asm__("a4") = e;
	__asm_syscall("r"(a7), "0"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4))
}

static inline long __syscall6(long n, long a, long b, long c, long d, long e, long f)
{
	register long a7 __asm__("a7") = n;
	register long a0 __asm__("a0") = a;
	register long a1 __asm__("a1") = b;
	register long a2 __asm__("a2") = c;
	register long a3 __asm__("a3") = d;
	register long a4 __asm__("a4") = e;
	register long a5 __asm__("a5") = f;
	__asm_syscall("r"(a7), "0"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5))
}
#endif

#define VDSO_USEFUL
#define VDSO_CGT_SYM "__vdso_clock_gettime"
#define VDSO_CGT_VER "LINUX_4.15"

#define IPC_64 0
