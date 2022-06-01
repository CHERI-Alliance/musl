#include <stddef.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[])
{
	if (!__builtin_cheri_tag_get(argv)) {
		printf("argv tag is not set: %#p\n", (void *)argv);
		return 5;
	}
	if (__builtin_cheri_length_get(argv) != (sizeof(char *) * (argc + 1))) {
		printf("argv length is not set correctly: %zu\n", __builtin_cheri_length_get(argv));
		return 6;
	}
	void *cap = NULL;
	size_t tag = 0;
	size_t sz = 0;
	for (int k = 0; k < argc; k++) {
		cap = argv[k];
		if (!(tag = __builtin_cheri_tag_get(cap))) {
			printf("argv[%d] has tag %zu\n", k, tag);
			return 1;
		}
		size_t xsz = (strlen(cap) + 1);
		sz = __builtin_cheri_length_get(cap);
		printf("argv[%d]=`%s` has length %zu (%zu)\n", k, argv[k], sz, xsz);
		if (sz != xsz) {
			return 2;
		}
	}
	cap = argv[argc];
#ifndef __SANITIZE_CHERISEED__
	if ((tag = __builtin_cheri_tag_get(cap))) {
		printf("argv[argc] has tag %zu\n", tag);
		return 3;
	}
#endif
	if ((sz = __builtin_cheri_length_get(cap)) != 0xfffffffffffffffful) {
		printf("argv[argc] has length %zu\n", sz);
		return 4;
	}
	return 0;
}
