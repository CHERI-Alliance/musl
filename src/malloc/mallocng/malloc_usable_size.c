#include <stdlib.h>
#include "meta.h"

size_t malloc_usable_size(void *p)
{
	if (!p) return 0;
#ifdef __CHERI_PURE_CAPABILITY__
	p = expand_bounds(p);
#endif
	struct meta *g = get_meta(p);
	int idx = get_slot_index(p);
	size_t stride = get_stride(g);
	unsigned char *start = g->mem->storage + stride*idx;
	unsigned char *end = start + stride - IB;
	return get_nominal_size(p, end);
}
