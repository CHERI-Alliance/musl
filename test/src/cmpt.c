#include <cmpt.h>
#include <stdio.h>
#include <sys/auxv.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "cheri_test_helpers.h"

#define PROTECTED_DATA_VALUE 52
static void print_cap(void* cap, const char* name)
{
	char out[256] = {};
	strfcap(out, 256, "0x%v:%B | %C | Otype: %s | Perms: %P", cap);
	printf("CAP %s | %s\n", name, out);
}

static void use_sealed_data(void* protected_capability)
{
	bool cap_unsealed = !__builtin_cheri_sealed_get(protected_capability);
	bool cap_tag = __builtin_cheri_tag_get(protected_capability);
	size_t cap_perms = __builtin_cheri_perms_get(protected_capability);

	print_cap(protected_capability, "UNSEALED DATA");

	//Cap should have/be:
	//	- Unsealed
	//	- Valid
	//	- Correct permissions
	if(!(	cap_tag &&
		cap_unsealed &&
		(cap_perms & READ_CAP_PERMS) &&
		(cap_perms & WRITE_CAP_PERMS) &&
		(cap_perms & __ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR__)))
	{
		printf("Incorrect properties for protected capability in use_sealed_data\n");
		exit(-1);
	}
	int protected_data = *(int*)protected_capability;
	printf("PROTECTED DATA VALUE: %d\n", protected_data);

	//Data should be correct
	if(protected_data != PROTECTED_DATA_VALUE)
	{
		printf("Dereferenced unsealed data capability has incorrect value: Deref'd %d Correct value %d\n", protected_data, PROTECTED_DATA_VALUE);
		exit(-1);
	}
}

int main(int argc, char** argv)
{
	printf("CMPT START\n");
	int protection_candidate_data = PROTECTED_DATA_VALUE;
	char* protection_candidate = (char*)&protection_candidate_data;

	cap_pair_t sealed_pair = {};
	int create_pair_ret = create_cap_pair(&sealed_pair, protection_candidate, sizeof(protection_candidate_data), (void*)use_sealed_data);

	if(create_pair_ret != 0)
	{
		printf("Create cap pair failed with return value %d and errno str %s\n", create_pair_ret, strerror(errno));
		return create_pair_ret;
	}

	print_cap(sealed_pair.code, "PAIR CODE");
	print_cap(sealed_pair.data, "PAIR DATA");

	bool pair_code_tag = __builtin_cheri_tag_get(sealed_pair.code);
	bool pair_data_tag = __builtin_cheri_tag_get(sealed_pair.data);
	bool pair_code_sealed = __builtin_cheri_sealed_get(sealed_pair.code);
	bool pair_data_sealed = __builtin_cheri_sealed_get(sealed_pair.data);
	bool pair_otypes_match = __builtin_cheri_type_get(sealed_pair.code) == __builtin_cheri_type_get(sealed_pair.data);
	size_t pair_code_perms = __builtin_cheri_perms_get(sealed_pair.code);
	size_t pair_data_perms = __builtin_cheri_perms_get(sealed_pair.data);

	//Caps should have/be:
	//	- Valid tags
	//	- Sealed
	//	- Same object type
	//	- Correct permissions
	if(!(	pair_code_tag &&
		pair_data_tag &&
		pair_code_sealed &&
		pair_data_sealed &&
		pair_otypes_match &&
		(pair_code_perms & READ_CAP_PERMS) &&
		(pair_code_perms & EXEC_CAP_PERMS) &&
		(pair_code_perms & __ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR__) &&
		(pair_data_perms & READ_CAP_PERMS) &&
		(pair_data_perms & WRITE_CAP_PERMS) &&
		(pair_data_perms & __ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR__)))
	{
		printf("Incorrect properties for pair members\n");
		return -1;
	}

	//Branch to function where pair unsealed
	call_to_cap_pair(&sealed_pair);

	printf("CMPT DONE\n");
	return 0;
}
