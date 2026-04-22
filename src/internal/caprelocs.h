#include <elf.h>

#if defined(__riscv_zcheripurecap)

#include "cheri_init_globals.h"

#if defined(__CHERI_CAPABILITY_TABLE__) && (__CHERI_CAPABILITY_TABLE__ > 3)
#define CAPRELOC_TIGHT_BOUNDS	1
#else
#define CAPRELOC_TIGHT_BOUNDS	0
#endif

/*
 * Process the caprelocs section if present.
 * @param DYN A pointer to the dynamic section.
 * @param BASE The base of this executable (to be used for relocation)
 * @param CAPRW A capability that allow writes to all writalbe sections
 * @param CAPRX A capability that allow read/exec access to all sections
 *
 * NOTE: The _rwoff dance ensures that _caprelocs is a capability derived
 * from CAPRW.
 */
#define PROCESS_CAPRELOCS(DYN, BASE, CAPRW, CAPRX) do {				\
	const void *_caprelocs = NULL;						\
	size_t _caprelocssz = 0;						\
	dynv_entry *_dyn = (DYN);						\
	size_t _base = (BASE);							\
	void *_caprw = (CAPRW);							\
	void *_caprx = (CAPRX);							\
	size_t _off = (size_t)_caprx - _base;					\
										\
	for (int i=0; _dyn[i].d_tag != DT_NULL; i++) {				\
		if (_dyn[i].d_tag == DT_RISCV_CHERI___CAPRELOCS)		\
			_caprelocs = _caprx + (_dyn[i].d_un.d_val - _off);	\
		if (_dyn[i].d_tag == DT_RISCV_CHERI___CAPRELOCSSZ)		\
			_caprelocssz = _dyn[i].d_un.d_val;			\
	}									\
										\
	if (_caprelocs && _caprelocssz) {					\
		cheri_init_globals_impl(_caprelocs,				\
		    _caprelocs + _caprelocssz, _caprw, _caprx, _caprx,		\
		    CAPRELOC_TIGHT_BOUNDS, _base);				\
	}									\
} while (0)


#ifdef __CHERI_CAP_PERMISSION_GLOBAL__
#define __SANITIZE_DEFAULT_PERMS __CHERI_CAP_PERMISSION_GLOBAL__
#else
#define __SANITIZE_DEFAULT_PERMS 0
#endif
#define __SANITIZE_RX_PERMS \
	(READ_CAP_PERMS | EXEC_CAP_PERMS | __SANITIZE_DEFAULT_PERMS)
#define __SANITIZE_RW_PERMS \
	(READ_CAP_PERMS | WRITE_CAP_PERMS | __SANITIZE_DEFAULT_PERMS)

#define SANITIZE_CAPS(RX, RW) do {						\
	(RX) = __builtin_cheri_perms_and((RX), __SANITIZE_RX_PERMS);		\
	(RW) = __builtin_cheri_perms_and((RW), __SANITIZE_RW_PERMS);		\
} while (0)

#ifdef __riscv_zcheripurecap

static inline void
cheri_do_caprelative(void *reloc_addr, size_t addend,
		     size_t base, void *rx, void *rw)
{
	size_t offset = __builtin_cheri_address_get(*(void **)reloc_addr);
	size_t len = __builtin_cheri_length_get(*(void **)reloc_addr);
	size_t perms = __builtin_cheri_perms_get(*(void **)reloc_addr);
	size_t is_sealed = __builtin_cheri_sealed_get(*(void **)reloc_addr);
	size_t v_addr = base + offset;
	const bool is_fn = perms & __CHERI_CAP_PERMISSION_EXECUTE__;
	const bool is_rw = perms & __CHERI_CAP_PERMISSION_WRITE__;
	char *cap;
	char *cap_rx = __builtin_cheri_address_set(rx, v_addr);
	char *cap_rw = __builtin_cheri_address_set(rw, v_addr);

	if (is_fn)
		cap = __builtin_cheri_perms_and(cap_rx, READ_CAP_PERMS | EXEC_CAP_PERMS);
	else if (is_rw)
		cap = __builtin_cheri_perms_and(cap_rw, READ_CAP_PERMS | WRITE_CAP_PERMS);
	else
		cap = __builtin_cheri_perms_and(cap_rx, READ_CAP_PERMS);

	if (!is_fn)
		cap = __builtin_cheri_bounds_set_exact(cap, len);
	cap += addend;

	if (is_fn && is_sealed)
		cap = __builtin_cheri_seal_entry(cap);

	*(char **)reloc_addr = cap;
}

#else

static inline void
cheri_do_caprelative(void *reloc_addr, size_t addend,
		     size_t base, void *rx, void *rw)
{
	size_t v_address = base + ((morello_reloc_cap_t *)reloc_addr)->address;
	size_t len = ((morello_reloc_cap_t *)reloc_addr)->length;
	size_t perms = ((morello_reloc_cap_t *)reloc_addr)->perms;
	char *cap;
	char *cap_rx = __builtin_cheri_bounds_set_exact(
		__builtin_cheri_address_set(rx, v_address), len);
	char *cap_rw = __builtin_cheri_bounds_set_exact(
		__builtin_cheri_address_set(rw, v_address), len);
	switch (perms) {
	case MORELLO_RELA_PERM_R:
		cap = __builtin_cheri_perms_and(cap_rx,
			__CHERI_CAP_PERMISSION_GLOBAL__ | READ_CAP_PERMS);
		break;
	case MORELLO_RELA_PERM_RW:
		cap = __builtin_cheri_perms_and(cap_rw,
			__CHERI_CAP_PERMISSION_GLOBAL__ | READ_CAP_PERMS | WRITE_CAP_PERMS);
		break;
	case MORELLO_RELA_PERM_RX:
		cap = __builtin_cheri_perms_and(cap_rx,
			__CHERI_CAP_PERMISSION_GLOBAL__ | READ_CAP_PERMS | EXEC_CAP_PERMS);
		break;
	default:
		cap = __builtin_cheri_perms_and(cap_rx, 0);
	}
	cap += addend;

	*(char **)reloc_addr = cap;
}

#endif

#else

#define PROCESS_CAPRELOCS(DYN, BASE, CAPRW, CAPRO)
#define SANITIZE_CAPS(RX, RW)

#endif
