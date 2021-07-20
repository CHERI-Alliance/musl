#include <stddef.h>
#include <stdio.h>

int main (int argc, char *argv[], char *envp[])
{
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
		if ((sz = __builtin_cheri_length_get(cap)) == 0ul) {
			printf("envp[%d] has length %zu\n", n, sz);
			return 6;
		}
		envp++;
		n++;
	}
	return 0;
}
