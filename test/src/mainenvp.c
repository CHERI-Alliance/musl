#include <stddef.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[], char *envp[])
{
	if (!__builtin_cheri_tag_get(envp)) {
		printf("envp tag is not set: %#p\n", (void *)envp);
		return 4;
	}
	void *cap = NULL;
	size_t tag = 0;
	size_t sz = 0;
	int n = 0;
	while (*envp) {
		cap = envp;
		if (!(tag = __builtin_cheri_tag_get(cap))) {
			printf("envp[%d] has tag %zu\n", n, tag);
			return 5;
		}
		size_t xsz = strlen(*envp) + 1;
		if ((sz = __builtin_cheri_length_get(*envp)) != xsz) {
			return 6;
		}
		printf("envp[%d] has length %zu (expected %zu)\n", n, sz, xsz);
		envp++;
		n++;
	}
	if (__builtin_cheri_length_get(envp) != ((n + 1) * sizeof(char *))) {
		printf("envp length is not set correctly: %zu\n", __builtin_cheri_length_get(envp));
	}
	return 0;
}
