#include <stddef.h>
#include <stdio.h>

int main (int argc, char *argv[])
{
	void *cap = NULL;
	size_t tag = 0;
	size_t sz = 0;
	for (int k = 0; k < argc; k++) {
		cap = argv[k];
		if (!(tag = __builtin_cheri_tag_get(cap))) {
			printf("argv[%d] has tag %zu\n", k, tag);
			return 1;
		}
		if ((sz = __builtin_cheri_length_get(cap)) == 0ul) {
			printf("argv[%d] has length %zu\n", k, sz);
			return 2;
		}
	}
	cap = argv[argc];
	if ((tag = __builtin_cheri_tag_get(cap))) {
		printf("argv[argc] has tag %zu\n", tag);
		return 3;
	}
	if ((sz = __builtin_cheri_length_get(cap)) != 0xfffffffffffffffful) {
		printf("argv[argc] has length %zu\n", sz);
		return 4;
	}
	return 0;
}
