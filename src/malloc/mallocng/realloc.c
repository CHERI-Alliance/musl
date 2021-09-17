#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include "meta.h"

void *realloc(void *user_p, size_t n)
{
	if (!user_p) return malloc(n);
	size_t morello_aligned_n = n;
#ifdef MORELLO
	morello_aligned_n += MAP_KEY_OFFSET;
	// Make sure that when the bounds are narrowed, the user don't have access to the next slot
	morello_aligned_n = __builtin_cheri_round_representable_length(morello_aligned_n);
	size_t alignment_requirement = get_morello_alignment(morello_aligned_n) - UNIT;
	morello_aligned_n += alignment_requirement;
#endif
	if (size_overflows(morello_aligned_n)) return 0;

	void* p = get_wide_capability(user_p);
	struct meta *g = get_meta(p);
	int idx = get_slot_index(p);
	size_t stride = get_stride(g);
	unsigned char *start = g->mem->storage + stride*idx;
	unsigned char *end = start + stride - IB;
	size_t old_size = get_nominal_size(p, end) - MAP_KEY_OFFSET;
	size_t avail_size = end-(unsigned char *)p;
	void *new;

/* TODO Those optimisation of realloc create a bunch of alignement issue
 * We will deal with them in a future commit. For now we fallback to using memcpy
 * Also consider those security issues : https://github.com/capablevms/cheri_misidioms/blob/master/cheri_misidioms.ltx#L88
	// only resize in-place if size class matches
	if (morello_aligned_n <= avail_size && morello_aligned_n<MMAP_THRESHOLD
	    && size_to_class(morello_aligned_n)+1 >= g->sizeclass) {
		set_size(p, end, n); //TODO I need to check if it's possible that the alignement requirement increase here.
		// it's fine if the alignement requirement decrease, we don't have to change anything, but if it increase,
		// this would fail and we would need to fallback to memcpy.
		return restrict_capability(p,n);
	}

	// use mremap if old and new size are both mmap-worthy
	if (g->sizeclass>=48 && morello_aligned_n>=MMAP_THRESHOLD) {
		assert(g->sizeclass==63);
		size_t base = (unsigned char *)p-start;
		size_t needed = (morello_aligned_n + base + GRP_SIZE + IB + 4095) & -4096;
		unmap_narrow_to_wide(p);
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
			end = g->mem->storage + (needed - GRP_SIZE) - IB;
			*end = 0;
			set_size(p, end, n);
			map_narrow_to_wide(p);
			user_p = restrict_capability(p,n);
			return user_p;
		}
	}
*/
	new = malloc(n);
	if (!new) return 0;
	memcpy(new, user_p, n < old_size ? n : old_size);
	free(user_p);
	return new;
}
