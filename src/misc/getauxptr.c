#ifdef __CHERI__

#include "aux_helper.h"
#include <errno.h>
#include "libc.h"

void *__getauxptr(unsigned long item)
{
	// error if asking for a non-pointer from getauxptr()
	//  this list is not a perfect enforcement as it currently supports
	//  both transitional and draft ABIs, which have different capability
	//  entries.
	switch (item) {
		AUX_PTR_CASES
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
