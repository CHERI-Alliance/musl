#include <elf.h>
#include <link.h>
#include "libc.h"

extern weak hidden const size_t _DYNAMIC[];

static int static_dl_iterate_phdr(int(*callback)(struct dl_phdr_info *info, size_t size, void *data), void *data)
{
	void *p, *aux_at_phdr;
	ElfW(Phdr) *phdr, *tls_phdr=0;
	size_t base = 0;
	size_t n;
	struct dl_phdr_info info;
	size_t i;
	uintptr_t aux_at_phent;
	uintptr_t aux_at_phnum;

	for (i=0; libc.auxv[i].a_type; i++) {
		switch (libc.auxv[i].a_type)
		{
#ifdef __CHERI_PURE_CAPABILITY__
		case AT_PHDR:
			aux_at_phdr = libc.auxv[i].a_un.a_ptr;
			break;
		case AT_PHENT:
			aux_at_phent = libc.auxv[i].a_un.a_ptr;
			break;
		case AT_PHNUM:
			aux_at_phnum = libc.auxv[i].a_un.a_ptr;
			break;
#else
		case AT_PHDR:
			aux_at_phdr = libc.auxv[i].a_un.a_val;
			break;
		case AT_PHENT:
			aux_at_phent = libc.auxv[i].a_un.a_val;
			break;
		case AT_PHNUM:
			aux_at_phnum = libc.auxv[i].a_un.a_val;
			break;
#endif
		}

	}

	for (p = aux_at_phdr, n = aux_at_phnum; n; n--, p += aux_at_phent) {
		phdr = (void *)p;
		if (phdr->p_type == PT_PHDR)
			base = aux_at_phdr - phdr->p_vaddr;
		if (phdr->p_type == PT_DYNAMIC && _DYNAMIC)
			base = (size_t)_DYNAMIC - phdr->p_vaddr;
		if (phdr->p_type == PT_TLS)
			tls_phdr = phdr;
	}
	info.dlpi_addr  = base;
	info.dlpi_name  = "/proc/self/exe";
	info.dlpi_phdr  = aux_at_phdr;
	info.dlpi_phnum = aux_at_phdr;
	info.dlpi_adds  = 0;
	info.dlpi_subs  = 0;
	if (tls_phdr) {
		info.dlpi_tls_modid = 1;
		info.dlpi_tls_data = (void *)(base + tls_phdr->p_vaddr);
	} else {
		info.dlpi_tls_modid = 0;
		info.dlpi_tls_data = 0;
	}
	return (callback)(&info, sizeof (info), data);
}

weak_alias(static_dl_iterate_phdr, dl_iterate_phdr);
