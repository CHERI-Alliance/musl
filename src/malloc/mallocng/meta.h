#ifndef MALLOC_META_H
#define MALLOC_META_H

#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include "glue.h"

__attribute__((__visibility__("hidden")))
extern const uint16_t size_classes[];

#define MMAP_THRESHOLD 131052

#define UNIT 16
#define GRP_SIZE 32
#define IB 4

#ifdef MORELLO
#define MAP_KEY_OFFSET UNIT
struct group {
	struct meta *meta;
	unsigned int capability_map_index;
	unsigned char active_idx:5;
	char pad[GRP_SIZE - sizeof(struct meta *) - sizeof(int)- 1];
	unsigned char storage[];
};
#else
#define MAP_KEY_OFFSET 0
struct group {
	struct meta *meta;
	unsigned char active_idx:5;
	char pad[GRP_SIZE - sizeof(struct meta *) - 1];
	unsigned char storage[];
};
#endif

struct meta {
	struct meta *prev, *next;
	struct group *mem;
	volatile int avail_mask, freed_mask;
	size_t last_idx:5;
	size_t freeable:1;
	size_t sizeclass:6;
	size_t maplen:8*sizeof(size_t)-12;
};

struct meta_area {
	uint64_t check;
	struct meta_area *next;
	int nslots;
	struct meta slots[];
};

struct malloc_context {
	uint64_t secret;
#ifndef PAGESIZE
	size_t pagesize;
#endif
	int init_done;
	unsigned mmap_counter;
	struct meta *free_meta_head;
	struct meta *avail_meta;
	size_t avail_meta_count, avail_meta_area_count, meta_alloc_shift;
	struct meta_area *meta_area_head, *meta_area_tail;
	unsigned char *avail_meta_areas;
	struct meta *active[48];
	size_t usage_by_class[48];
	uint8_t unmap_seq[32], bounces[32];
	uint8_t seq;
#ifdef MORELLO
	uint8_t map_count;
	uint8_t allocated_map_table_count; // we allocate more as we need, up to 44
	struct group** capability_map_meta_table[44]; // support at least an hexabyte of memory
#endif
	uintptr_t brk;
};

__attribute__((__visibility__("hidden")))
extern struct malloc_context ctx;

#ifdef PAGESIZE
#define PGSZ PAGESIZE
#else
#define PGSZ ctx.pagesize
#endif

__attribute__((__visibility__("hidden")))
struct meta *alloc_meta(void);

__attribute__((__visibility__("hidden")))
int is_allzero(void *);

static inline void queue(struct meta **phead, struct meta *m)
{
	assert(!m->next);
	assert(!m->prev);
	if (*phead) {
		struct meta *head = *phead;
		m->next = head;
		m->prev = head->prev;
		m->next->prev = m->prev->next = m;
	} else {
		m->prev = m->next = m;
		*phead = m;
	}
}

static inline void dequeue(struct meta **phead, struct meta *m)
{
	if (m->next != m) {
		m->prev->next = m->next;
		m->next->prev = m->prev;
		if (*phead == m) *phead = m->next;
	} else {
		*phead = 0;
	}
	m->prev = m->next = 0;
}

static inline struct meta *dequeue_head(struct meta **phead)
{
	struct meta *m = *phead;
	if (m) dequeue(phead, m);
	return m;
}

static inline void free_meta(struct meta *m)
{
	*m = (struct meta){0};
	queue(&ctx.free_meta_head, m);
}

static inline uint32_t activate_group(struct meta *m)
{
	assert(!m->avail_mask);
	uint32_t mask, act = (2u<<m->mem->active_idx)-1;
	do mask = m->freed_mask;
	while (a_cas(&m->freed_mask, mask, mask&~act)!=mask);
	return m->avail_mask = mask & act;
}

#ifdef MORELLO
size_t get_morello_alignment(size_t len);
#endif

