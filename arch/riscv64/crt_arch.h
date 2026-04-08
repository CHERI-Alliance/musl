#ifdef __CHERI_PURE_CAPABILITY__

#ifndef SHARED
#include <stddef.h>
#include <stdint.h>
#include <sys/auxv.h>
#include "elf.h"
#include "cap_perms.h"
#include "reloc.h"
#endif

#include "cheri_init_globals.h"

/*
 * For CHERI the kernel passes most useful information in registers
 * because that gives us correct bounds on argv, auxv and envp.
 * Thus the registers passed by the kernel are
 * - a0: int argc
 * - a1: char **argv
 * - a2: char **envp
 * - a3: AUX vector
 * Additionally, all of this is passed in the traditional way
 * on the stack, too. But there is no guarantee that the stack
 * pointer is sufficient to access all of argv/envp/auxv and even
 * if it was we would have to set bounds manually. Thus we rely
 * on the kernel provided values except for argc because some kernel
 * versions globber a0 with the "return value" of exec() after it is
 * set by startup code.
 *
 * In addition to the kernel this entry point is also called by the
 * interpreter (i.e. the runtime linker). This can either
 * happen implicitly because the kernel loads the interpreter and jumps
 * into its entry point or explicitly because someone calls
 * /lib/ld.so /path/to/my/binary. In the latter case the interpreter is
 * responsible to mangle argc, argv and the aux vector in a way that
 * makes the program think it is being started the normal way.
 */

__asm__(
".section .sdata,\"aw\"\n"
".text \n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
".weak __global_pointer$\n"
".hidden __global_pointer$\n"
"       llc cgp, __global_pointer$\n"
#ifndef SHARED
"	cmv cs3, ca1\n"
"	cmv cs4, ca2\n"
"	cmv cs5, ca3\n"
"	llc ct0, __cheri_init_static\n"
"	jalr ct0\n"
#else
"	cmv ca5, ca3\n"
"	cmv ca4, ca2\n"
"	cmv ca3, ca1\n"
#endif
"	cmv ca0, csp\n"
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n"
"	llc ca1, _DYNAMIC\n"
"	lw  a2, 0(csp)\n"
#ifndef SHARED
"	cmv ca3, cs3\n"
"	cmv ca4, cs4\n"
"	cmv ca5, cs5\n"
#endif
"	llc ct0, " START "_c\n"
"	jr  ct0\n"
".size " START ", .-" START "\n"
);

#ifndef SHARED

/*
 * The dummy argument allows the call above to re-use existing values
 * in a1-a3 without moving them.
 */
void
__cheri_init_static(int dummy, char **argv, char **envp, auxv_entry *auxv)
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

	/*
	 * If we are not a standalone binary the interpreter is
	 * responsible for capability relocations.
	 */
	while(phnum--) {
		if (ph->p_type == PT_INTERP) {
			return;
		}
		ph = (void *)((char *)ph + phent);
	}

	cheri_init_globals_3(rw, rx, rx);
	cheri_init_globals_cbuildcap(rw, rx, rx);
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
