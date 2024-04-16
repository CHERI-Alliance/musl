#ifndef SHARED
#include <stddef.h>
#include <stdint.h>
#include <sys/auxv.h>
#include "elf.h"
#include "cap_perms.h"
#include "reloc.h"
#endif

struct capreloc {
  __SIZE_TYPE__ capability_location;
  __SIZE_TYPE__ object;
  __SIZE_TYPE__ offset;
  __SIZE_TYPE__ size;
  __SIZE_TYPE__ permissions;
};

static const __SIZE_TYPE__ function_reloc_flag = (__SIZE_TYPE__)1
                                                 << (__SIZE_WIDTH__ - 1);
static const __SIZE_TYPE__ function_pointer_permissions_mask =
    ~(__SIZE_TYPE__)(__CHERI_CAP_PERMISSION_PERMIT_SEAL__ |
                     __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ |
                     __CHERI_CAP_PERMISSION_PERMIT_STORE__);
static const __SIZE_TYPE__ constant_reloc_flag = (__SIZE_TYPE__)1
                                                 << (__SIZE_WIDTH__ - 2);
static const __SIZE_TYPE__ constant_pointer_permissions_mask =
    ~(__SIZE_TYPE__)(__CHERI_CAP_PERMISSION_PERMIT_SEAL__ |
                     __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ |
                     __CHERI_CAP_PERMISSION_PERMIT_STORE_LOCAL__ |
                     __CHERI_CAP_PERMISSION_PERMIT_STORE__ |
                     __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__);
static const __SIZE_TYPE__ global_pointer_permissions_mask =
    ~(__SIZE_TYPE__)(__CHERI_CAP_PERMISSION_PERMIT_SEAL__ |
                     __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__);

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
