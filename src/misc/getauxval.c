#include <sys/auxv.h>
#include <errno.h>
#include "libc.h"

unsigned long __getauxval(unsigned long item)
{
#ifdef __CHERI__
	// error if asking for a pointer from getauxval()
	switch (item) {
		case AT_ENTRY:
		case AT_PHDR:
		case AT_BASE:
		case AT_SYSINFO_EHDR:
		case AT_EXECFN:
		case AT_RANDOM:
		case AT_PLATFORM:
		case AT_CHERI_EXEC_RW_CAP:
		case AT_CHERI_EXEC_RX_CAP:
		case AT_CHERI_INTERP_RW_CAP:
		case AT_CHERI_INTERP_RX_CAP:
		case AT_CHERI_STACK_CAP:
		case AT_CHERI_SEAL_CAP:
			goto error;
	}
#endif

	auxv_entry *auxv = libc.auxv;
	if (item == AT_SECURE) return libc.secure;
	for (; auxv->a_type; auxv++)
		if (auxv->a_type == item) return auxv->a_un.a_val;

error:
	errno = ENOENT;
	return 0;
}

weak_alias(__getauxval, getauxval);
