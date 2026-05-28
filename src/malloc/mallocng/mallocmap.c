#ifdef __CHERI_PURE_CAPABILITY__

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stddef.h>
#include "cheri_helpers.h"
#include "glue.h"
#include "meta.h"

#include "mallocmap.h"


/*
 * Sharded hash-map implementation.
 *
 * 1. Sharded Concurrency:
 * Each shard maintains its own read/write lock, capacity limit, and entry array.
 *
 * 2. Hashing
 * - Hightest bits (SHARD_SHIFT) of the resulting hash determine the shard index.
 * - Middle-high bits dictate the ideal bucket index within that shard.
 *
 * 3. Open Addressing
 * Dynamic allocations required by hash-map implementations using
 * chaining create a catch-22 problem when the hash-map is supposed to
 * be used by an allocator. Therefore, we use open addressing, which
 * also increases cache locality.
 *
 * 4. Collision Resolution: Linear Probing
 * The hash map uses open addressing with linear probing. If a bucket
 * is occupied, it probes the next adjacent bucket until an empty slot
 * is found.
 *
 * 5. Backward-Shift Deletion (No Tombstones):
 * Traditional open-addressing hash-maps use "tombstones" for deleted
 * elements, which slowly degrade read performance and trigger
 * rehashes. This map uses a Backward-Shift Deletion algorithm
 * instead. When an entry is deleted, subsequent entries in the
 * cluster are evaluated and shifted backwards to close the gap
 * (unless they would jump ahead of their ideal index). This keeps the
 * map optimally clean and balances the work between insert and delete
 * operations. For comparison, a previous tombstone-based
 * implementation of mallocmap spent 8.9% of execution time on inserts
 * and 1.0% on deletes (9.9% total) during the SPEC omnetpp_r
 * benchmark. This backward-shift approach balances the workload,
 * spending 4.1% on inserts and 5.1% on deletes, which reduces the
 * total overhead to 9.2%.
 *
 * 6. Dynamic Localized Resizing:
 * Shards resize independently. When an individual shard's load factor
 * exceeds 75%, it allocates a new double-sized map and linear probes
 * the old entries into the new map, and unmaps the old memory.
 */

#define NOT_PRESENT ((void *) NULL)

#define MINSIZE 8
#define MAXSIZE ((size_t)-1/2 + 1)

#if !defined(MALLOCMAP_SHARD_BITS)
#define MALLOCMAP_SHARD_BITS 4
#endif
#define NUM_SHARDS (1 << MALLOCMAP_SHARD_BITS)
#define SHARD_MASK (NUM_SHARDS - 1)
#define SHARD_SHIFT (64 - MALLOCMAP_SHARD_BITS)

#if !defined(MALLOCMAP_SHARD_RDLOCK_MAX_TRIES)
#define MALLOCMAP_SHARD_RDLOCK_MAX_TRIES 50
#endif

#if !defined(MALLOCMAP_SHARD_WRLOCK_MAX_TRIES)
#define MALLOCMAP_SHARD_WRLOCK_MAX_TRIES 50
#endif

#define GOLDEN_RATIO_64 11400714819323198485ull

typedef struct shard {
	int lock[1];
	size_t mask;
	size_t used;
	MALLOCMAP_ENTRY *entries;
} __attribute__((__aligned__(64))) shard_t;

static inline void shard_rdlock(shard_t *s) {
	if (MT) ll_rdlock(s->lock, MALLOCMAP_SHARD_RDLOCK_MAX_TRIES);
}

static inline void shard_wrlock(shard_t *s) {
	if (MT) ll_wrlock(s->lock, MALLOCMAP_SHARD_WRLOCK_MAX_TRIES);
}

static inline void shard_unlock(shard_t *s) {
	if (MT) ll_unlock(s->lock);
}

static inline int shard_upgradelock(shard_t *s) {
	if (!MT) return 1;

	if (ll_try_upgradelock(s->lock)) return 1;

	int max_tries = 10;
	ll_unlock(s->lock);
	ll_wrlock(s->lock, max_tries);
	return 0;
}


