#include <sys/auxv.h>
#include <stdio.h>

#include "cheri_test_helpers.h"

#define RW_PERMS    (READ_CAP_PERMS | WRITE_CAP_PERMS | ROOT_CAP_PERMS)
#define RX_PERMS    (READ_CAP_PERMS | EXEC_CAP_PERMS | ROOT_CAP_PERMS)
#define CMPT_ID_PERMS    (__ARM_CAP_PERMISSION_COMPARTMENT_ID__)

typedef unsigned long uint;

static int check(void *cap, uint req_perms, const char name[]);

int main (void) {

	uint items[] = {
		AT_CHERI_EXEC_RW_CAP,
		AT_CHERI_EXEC_RX_CAP,
#ifdef DYNAMIC
		AT_CHERI_INTERP_RW_CAP,
		AT_CHERI_INTERP_RX_CAP,
#endif
		AT_CHERI_STACK_CAP,
		AT_CHERI_SEAL_CAP,
		AT_CHERI_CID_CAP
	};

	uint req_perms[] = {
		RW_PERMS,
		RX_PERMS,
#ifdef DYNAMIC
		RW_PERMS,
		RX_PERMS,
#endif
		RW_PERMS,
		SEAL_CAP_PERMS,
		CMPT_ID_PERMS
	};

	const char* names[] = {
		"AT_CHERI_EXEC_RW_CAP",
		"AT_CHERI_EXEC_RX_CAP",
#ifdef DYNAMIC
		"AT_CHERI_INTERP_RW_CAP",
		"AT_CHERI_INTERP_RX_CAP",
#endif
		"AT_CHERI_STACK_CAP",
		"AT_CHERI_SEAL_CAP",
		"AT_CHERI_CID_CAP"
	};

	/* todo: remove -1 when AT_CHERI_CID_CAP is provided by libshim */
	int n = sizeof(items) / sizeof(uint) - 1;

	for (int k = 0; k < n; k++) {
		void *cap = getauxptr(items[k]);
		if (check(cap, req_perms[k], names[k])) {
			return k + 1;
		}
	}

	return 0;
}

static int check(void *cap, uint req_perms, const char name[])
{
	if (!__builtin_cheri_tag_get(cap)) {
		printf("tag is not set for %s: %#p\n", name, cap);
		return 1;
	}
	if (__builtin_cheri_perms_get(cap) != req_perms) {
		printf("perms are not correct for %s: %#p\n", name, cap);
		return 2;
	}
	if (__builtin_cheri_sealed_get(cap)) {
		printf("sealed capability %s: %#p\n", name, cap);
		return 3;
	}
	return 0;
}
