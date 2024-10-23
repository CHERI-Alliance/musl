#if !defined(MUSL_USE_COMPILER_BUILTINS)
static inline uintptr_t __get_tp()
{
	uintptr_t tp;
#ifndef __CHERI_PURE_CAPABILITY__
	__asm__ __volatile__("mv %0, tp" : "=r"(tp));
#else
	__asm__ __volatile__("cmv %0, ctp" : "=C"(tp));
#endif
	return tp;
}
#endif

#define TLS_ABOVE_TP
#define GAP_ABOVE_TP 0

#ifndef __CHERI_PURE_CAPABILITY__
#define DTP_OFFSET 0x800
#endif

#define MC_PC __gregs[0]
