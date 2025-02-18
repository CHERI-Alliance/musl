#include <elf.h>
#include <link.h>
#include "pthread_impl.h"
#include "libc.h"

extern weak hidden const size_t _DYNAMIC[];

static int static_dl_iterate_phdr(int(*callback)(struct dl_phdr_info *info, size_t size, void *data), void *data)
{
	void *aux_at_phdr;
	void *rxcap;
	ElfW(Phdr) *phdr, *tls_phdr=0;
	uintptr_t base = 0;
	size_t n;
	struct dl_phdr_info info;
	size_t i;
	size_t aux_at_phent, aux_at_phnum;

	for (i=0; libc.auxv[i].a_type; i++) {
		switch (libc.auxv[i].a_type)
		{
		case AT_PHDR:
			aux_at_phdr = libc.auxv[i].a_un.a_ptr;
			break;
		case AT_PHENT:
			aux_at_phent = libc.auxv[i].a_un.a_val;
			break;
		case AT_PHNUM:
			aux_at_phnum = libc.auxv[i].a_un.a_val;
			break;
		case AT_CHERI_EXEC_RX_CAP:
			rxcap = libc.auxv[i].a_un.a_ptr;
			break;
		}
	}

	for (phdr = aux_at_phdr, n = aux_at_phnum; n; n--) {
		if (phdr->p_type == PT_PHDR)
			base = (uintptr_t)((char *)aux_at_phdr - phdr->p_vaddr);
		if (phdr->p_type == PT_DYNAMIC && _DYNAMIC)
			base = (size_t)_DYNAMIC - phdr->p_vaddr;
		if (phdr->p_type == PT_TLS)
			tls_phdr = phdr;
		/*
		 * We need to advance by the amount of bytes advertised in
		 * the AUX vector, not just by the size of our PHDR struct.
		 */
		phdr = (ElfW(Phdr) *)((char *)phdr + aux_at_phent);
	}

#if defined(__CHERI_PURE_CAPABILITY__)
	// For statically linked C++ programs libunwind needs a capability which covers
	// the entire executable. With the base of 0, this is generally not possible due 
	// to capability compression. Instead we ensure that we leave 'base' as untagged 0,
	// and provide dlpi_phdr which spans the executable mapping to use instead.
	if(base == 0 && !__builtin_cheri_tag_get((void *)base)){
		aux_at_phdr = __builtin_cheri_address_set(rxcap, (size_t)aux_at_phdr);
	}
#endif

	info.dlpi_addr  = base;
	info.dlpi_name  = "/proc/self/exe";
	info.dlpi_phdr  = aux_at_phdr;
	info.dlpi_phnum = aux_at_phnum;
	info.dlpi_adds  = 0;
	info.dlpi_subs  = 0;
	if (tls_phdr) {
		info.dlpi_tls_modid = 1;
		info.dlpi_tls_data = __tls_get_addr((tls_mod_off_t[]){1,0});
	} else {
		info.dlpi_tls_modid = 0;
		info.dlpi_tls_data = 0;
	}
	return (callback)(&info, sizeof (info), data);
}

weak_alias(static_dl_iterate_phdr, dl_iterate_phdr);
