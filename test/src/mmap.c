#include <stdlib.h>
#include <sys/mman.h>

#define MEM_PROT    PROT_READ | PROT_WRITE
#define MEM_FLAGS   MAP_PRIVATE | MAP_ANONYMOUS

int main(int argc, char *argv[])
{
	int *p = mmap(NULL, 128, MEM_PROT, MEM_FLAGS, 0, 0);
	if (__builtin_cheri_tag_get(p) != 1ul) {
		return 1;
	}
	if (__builtin_cheri_length_get(p) != 128ul) {
		return 1;
	}
	p[0] = 0;
	int z = p[0];
	munmap(p, 128);
	return z;
}