static inline int get_slot_index(const unsigned char *p)
{
	return p[-3] & 31;
}

/*

|
|
|--------- p
|    |
| -2 | offset
| -3 . slot index (5 low bit) and reserve (3 high bit)
| -4 . idk, but this is checked against 0
|    |
|    |
|    |
| -8 | offset, if p -4 != 0

*/

static inline struct meta *get_meta(const unsigned char *p)
{
	assert(!((uintptr_t)p & 15));
	int offset = *(const uint16_t *)(p - 2);
	int index = get_slot_index(p);
	if (p[-4]) {
		assert(!offset);
		offset = *(uint32_t *)(p - 8);
		assert(offset > 0xffff);
	}
	const struct group *base = (const void *)(p - UNIT*offset - GRP_SIZE);
	const struct meta *meta = base->meta;
	assert(meta->mem == base);
	assert(index <= meta->last_idx);
	assert(!(meta->avail_mask & (1u<<index)));
	assert(!(meta->freed_mask & (1u<<index)));
	const struct meta_area *area = (void *)((uintptr_t)meta & -4096);
	assert(area->check == ctx.secret);
	if (meta->sizeclass < 48) {
		assert(offset >= size_classes[meta->sizeclass]*index);
		assert(offset < size_classes[meta->sizeclass]*(index+1));
	} else {
		assert(meta->sizeclass == 63);
	}
	if (meta->maplen) {
		assert(offset <= meta->maplen*4096UL/UNIT - 1);
	}
	return (struct meta *)meta;
}

static inline size_t get_nominal_size(const unsigned char *p, const unsigned char *end)
{
	size_t reserved = p[-3] >> 5;
	if (reserved >= 5) {
		assert(reserved == 5);
		reserved = *(const uint32_t *)(end-4);
		assert(reserved >= 5);
		assert(!end[-5]);
	}
	assert(reserved <= end-p);
	assert(!*(end-reserved));
	// also check the slot's overflow byte
	assert(!*end);
	return end-reserved-p;
}

static inline size_t get_stride(const struct meta *g)
{
	if (!g->last_idx && g->maplen) {
		return g->maplen*4096UL - GRP_SIZE;
	} else {
		return UNIT*size_classes[g->sizeclass];
	}
}

static inline void set_size(unsigned char *p, unsigned char *end, size_t n)
{
	int reserved = end-p-n; // reserved is what "slack" we are left after accounting the offset ?
	if (reserved) end[-reserved] = 0; //and so we force writting a 0 after the last byte the user has requested
	if (reserved >= 5) { //if reserved is too big (we only have 3 bits to represent it at p-3)
		*(uint32_t *)(end-4) = reserved; // then we store it near the end
		end[-5] = 0; // write the null byte
		reserved = 5; // and set reserved to the special value to indicate one must read the field at end-4
	}
	p[-3] = (p[-3]&31) + (reserved<<5);
}

