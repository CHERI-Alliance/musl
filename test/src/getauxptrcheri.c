#include <sys/auxv.h>
#include <stdio.h>

#include "cheri_helpers.h"

#define RW_PERMS    (READ_CAP_PERMS | WRITE_CAP_PERMS | ROOT_CAP_PERMS)
#define RX_PERMS    (READ_CAP_PERMS | EXEC_CAP_PERMS | ROOT_CAP_PERMS)

typedef unsigned long uint;

static int check(void *cap, uint req_perms, const char name[]);

int main (void) {

	uint items[] = {
		AT_CHERI_EXEC_RW_CAP,
		AT_CHERI_EXEC_RX_CAP,
		AT_CHERI_SEAL_CAP
	};

	uint req_perms[] = {
		RW_PERMS,
		RX_PERMS,
		SEAL_CAP_PERMS
	};

	const char* names[] = {
		"AT_CHERI_EXEC_RW_CAP",
		"AT_CHERI_EXEC_RX_CAP",
		"AT_CHERI_SEAL_CAP"
	};

	for (int k = 0; k < 3; k++) {
		void *cap = getauxptr(items[k]);
		if (check(cap, req_perms[k], names[k])) {
			return 1;
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
