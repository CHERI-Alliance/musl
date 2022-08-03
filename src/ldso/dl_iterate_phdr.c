#include <elf.h>
#include <link.h>
#include "libc.h"

extern weak hidden const size_t _DYNAMIC[];

static int static_dl_iterate_phdr(int(*callback)(struct dl_phdr_info *info, size_t size, void *data), void *data)
{
	unsigned char *p, *aux_at_phdr;
	void * rx_cap;
	ElfW(Phdr) *phdr, *tls_phdr=0;
	size_t base = 0;
	size_t n;
	struct dl_phdr_info info;
	size_t i, aux[AUX_CNT] = {0};

	for (i=0; libc.auxv[i].a_type; i++) {
		if (libc.auxv[i].a_type<AUX_CNT)
			aux[libc.auxv[i].a_type] = libc.auxv[i].a_un.a_val;
#ifdef __CHERI_PURE_CAPABLIITY__
		if (libc.auxv[i].a_type==AT_CHERI_EXEC_RX_CAP)
			rx_cap = libc.auxv[i].a_un.a_ptr;
#endif
	}

#if defined(__CHERI_PURE_CAPABILITY__) && defined(LIBSHIM)
	aux_at_phdr = p = __builtin_cheri_address_set(rx_cap, (size_t) aux[AT_PHDR]);
#else
	aux_at_phdr = p = (void *)aux[AT_PHDR];
#endif
	for (p=(void *)aux[AT_PHDR],n=aux[AT_PHNUM]; n; n--,p+=aux[AT_PHENT]) {
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
	info.dlpi_phdr  = (void *)aux[AT_PHDR];
	info.dlpi_phnum = aux[AT_PHNUM];
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
