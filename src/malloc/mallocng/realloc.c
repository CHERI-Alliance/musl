#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include "meta.h"

void *realloc(void *p, size_t n)
{
	if (!p) return malloc(n);

	p = __expand_ddc((void *)p);

	if (size_overflows(n)) return 0;

	struct meta *g = get_meta(p);
	int idx = get_slot_index(p);
	size_t stride = get_stride(g);
	unsigned char *start = g->mem->storage + stride*idx;
	unsigned char *end = start + stride - IB;
	size_t old_size = get_nominal_size(p, end);
	size_t avail_size = end-(unsigned char *)p;
	void *new;

/* TODO Those optimisation of realloc create a bunch of alignement issue
 * We will deal with them in a future commit. For now we fallback to using memcpy
 * Also consider those security issues : https://github.com/capablevms/cheri_misidioms/blob/master/cheri_misidioms.ltx#L88 */
	// only resize in-place if size class matches
	if (n <= avail_size && n<MMAP_THRESHOLD
	    && size_to_class(n)+1 >= g->sizeclass) {
		set_size(p, end, n);
#ifdef MORELLO
	return __builtin_cheri_bounds_set(p, n);
#else
	return p;
#endif
	}

	// use mremap if old and new size are both mmap-worthy
	if (g->sizeclass>=48 && n>=MMAP_THRESHOLD) {
		assert(g->sizeclass==63);
		size_t base = (unsigned char *)p-start;
		size_t needed = (n + base + UNIT + IB + 4095) & -4096;
		new = g->maplen*4096UL == needed ? g->mem :
			mremap(g->mem, g->maplen*4096UL, needed, MREMAP_MAYMOVE);
			//TODO for huge size/small pages, the alignement requirement might be of more than one page
			//TODO even without the above point, realloc for a bigger size is very likely to make things unalign
			// (think of a 512 allign going up to 1024 because of the increased size). This would force a memove
			// I'll have to check with other people, but afaik, when one use realloc, they are likely to use it
			// several time on the same object, usually doubling the size each time. In this case, it might be
			// best to overalign the user's pointer to a whole page from the start (or above a certain size)
			// so that we can just remap instead of copy for the following realloc ?
		if (new!=MAP_FAILED) {
			g->mem = new;
			g->maplen = needed/4096;
			p = g->mem->storage + base;
			end = g->mem->storage + (needed - UNIT) - IB;
			*end = 0;
			set_size(p, end, n);
#ifdef MORELLO
			return __builtin_cheri_bounds_set(p, n);
#else
			return p;
#endif
		}
	}

	new = malloc(n);
	if (!new) return 0;
	memcpy(new, p, n < old_size ? n : old_size);
	free(p);
#ifdef MORELLO
	return __builtin_cheri_bounds_set(new, n);
#else
	return new;
#endif
}