static inline void *enframe(struct meta *g, int idx, size_t n, int ctr)
{
	size_t offsetted_n = n + MAP_KEY_OFFSET;

	size_t stride = get_stride(g);
	size_t slack = (stride-IB-offsetted_n)/UNIT;
	unsigned char *p = g->mem->storage + stride*idx;
	unsigned char *end = p+stride-IB;
	// cycle offset within slot to increase interval to address
	// reuse, facilitate trapping double-free.
	// TODO As far as I can tell, this offset thing is only usefull to reduce address reuse. I can't find
	// anything that use it. I can probably use that to point to the user pointer *after* bound alignment
	size_t required_alignment = UNIT;
#ifdef MORELLO
	required_alignment = get_morello_alignment(offsetted_n);
	size_t align_multiplier = (required_alignment/UNIT) ? (required_alignment/UNIT) : 1;
	int off = (p[-3] ? *(uint16_t *)(p-2) + align_multiplier : ctr) & 255;
	off &= ~(align_multiplier-1); //round down to alignment multiple
	off += (__builtin_align_up(p,required_alignment) - p) / UNIT; //round up p to align it
#else
	int off = (p[-3] ? *(uint16_t *)(p-2) + 1 : ctr) & 255;
#endif
	assert(!p[-4]);
	if (off > slack) {
		size_t m = slack;
		m |= m>>1; m |= m>>2; m |= m>>4;
		off &= m;
		if (off > slack) off -= slack+1;
		assert(off <= slack);
	}
	assert((uintptr_t)(p+UNIT*off)%required_alignment == 0); //make sure the offsetted p is aligned correctly
	if (off) {
		// store offset in unused header at offset zero
		// if enframing at non-zero offset.
		*(uint16_t *)(p-2) = off;
		p[-3] = 7<<5;
		p += UNIT*off;
		// for nonzero offset there is no permanent check
		// byte, so make one.
		p[-4] = 0;
	}
	*(uint16_t *)(p-2) = (size_t)(p-g->mem->storage)/UNIT;
	p[-3] = idx;
	set_size(p, end, offsetted_n);
	return p;
}

static inline int size_to_class(size_t n)
{
	n = (n+IB-1)/UNIT;
	if (n<10) return n;
	n++;
	int i = (28-a_clz_32(n))*4 + 8;
	if (n>size_classes[i+1]) i+=2;
	if (n>size_classes[i]) i++;
	return i;
}

static inline int size_overflows(size_t n)
{
	if (n >= SIZE_MAX/2 - 4096) {
		errno = ENOMEM;
		return 1;
	}
	return 0;
}

static inline void step_seq(void)
{
	if (ctx.seq==255) {
		for (int i=0; i<32; i++) ctx.unmap_seq[i] = 0;
		ctx.seq = 1;
	} else {
		ctx.seq++;
	}
}

static inline void record_seq(int sc)
{
	if (sc-7U < 32) ctx.unmap_seq[sc-7] = ctx.seq;
}

static inline void account_bounce(int sc)
{
	if (sc-7U < 32) {
		int seq = ctx.unmap_seq[sc-7];
		if (seq && ctx.seq-seq < 10) {
			if (ctx.bounces[sc-7]+1 < 100)
				ctx.bounces[sc-7]++;
			else
				ctx.bounces[sc-7] = 150;
		}
	}
}

static inline void decay_bounces(int sc)
{
	if (sc-7U < 32 && ctx.bounces[sc-7])
		ctx.bounces[sc-7]--;
}

static inline int is_bouncing(int sc)
{
	return (sc-7U < 32 && ctx.bounces[sc-7] >= 100);
}

/*
 * This function attempt to find the wide capability related to the
 * user's capability. If used correctly, it should return the same
 * capability used in restrict_capability to get the user capability
 * This will check that the user didn't tamper with the index.
 * What happen on a corrupted index is yet undetermined, but in
 * no case will it return a valid capability on something else that
 * the correct wide capability
 */
void* get_wide_capability(void* user_capability);

/*
 * This function narrow the bounds of a wide capability and set up
 * the index in the userspace. It require the mapping to already
 * for the related group.
 */
void* restrict_capability(void* wide_capability, size_t user_size);

/*
 * This function is to be called upon the creation of a new group before
 * it can be used. ie : one of its slot is given to the user. This adds an
 * entry to the map which allow one to get the wide pointer to the group
 * using an unique index saved in the group.
 * This is safe to call multiple time. If a mapping already exist, no new
 * one will be created
 */
size_t map_narrow_to_wide(void* wide_capability);

/*
 * This clear the map created by map_narrow_to_wide. This is required to
 * avoid memory leak gven that group can be destroyed, which would lead
 * to wrong map entries.
 * This is safe to call multiple time. If the map is already cleared it
 * won't try to clear it again.
 */
void unmap_narrow_to_wide(void* wide_capability);

#endif
