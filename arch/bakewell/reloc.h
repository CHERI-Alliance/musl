#if __BYTE_ORDER == __BIG_ENDIAN
#define ENDIAN_SUFFIX "_be"
#else
#define ENDIAN_SUFFIX ""
#endif

#define LDSO_ARCH "aarch64" ENDIAN_SUFFIX

#define NO_LEGACY_INITFINI

#define TPOFF_K 0

#define REL_SYMBOLIC    R_RISCV_64
#define REL_PLT         R_RISCV_JUMP_SLOT /* TODO */
#define REL_RELATIVE    R_RISCV_RELATIVE /* TODO */
#define REL_COPY        R_RISCV_COPY
#define REL_DTPMOD      R_RISCV_TLS_DTPMOD64
#define REL_DTPOFF      R_RISCV_TLS_DTPREL64
#define REL_TPOFF       R_RISCV_TLS_TPREL64
#if 0
#define REL_TLSDESC     R_RISCV_TLSDESC /* TODO */
#endif
#define REL_CAPINIT     R_RISCV_CHERI_CAPABILITY /* TODO */

#define BAKEWELL_RELA_PERM_RX 4 /* TODO */
#define BAKEWELL_RELA_PERM_RW 2 /* TODO */
#define BAKEWELL_RELA_PERM_R 1  /* TODO */

#define CRTJMP(pc,sp) __asm__ __volatile__( \
	"cincoffsetimm csp, %1, 0 ; cjr %0" : : "C"(pc), "C"(sp) : "memory" )

#define CRTJMPCHERI(pc, sp, argc, argv, envp, auxv) __asm__ __volatile__ ( \
	"mv a0, %0\n" \
	"cmove ca1, %1\n" \
	"cmove ca2, %2\n" \
	"cmove ca3, %3\n" \
	"cmove csp,%4 ; jr %5\n" \
	: : "r" (argc), "C" (argv), "C" (envp), "C" (auxv), "C"(sp), "C"(AUX_PTR(aux[AT_ENTRY])) \
	: "ca0", "ca1", "ca2", "ca3", "memory")

typedef struct {
	uint64_t address;
	uint64_t length : 56;
	uint64_t perms : 8;
} bakewell_reloc_cap_t; /* TODO */
