#include <signal.h>

#ifdef __CHERI__
#define ELF_NGREG 34
typedef __uintcap_t elf_greg_t, elf_gregset_t[ELF_NGREG];
#else
#define ELF_NGREG 32
typedef unsigned long elf_greg_t, elf_gregset_t[ELF_NGREG];
#endif
#define ELF_NFPREG 33

typedef union __riscv_mc_fp_state elf_fpregset_t;
