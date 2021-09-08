#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include "meta.h"

void *realloc(void *user_p, size_t n)
{
	if (!user_p) return malloc(n);
	if (size_overflows(n)) return 0;

	void* p = get_wide_capability(user_p);
	struct meta *g = get_meta(p);
	int idx = get_slot_index(p);
	size_t stride = get_stride(g);
	unsigned char *start = g->mem->storage + stride*idx;
	unsigned char *end = start + stride - IB;
	size_t old_size = get_nominal_size(p, end);
	size_t avail_size = end-(unsigned char *)p;
	void *new;

	// only resize in-place if size class matches
	if (n <= avail_size && n<MMAP_THRESHOLD
	    && size_to_class(n)+1 >= g->sizeclass) {
		set_size(p, end, n);
		return restrict_capability(p,n);
	}

	// use mremap if old and new size are both mmap-worthy
	if (g->sizeclass>=48 && n>=MMAP_THRESHOLD) {
		assert(g->sizeclass==63);
		size_t base = (unsigned char *)p-start;
		size_t needed = (n + base + GRP_SIZE + IB + 4095) & -4096;
		unmap_narrow_to_wide(p);
		new = g->maplen*4096UL == needed ? g->mem :
			mremap(g->mem, g->maplen*4096UL, needed, MREMAP_MAYMOVE);
		if (new!=MAP_FAILED) {
			g->mem = new;
			g->maplen = needed/4096;
			p = g->mem->storage + base;
			end = g->mem->storage + (needed - GRP_SIZE) - IB;
			*end = 0;
			set_size(p, end, n);
			map_narrow_to_wide(p);
			user_p = restrict_capability(p,n);
			return user_p;
		}
	}

	new = malloc(n);
	if (!new) return 0;
	memcpy(new, user_p, n < old_size ? n : old_size);
	free(user_p);
	return new;
}
