#if defined __riscv_float_abi_soft
#define RISCV_FP_SUFFIX "-sf"
#elif defined __riscv_float_abi_single
#define RISCV_FP_SUFFIX "-sp"
#elif defined __riscv_float_abi_double
#define RISCV_FP_SUFFIX ""
#endif

#define LDSO_ARCH "riscv64" RISCV_FP_SUFFIX

#define TPOFF_K 0

#define REL_SYMBOLIC    R_RISCV_64
#define REL_PLT         R_RISCV_JUMP_SLOT
#define REL_RELATIVE    R_RISCV_RELATIVE
#define REL_COPY        R_RISCV_COPY
#define REL_DTPMOD      R_RISCV_TLS_DTPMOD64
#define REL_DTPOFF      R_RISCV_TLS_DTPREL64
#define REL_TPOFF       R_RISCV_TLS_TPREL64
#define REL_TLSDESC     R_RISCV_TLSDESC

#ifdef __CHERI_PURE_CAPABILITY__
#define REL_CAPINIT     R_RISCV_CHERI_CAPABILITY /* TODO */
#define REL_CAPRELATIVE R_RISCV_CHERI_RELATIVE
#define REL_FUNCREL     R_RISCV_FUNC_RELATIVE

#define BAKEWELL_RELA_PERM_RX 4 /* TODO */
#define BAKEWELL_RELA_PERM_RW 2 /* TODO */
#define BAKEWELL_RELA_PERM_R 1  /* TODO */

typedef struct {
	uint64_t address;
	uint64_t length : 56;
	uint64_t perms : 8;
} bakewell_reloc_cap_t; /* TODO */
#endif

#ifdef __CHERI_PURE_CAPABILITY__
#define CRTJMP(pc,sp) __asm__ __volatile__( \
	"cmv csp, %1 ; jr %0" : : "C"(pc), "C"(sp) : "memory" )

/*
 * The on-stack value of argc at 0(csp) must be set by the caller.
 * The value in a0 will be ignored.
 */
#define CRTJMPCHERI(pc, sp, argc, argv, envp, auxv) __asm__ __volatile__ ( \
	"mv a0, %0\n" \
	"mv ca1, %1\n" \
	"mv ca2, %2\n" \
	"mv ca3, %3\n" \
	"mv csp, %4\n" \
	"jr %5\n" \
	: : "r" (argc), "C" (argv), "C" (envp), "C" (auxv), "C"(sp), "C"(AUX_PTR(aux[AT_ENTRY])) \
	: "ca0", "ca1", "ca2", "ca3", "memory")
#else
#define CRTJMP(pc,sp) __asm__ __volatile__(			\
	"mv sp, %1 ; jr %0" : : "r"(pc), "r"(sp) : "memory" )
#endif
