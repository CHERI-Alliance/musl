static inline uintptr_t __get_tp()
{
	uintptr_t tp;
#ifdef MORELLO
	__asm__ ("mrs %0,ctpidr_el0" : "=C"(tp));
#else
	__asm__ ("mrs %0,tpidr_el0" : "=r"(tp));
#endif
	return tp;
}

#define TLS_ABOVE_TP
#define GAP_ABOVE_TP 16

#define MC_PC pc
