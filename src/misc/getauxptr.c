#ifdef __CHERI__

#include <sys/auxv.h>
#include <errno.h>
#include "libc.h"

void *__getauxptr(unsigned long item)
{
	// error if asking for a non-pointer from getauxptr()
	//  this list is not a perfect enforcement as it currently supports
	//  both transitional and draft ABIs, which have different capability
	//  entries.
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
		case AT_CHERI_SEAL_CAP:
		{
			auxv_entry *auxv = libc.auxv;
			for (; auxv->a_type; auxv++)
				if (auxv->a_type == item) return auxv->a_un.a_ptr;
		}
	}


	errno = ENOENT;
	return 0;
}

weak_alias(__getauxptr, getauxptr);

#endif
