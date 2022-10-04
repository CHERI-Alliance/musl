#define LDSO_ARCH "x86_64"

#define REL_SYMBOLIC    R_X86_64_64
#define REL_OFFSET32    R_X86_64_PC32
#define REL_GOT         R_X86_64_GLOB_DAT
#define REL_PLT         R_X86_64_JUMP_SLOT
#define REL_RELATIVE    R_X86_64_RELATIVE
#define REL_COPY        R_X86_64_COPY
#define REL_DTPMOD      R_X86_64_DTPMOD64
#define REL_DTPOFF      R_X86_64_DTPOFF64
#define REL_TPOFF       R_X86_64_TPOFF64
#define REL_TLSDESC     R_X86_64_TLSDESC

#if defined(__SANITIZE_CHERISEED__)

__attribute__((naked,noreturn))
static void CRTJMP(int argc, char **argv, char **envp, uintptr_t *auxv,
                   void *sp, uintptr_t *entry) {
	__asm__ __volatile__(
		"push %r8\n"
		"push %rbp\n"
		"push %rcx\n"
		"push %rdx\n"
		"push %rsi\n"
		"push %rdi\n"
		"sub  $8, %rsp\n"
		"mov  %r9, %rdi\n"
		"call __cheriseed_address_get\n"
		"add  $8, %rsp\n"
		"pop  %rdi\n"
		"pop  %rsi\n"
		"pop  %rdx\n"
		"pop  %rcx\n"
		"pop  %rbp\n"
		"pop  %rsp\n"
		"jmp  *%rax\n"
	);
}

// Rely on the fact that "__got" is derived from AT_CHERI_INTERP_RX_CAP.
#define GETFUNCSYM(__fp, __sym, __got) \
	{ \
		ptraddr_t sym_addr; \
		__asm__ ( \
			".hidden " #__sym "\n" \
			"lea " #__sym "(%%rip),%0\n" \
			: "=r"(sym_addr) : : "memory" \
		); \
		*__fp = __builtin_cheri_address_set(__got, sym_addr); \
		*__fp = __builtin_cheri_bounds_set(*__fp, 1); \
	}

#else  // defined(__SANITIZE__CHERISEED__)

#define CRTJMP(pc,sp) __asm__ __volatile__( \
	"mov %1,%%rsp ; jmp *%0" : : "r"(pc), "r"(sp) : "memory" )

#define GETFUNCSYM(fp, sym, got) __asm__ ( \
	".hidden " #sym "\n" \
	"	lea " #sym "(%%rip),%0\n" \
	: "=r"(*fp) : : "memory" )

#endif  // defined(__SANITIZE__CHERISEED__)
