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

#else

#define PROCESS_CAPRELOCS(DYN, BASE, CAPRW, CAPRO)

#endif
