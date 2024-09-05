#ifdef __CHERI_PURE_CAPABILITY__

#ifndef SHARED
#include <stddef.h>
#include <stdint.h>
#include <sys/auxv.h>
#include "elf.h"
#include "cap_perms.h"
#include "reloc.h"
#endif

#include "cheri_init_globals_bw.h"

__asm__(
".section .sdata,\"aw\"\n"
".text \n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
#ifndef SHARED
"	cmv cs2, ca0\n"
"	cmv cs3, ca1\n"
"	cmv cs4, ca2\n"
"	cmv cs5, ca3\n"
"	llc ct0, __bakewell_init_static\n"
"	jalr ct0\n"
#else
"	cmv ca5, ca3\n"
"	cmv ca4, ca2\n"
"	cmv ca3, ca1\n"
"	cmv ca2, ca0\n"
#endif
"	cmv ca0, csp\n"
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n"
"	llc ca1, _DYNAMIC\n"
"	andi s6, sp, -16\n\t"
"	scaddr csp, csp, s6\n\t"
#ifndef SHARED
"	cmv ca2, cs2\n"
"	cmv ca3, cs3\n"
"	cmv ca4, cs4\n"
"	cmv ca5, cs5\n"
#endif
"	llc ct0, " START "_c\n"
"	jr  ct0\n"
".size " START ", .-" START "\n"
);

#ifndef SHARED
void
__bakewell_init_static(int argc, char **argv, char **envp, auxv_entry *auxv)
{
	void *rw = NULL, *rx = NULL;
	size_t phnum = 0, phent = 0;
	Elf64_Phdr *ph = NULL;
	for (; auxv->a_type; auxv++) {
		if (auxv->a_type == AT_CHERI_EXEC_RW_CAP) {
			rw = auxv->a_un.a_ptr; // used to derive read-only and rw objects
		} else if (auxv->a_type == AT_CHERI_EXEC_RX_CAP) {
			rx = auxv->a_un.a_ptr; // used to derive function pointers
		} else if (auxv->a_type == AT_PHDR) {
			ph = auxv->a_un.a_ptr;
		} else if (auxv->a_type == AT_PHNUM) {
			phnum = auxv->a_un.a_val;
		} else if (auxv->a_type == AT_PHENT) {
			phent = auxv->a_un.a_val;
		}
		if (rw && rx && ph && phnum && phent) {
			break;
		}
	}

	while(phnum--) {
		if (ph->p_type == PT_INTERP) {
			return;
		}
		ph = (void *)((char *)ph + phent);
	}

	cheri_init_globals_3(rw, rx, rw);
}
#endif

#else /* ! __CHERI_PURE_CAPABILITY__ */
__asm__(
".section .sdata,\"aw\"\n"
".text\n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
".weak __global_pointer$\n"
".hidden __global_pointer$\n"
".option push\n"
".option norelax\n\t"
"lla gp, __global_pointer$\n"
".option pop\n\t"
"mv a0, sp\n"
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n\t"
"lla a1, _DYNAMIC\n\t"
"andi sp, sp, -16\n\t"
"tail " START "_c"
);
#endif