inline static uint64_t mallocmap_keyhash_impl(ptraddr_t k) {
#if defined(__aarch64__) && defined(__ARM_FEATURE_CRC32)
	uint32_t r;
	unsigned x = 42;
	__asm__ volatile(
		"crc32cx %w0, %w1, %x2" : "=r"(r) : "r"(x), "r"(k)
	);
	return ((uint64_t)r | ((uint64_t)r << 32)) * GOLDEN_RATIO_64;
#elif defined(__x86_64__)
	uint32_t r = 42;
	__asm__ volatile(
		"crc32 %1, %0" : "+r"(r) : "r"(k)
	);
	return ((uint64_t)r | ((uint64_t)r << 32)) * GOLDEN_RATIO_64;
#else
	// Fibonacci hashing
	return (uint64_t)k * GOLDEN_RATIO_64;
#endif
}

inline static uint64_t mallocmap_keyhash(void *k) {
	return mallocmap_keyhash_impl(__builtin_cheri_address_get(k));
}

/*
 * Resizes an individual shard. Must be called with the shard write lock held.
 */
static int shard_resize(shard_t *shard, size_t nel) {
	size_t newsize = MINSIZE;
	if (nel > MAXSIZE) nel = MAXSIZE;
	while (newsize < nel) newsize *= 2;

	size_t map_size = ((newsize * sizeof(MALLOCMAP_ENTRY)) + PGSZ - 1) & -PGSZ;
	MALLOCMAP_ENTRY *new_entries = mmap(0, map_size,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

	if (new_entries == MAP_FAILED) return errno;

#ifdef __CHERI_PURE_CAPABILITY__
	new_entries = __builtin_cheri_bounds_set(new_entries, map_size);
	new_entries = __builtin_cheri_perms_and(new_entries, MUSL_CAP_PROT_MALLOC);
#endif

	size_t old_mask = shard->mask;
	MALLOCMAP_ENTRY *old_entries = shard->entries;

	shard->mask = newsize - 1;
	shard->entries = new_entries;
	shard->used = 0;

	if (!old_entries) return 0;

	// Repopulate using linear probing without tombstones
	for (size_t i = 0; i <= old_mask; i++) {
		if (old_entries[i].key != NOT_PRESENT) {
			uint64_t hash = mallocmap_keyhash(old_entries[i].key);
			size_t idx = (hash >> 32) & shard->mask; // Middle-high bits for bucket

			while (shard->entries[idx].key != NOT_PRESENT) {
				idx = (idx + 1) & shard->mask;
			}
			shard->entries[idx] = old_entries[i];
			shard->used++;
		}
	}

	size_t old_map_size = (((old_mask + 1) * sizeof(MALLOCMAP_ENTRY)) + PGSZ - 1) & -PGSZ;
	int err = munmap(old_entries, old_map_size);
	if (err)
		return errno;

	return 0;
}

/*
 * Create a new mallocmap, returns zero on success.
 */
int mallocmap_create(size_t nel, struct __mallocmap_tab *htab) {
	size_t map_size = ((NUM_SHARDS * sizeof(shard_t)) + PGSZ - 1) & -PGSZ;
	shard_t *shards = mmap(0, map_size,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

	if (shards == MAP_FAILED) return ENOMEM;

#ifdef __CHERI_PURE_CAPABILITY__
	shards = __builtin_cheri_bounds_set(shards, map_size);
	shards = __builtin_cheri_perms_and(shards, MUSL_CAP_PROT_MALLOC);
#endif

	size_t per_shard_nel = nel / NUM_SHARDS;
	if (per_shard_nel < MINSIZE) per_shard_nel = MINSIZE;

	for (int i = 0; i < NUM_SHARDS; i++) {
		shards[i].lock[0] = 0;
		shards[i].used = 0;
		shards[i].entries = NULL;
		int err = shard_resize(&shards[i], per_shard_nel);
		if (err) {
			for (int j = 0; j < i; j++) {
				size_t old_map_size = (((shards[j].mask + 1) * sizeof(MALLOCMAP_ENTRY)) + PGSZ - 1) & -PGSZ;
				munmap(shards[j].entries, old_map_size);
			}
			munmap(shards, map_size);
			return ENOMEM;
		}
	}

	htab->entries = (MALLOCMAP_ENTRY *)shards;
	return 0;
}

/*
 * Returns a value for the given key. Uses read-side lock exclusively.
 */
void *mallocmap_find(void *key, struct __mallocmap_tab *htab) {
	MALLOCMAP_ENTRY *e;
	void *res = NULL;
	shard_t *shards = (shard_t *)htab->entries;
	uint64_t hash = mallocmap_keyhash(key);
	shard_t *shard = &shards[hash >> SHARD_SHIFT]; // Top bits for shard

	shard_rdlock(shard);

	size_t idx = (hash >> 32) & shard->mask; // Middle-high bits for bucket
	while ((e = &shard->entries[idx])->key != NOT_PRESENT) {
		if (__builtin_cheri_equal_exact(key, e->key)) {
			res = e->data;
			goto out;
		}
		idx = (idx + 1) & shard->mask;
	}

 out:
	shard_unlock(shard);
	return res;
}

/*
 * Inserts a new value.
 * Takes a direct write lock because malloc() almost always inserts a new pointer,
 * making read-lock upgrades an unnecessary atomic overhead.
 * Returns zero on success. Non zero return values include EEXIST if an entry with
 * the given key already exists and ENOMEM if out of memory.
 */
int mallocmap_insert(void *key, void *data, struct __mallocmap_tab *htab) {
	MALLOCMAP_ENTRY *e;
	shard_t *shards = (shard_t *)htab->entries;
	uint64_t hash = mallocmap_keyhash(key);
	shard_t *shard = &shards[hash >> SHARD_SHIFT]; // Top bits for shard

	shard_wrlock(shard);

	// Double capacity if load factor exceeds 75%. We resize before
	// checking for duplicates, as for mallocmap's use case, there
	// should not be any duplicates on insertion. Otherwise, we would
	// need to recalculate the index after the resize operation again.
	if (shard->used + 1 > shard->mask - (shard->mask / 4)) {
		int err = shard_resize(shard, (shard->mask + 1) * 2);
		if (err) {
			shard_unlock(shard);
			return err;
		}
	}

	size_t idx = (hash >> 32) & shard->mask; // Middle-high bits for bucket
	// Read phase: Ensure the key doesn't already exist
	while ((e = &shard->entries[idx])->key != NOT_PRESENT) {
		if (e->key == key) {
			shard_unlock(shard);
			return EEXIST;
		}
		idx = (idx + 1) & shard->mask;
	}

	e->key = key;
	e->data = data;
	shard->used++;

	shard_unlock(shard);
	return 0;
}

/*
 * Updates an existing value. Does not insert the data if no
 * existing key could be found. Takes read lock to find, upgrades to
 * write lock to execute the swap.
 */
int mallocmap_update(void *old_key, void *new_key, void *new_data, struct __mallocmap_tab *htab) {
	MALLOCMAP_ENTRY *e;
	shard_t *shards = (shard_t *)htab->entries;
	ptraddr_t old_addr = __builtin_cheri_address_get(old_key);
	ptraddr_t new_addr = __builtin_cheri_address_get(new_key);
	uint64_t old_hash = mallocmap_keyhash(old_key);

	// Fast Path: In-place update (Address is identical, only bounds/perms changed)
	// Safe to update within the exact same bucket.
	if (old_addr == new_addr) {
		uint64_t hash = old_hash;
		shard_t *shard = &shards[old_hash >> SHARD_SHIFT];
		size_t idx = (hash >> 32) & shard->mask;

		shard_rdlock(shard);

		while ((e = &shard->entries[idx])->key != NOT_PRESENT) {
			if (!__builtin_cheri_equal_exact(old_key, e->key)) {
				idx = (idx + 1) & shard->mask;
				continue;
			}

			if (!shard_upgradelock(shard)) {
				// Lock upgrade was non-atomic, re-verify element didn't shift
				idx = (hash >> 32) & shard->mask;
				while ((e = &shard->entries[idx])->key != NOT_PRESENT) {
					if (__builtin_cheri_equal_exact(old_key, e->key)) {
						goto do_update_fast;
					}
					idx = (idx + 1) & shard->mask;
				}
				shard_unlock(shard);
				return ENOENT; // Deleted while waiting to upgrade the lock.
			}
		do_update_fast:
			e->key = new_key;
			e->data = new_data;
			shard_unlock(shard);
			return 0;
		}
		shard_unlock(shard);
		return ENOENT;
	}

	// Address changed. Calculate new hash to route the operation.
	uint64_t new_hash = mallocmap_keyhash(new_key);
	size_t old_shard_idx = old_hash >> SHARD_SHIFT;
	size_t new_shard_idx = new_hash >> SHARD_SHIFT;

	// Cross-Shard Update. This is non-atomic wrt. to mallocmap.
	if (old_shard_idx != new_shard_idx) {
		int err = mallocmap_delete(old_key, NULL, htab);
		if (err) return err;
		return mallocmap_insert(new_key, new_data, htab);
	}

	// Same-Shard Update: Atomic delete + insert, but only to acquire the lock once.
	// Address changed, so ideal bucket changed. We must maintain linear probing invariants.
	shard_t *shard = &shards[old_shard_idx];
	shard_wrlock(shard);

	// First, Inline Backward-Shift Delete
	void *deleted_data = NULL;
	size_t idx = (old_hash >> 32) & shard->mask;

	while ((e = &shard->entries[idx])->key != NOT_PRESENT) {
		if (!__builtin_cheri_equal_exact(old_key, e->key)) {
			idx = (idx + 1) & shard->mask;
			continue;
		}

		deleted_data = e->data;
		shard->entries[idx].key = NOT_PRESENT;
		shard->entries[idx].data = NOT_PRESENT;
		shard->used--;

		size_t curr = idx;
		size_t next = (curr + 1) & shard->mask;
		while (shard->entries[next].key != NOT_PRESENT) {
			uint64_t next_hash = mallocmap_keyhash(shard->entries[next].key);
			size_t ideal = (next_hash >> 32) & shard->mask;
			size_t dist_next = (next - ideal) & shard->mask;
			size_t dist_curr = (curr - ideal) & shard->mask;

			if (dist_curr < dist_next) {
				shard->entries[curr] = shard->entries[next];
				shard->entries[next].key = NOT_PRESENT;
				shard->entries[next].data = NOT_PRESENT;
				curr = next;
			}
			next = (next + 1) & shard->mask;
		}
		break;
	}

	if (!deleted_data) {
		shard_unlock(shard);
		return ENOENT;
	}

	// Next, Inline Linear-Probe Insert
	idx = (new_hash >> 32) & shard->mask;
	while (shard->entries[idx].key != NOT_PRESENT)
		idx = (idx + 1) & shard->mask;

	// Note that we do not need to resize the shard, since we
	// previously deleted an element.

	shard->entries[idx].key = new_key;
	shard->entries[idx].data = new_data;
	shard->used++;

	shard_unlock(shard);
	return 0;  // Successful update;
}

/*
 * Deletes item using Backward-Shift Deletion.
 * Starts with a direct write lock because a successful deletion instantly
 * modifies the structural layout of the buckets.
 */
int mallocmap_delete(void *key, void** existing, struct __mallocmap_tab *htab) {
	MALLOCMAP_ENTRY *e;
	shard_t *shards = (shard_t *)htab->entries;
	uint64_t hash = mallocmap_keyhash(key);
	shard_t *shard = &shards[hash >> SHARD_SHIFT]; // Top bits for shard

	shard_wrlock(shard);

	size_t idx = (hash >> 32) & shard->mask; // Middle-high bits for bucket
	while ((e = &shard->entries[idx])->key != NOT_PRESENT) {
		if (!__builtin_cheri_equal_exact(key, e->key)) {
			idx = (idx + 1) & shard->mask;
			continue;
		}

		if (existing)
			*existing = e->data;

		// Wipe the entry
		shard->entries[idx].key = NOT_PRESENT;
		shard->entries[idx].data = NOT_PRESENT;
		shard->used--;

		// Backward shift algorithm to close the gap without tombstones
		size_t curr = idx;
		size_t next = (curr + 1) & shard->mask;

		while (shard->entries[next].key != NOT_PRESENT) {
			uint64_t next_hash = mallocmap_keyhash(shard->entries[next].key);
			size_t ideal = (next_hash >> 32) & shard->mask; // Middle-high bits for ideal bucket

			size_t dist_next = (next - ideal) & shard->mask;
			size_t dist_curr = (curr - ideal) & shard->mask;

			// If next element is further from ideal than it would be at curr, shift it back.
			if (dist_curr < dist_next) {
				shard->entries[curr] = shard->entries[next];
				shard->entries[next].key = NOT_PRESENT;
				shard->entries[next].data = NOT_PRESENT;
				curr = next;
			}
			next = (next + 1) & shard->mask;
		}

		shard_unlock(shard);
		return 0;
	}

	shard_unlock(shard);
	return ENOENT;
}

#endif
