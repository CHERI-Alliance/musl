#define _GNU_SOURCE
#include <sys/mman.h>
#undef _GNU_SOURCE

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <stdbool.h>

#include "checkmacros.h"

int getpagesize (void);

#define SIZE 300

#define CHECK_ALIGNED(cap, sz) ({ \
	((__builtin_cheri_address_get(cap) & (sz - 1)) == 0) && ((__builtin_cheri_length_get(cap) % sz) == 0);\
})

int x, y, z;

#define ASSIGN_CAPS(dst) ({ \
	dst[3] = &x;            \
	dst[4] = NULL;          \
	dst[5] = &p;            \
	dst[6] = &q;            \
	dst[7] = &y;            \
	dst[8] = &z;            \
})

static int test_remap(bool move)
{
	const size_t page_size = getpagesize();

	int p, q;

	void *mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

	// check page alignment
	if (!CHECK_ALIGNED(mem, page_size)) {
		printf("mmap returned address which is not page-aligned: %#p\n", mem);
		return 1;
	}

	memset(mem, 0, SIZE);

	int **ptr = (int **)mem;
	ASSIGN_CAPS(ptr);

	void *new = NULL;
	if (move) {
		void *g = mmap(NULL, SIZE * 2, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
		memset(g, 0, SIZE * 2);
		new = mremap(mem, SIZE, SIZE * 2, MREMAP_MAYMOVE | MREMAP_FIXED, g);
	} else {
		new = mremap(mem, SIZE, SIZE * 2, MREMAP_MAYMOVE);
	}

	size_t f;

	// check if moved
	if (new != mem) {
		printf("memory moved: %#p --> %#p\n", mem, new);
		/* If memory is moved my mremap we won't be able to use old `mem` for tag comparison */
		int **ref = (int **)malloc(SIZE);
		memset(ref, 0, SIZE);
		ASSIGN_CAPS(ref);
		f = CHECK_MEM_TAGS(new, ref, SIZE);
	} else {
		printf("memory extended: %#p --> %#p\n", mem, new);
		f = CHECK_MEM_TAGS(new, mem, SIZE);
	}

	if (f) {
		printf("%s: mem tags are not equal at offset %zu\n", __func__, f - 1);
		return 2;
	}
	return 0;
}

int main (int argc, char *argv[])
{
	switch (argv[1][0]) {
	case '0': return test_remap(false); // extend
	case '1': return test_remap(true); // move
	}
	printf("unknown test %c\n", argv[1][0]);
	return -1;
}
