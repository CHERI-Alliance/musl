#if __BYTE_ORDER == __BIG_ENDIAN
#define ENDIAN_SUFFIX "_be"
#else
#define ENDIAN_SUFFIX ""
#endif

#define LDSO_ARCH "aarch64" ENDIAN_SUFFIX

#define NO_LEGACY_INITFINI

#define TPOFF_K 0

#define REL_SYMBOLIC    R_AARCH64_ABS64
#define REL_GOT         R_AARCH64_GLOB_DAT
#define REL_PLT         R_AARCH64_JUMP_SLOT
#define REL_RELATIVE    R_AARCH64_RELATIVE
#define REL_COPY        R_AARCH64_COPY
#define REL_DTPMOD      R_AARCH64_TLS_DTPMOD64
#define REL_DTPOFF      R_AARCH64_TLS_DTPREL64
#define REL_TPOFF       R_AARCH64_TLS_TPREL64
#define REL_TLSDESC     R_AARCH64_TLSDESC

#if defined(__SANITIZE_CHERISEED__)

__attribute__((naked,noreturn))
static void CRTJMP(int argc, char **argv, char **envp, uintptr_t *auxv,
                   void *sp, uintptr_t *entry) {
	__asm__ __volatile__(
		"stp x29, x30, [sp, #-64]!\n"
		"str x4, [sp, #48]\n"
		"stp x2, x3, [sp, #32]\n"
		"stp x0, x1, [sp, #16]\n"
		"mov x0, x5\n"
		"bl  __cheriseed_address_get\n"
		"mov x5, x0\n"
		"ldp x0, x1, [sp, #16]\n"
		"ldp x2, x3, [sp, #32]\n"
		"ldr x4, [sp, #48]\n"
		"ldp x29, x30, [sp], #64\n"
		"mov sp, x4\n"
		"br  x5\n"
	);
}

// Rely on the fact that "__got" is derived from AT_CHERI_INTERP_RX_CAP.
#define GETFUNCSYM(__fp, __sym, __got) \
	{ \
		ptraddr_t sym_addr; \
		__asm__ ( \
			".hidden " #__sym "\n" \
			"adrp %0, " #__sym "\n" \
			"add  %0, %0, :lo12:" #__sym "\n" \
			: "=r"(sym_addr) : : "memory" \
		); \
		*__fp = __builtin_cheri_address_set(__got, sym_addr); \
		*__fp = __builtin_cheri_bounds_set(*__fp, 1); \
	}

#else  // defined(__SANITIZE__CHERISEED__)

#define CRTJMP(pc,sp) __asm__ __volatile__( \
	"mov sp,%1 ; br %0" : : "r"(pc), "r"(sp) : "memory" )

#endif  // defined(__SANITIZE__CHERISEED__)
