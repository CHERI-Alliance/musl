#ifndef MALLOC_GLUE_H
#define MALLOC_GLUE_H

#include <stdint.h>
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <sys/auxv.h>
#include "atomic.h"
#include "syscall.h"
#include "libc.h"
#include "lock.h"
#include "dynlink.h"

// use macros to appropriately namespace these.
#define size_classes __malloc_size_classes
#define ctx __malloc_context
#define alloc_meta __malloc_alloc_meta
#define is_allzero __malloc_allzerop
#define dump_heap __dump_heap

#define malloc __libc_malloc_impl
#define realloc __libc_realloc
#define free __libc_free

#define USE_MADV_FREE 0

#if USE_REAL_ASSERT
#include <assert.h>
#else
#undef assert
#define assert(x) do { if (!(x)) a_crash(); } while(0)
#endif

#ifndef __CHERI_PURE_CAPABILITY__
#define brk(p) ((uintptr_t)__syscall(SYS_brk, p))
#endif

#define mmap __mmap
#define madvise __madvise
#define mremap __mremap

#define DISABLE_ALIGNED_ALLOC (__malloc_replaced && !__aligned_alloc_replaced)

static inline uint64_t get_random_secret()
{
	uint64_t secret = (size_t)&secret * 1103515245;
#ifdef __CHERI__
	void * random = getauxptr(AT_RANDOM);
#else
	uintptr_t random = getauxval(AT_RANDOM);
#endif
	if (random) secret = *((uint64_t*)((char*)random + 8));
	return secret;
}

#ifndef PAGESIZE
#define PAGESIZE PAGE_SIZE
#endif

#define MT (libc.need_locks)

#define FUTEX_WAIT_BITSET   9
#define FUTEX_WAKE_BITSET   10
#define FUTEX_PRIVATE_FLAG  128
#define FUTEX_WAIT_BITSET_PRIVATE   (FUTEX_WAIT_BITSET | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_BITSET_PRIVATE   (FUTEX_WAKE_BITSET | FUTEX_PRIVATE_FLAG)

#define FUTEX_BITSET_READER (1U << 0)
#define FUTEX_BITSET_WRITER (1U << 1)
#define FUTEX_BITSET_UPGRADE (1U << 2)

#define W_LOCK (1U << 31)
#define W_WAIT (1U << 30)
#define R_WAIT (1U << 29)
#define U_WAIT (1U << 28)
#define R_MASK ((1U << 28) - 1)

static inline void ll_rdlock(volatile int *lock, int max_tries) {
	int current;
	int try = 0;

	while (1) {
		current = *lock;

		if (current & (W_LOCK | W_WAIT | U_WAIT)) {
			if (try++ < max_tries) {
#if defined(__riscv)
				// pause insn
				__asm__ __volatile__ (".word 0x0100000f" ::: "memory");
#endif
				continue;
			}

			// A writer is waiting or is holding the lock, mark that
			// this reader is going to sleep while attempting to
			// obtain the lock.
			if (!(current & R_WAIT)) {
				// No reader is currently waiting, try to set R_WAIT.
				if (a_cas(lock, current, current | R_WAIT) != current)
					continue;

				current |= R_WAIT;
			}

			// A writer holds the lock. Wait until the state changes from -1.
			__syscall(SYS_futex, lock, FUTEX_WAIT_BITSET_PRIVATE, (uint32_t)current, 0, 0, FUTEX_BITSET_READER);
		} else if (a_cas(lock, current, current + 1) == current)
			break;
	}
}

static inline void ll_wrlock(volatile int *lock, int max_tries) {
	int current;
	int try = 0;

	while (1) {
		current = *lock;

		if (!(current & W_LOCK) && !(current & R_MASK)) {
			// No writer is currently holding the lock and there are
			// no active readers.
			if (a_cas(lock, current, current | W_LOCK) == current)
				// We acquired the write lock. No need to invoke the
				// futex syscall.
				break;

			continue;
		}

		if (try++ < max_tries) {
#if defined(__riscv)
			// pause insn
			__asm__ __volatile__ (".word 0x0100000f" ::: "memory");
#endif
			continue;
		}

		// Lock is busy, either by a writer or reader(s). Mark that a
		// writer is waiting.
		if (!(current & W_WAIT)) {
			if (a_cas(lock, current, current | W_WAIT) != current)
				continue;

			// Update current that one writer is now waiting.
			current |= W_WAIT;
		}

		__syscall(SYS_futex, lock, FUTEX_WAIT_BITSET_PRIVATE, (uint32_t)current, 0, 0, FUTEX_BITSET_WRITER);
	}
}

static inline int ll_try_upgradelock(int *lock) {
	int current;
	int owns_u_wait = 0;

	while (1) {
		current = *lock;

		// We are the only reader.
		if ((current & R_MASK) == 1) {
			// Atomically drop our read count, set W_LOCK, and clear U_WAIT (if we set it)
			int next = ((current - 1) | W_LOCK) & ~U_WAIT;
			if (a_cas(lock, current, next) == current)
				return 1; // Successfully upgraded to write lock

			continue;
		}

		// Another reader is already upgrading.
		if ((current & U_WAIT) && !owns_u_wait)
			return 0;

		// Claim the upgrade bit.
		if (!owns_u_wait) {
			if (a_cas(lock, current, current | U_WAIT) != current)
				continue;

			owns_u_wait = 1;
			current |= U_WAIT;
			continue;
		}

		// Go to sleep on the dedicated UPGRADE channel.
		__syscall(SYS_futex, lock, FUTEX_WAIT_BITSET_PRIVATE,
				  (uint32_t)current, 0, 0, FUTEX_BITSET_UPGRADE);
	}
}

static inline void ll_unlock(int *lock) {
	int current = *lock;

	if (current & W_LOCK) {
		// We are unlocking a writer.
		while (1) {
			int next;

			// First check if we should be waking waiting writers.
			if (current & W_WAIT) {
				// Clear the lock and W_WAIT, but leave R_WAIT intact
				next = current & ~(W_LOCK | W_WAIT);
				if (a_cas(lock, current, next) == current) {
					// N.B. we have to wake all waiting writers, to
					// keep the state correct, but only one will be
					// taking the lock.
					long woken = __syscall(SYS_futex, lock, FUTEX_WAKE_BITSET_PRIVATE,
							  INT_MAX, 0, 0, FUTEX_BITSET_WRITER);
					if (woken == 0 && (current & R_WAIT))
						__syscall(SYS_futex, lock, FUTEX_WAKE_BITSET_PRIVATE,
							  INT_MAX, 0, 0, FUTEX_BITSET_READER);
					break;
				}
			}
			// Then check if we should be waking all waiting readers.
			else if (current & R_WAIT) {
				// Clear the lock and R_WAIT
				next = current & ~(W_LOCK | R_WAIT);
				if (a_cas(lock, current, next) == current) {
					__syscall(SYS_futex, lock, FUTEX_WAKE_BITSET_PRIVATE,
							  INT_MAX, 0, 0, FUTEX_BITSET_READER);
					break;
				}
			}
			// No one is waiting, attempt to clear the lock.
			else {
				next = current & ~W_LOCK;
				if (a_cas(lock, current, next) == current)
					break;
			}

			// Update failed, re-try.
			current = *lock;
		}
	} else {
		// We are unlocking a reader.
		int old = a_fetch_add(lock, -1);

		// Are we the second-to-last reader, and is the last reader
		// waiting to upgrade, then wake this reader who wants to upgrade.
		if ((old & R_MASK) == 2 && (old & U_WAIT))
			__syscall(SYS_futex, lock, FUTEX_WAKE_BITSET_PRIVATE,
					  1, 0, 0, FUTEX_BITSET_UPGRADE);
		// We are the last reader *and* W_WAIT flag was set by a
		// blocked writer. We will wake up one waiter here (readers
		// only ever wakeup writers).
		else if ((old & R_MASK) == 1 && (old & W_WAIT))
			__syscall(SYS_futex, lock, FUTEX_WAKE_BITSET_PRIVATE,
					  1, 0, 0, FUTEX_BITSET_WRITER);
	}
}

#if !defined(MALLOCNG_LOCK_MUTEX) && !defined(MALLOCNG_LOCK_RWLOCK)
#define MALLOCNG_LOCK_MUTEX
#endif

__attribute__((__visibility__("hidden")))
extern int __malloc_lock[1];

__attribute__((__visibility__("hidden")))
extern int __mallocmap_lock[1];

#define LOCK_OBJ_DEF \
void __malloc_atfork(int who) { malloc_atfork(who); } \
int __malloc_lock[1];								  \
int __mallocmap_lock[1]

#if defined(MALLOCNG_LOCK_MUTEX) && defined(MALLOCNG_LOCK_RWLOCK)
#error "Cannot define both MALLOCNG_LOCK_MUTEX and MALLOCNG_LOCK_RWLOCK"
#elif defined(MALLOCNG_LOCK_MUTEX)

#define RDLOCK_IS_EXCLUSIVE 1

static inline void rdlock()
{
	if (MT) LOCK(__malloc_lock);
}
static inline void wrlock()
{
	if (MT) LOCK(__malloc_lock);
}
static inline void unlock()
{
	UNLOCK(__malloc_lock);
}
static inline void upgradelock()
{
}
static inline void mallocmap_rdlock()
{
	if (MT) LOCK(__mallocmap_lock);
}
static inline void mallocmap_wrlock()
{
	if (MT) LOCK(__mallocmap_lock);
}
static inline void mallocmap_unlock()
{
	UNLOCK(__mallocmap_lock);
}

#elif defined(MALLOCNG_LOCK_RWLOCK)

#define RDLOCK_IS_EXCLUSIVE 0

#if !defined(LL_RDLOCK_MAX_TRIES)
#define LL_RDLOCK_MAX_TRIES 50
#endif

#if !defined(LL_WRLOCK_MAX_TRIES)
#define LL_WRLOCK_MAX_TRIES 50
#endif

static inline void rdlock() {
	if (MT) ll_rdlock(__malloc_lock, LL_RDLOCK_MAX_TRIES);
}

static inline void wrlock() {
	if (MT) ll_wrlock(__malloc_lock, LL_WRLOCK_MAX_TRIES);
}

static inline void unlock() {
	if (MT) ll_unlock(__malloc_lock);
}

static inline void upgradelock(void) {
	if (!MT) return;

	if (ll_try_upgradelock(__malloc_lock))
		return;

	unlock();
	wrlock();
}

static inline void mallocmap_rdlock() {
	ll_rdlock(__mallocmap_lock, LL_WRLOCK_MAX_TRIES);
}

static inline void mallocmap_wrlock() {
	ll_wrlock(__mallocmap_lock, LL_WRLOCK_MAX_TRIES);
}

static inline void mallocmap_unlock() {
	ll_unlock(__mallocmap_lock);
}

#else
#error "A lock implementation must be selected"
#endif

static inline void resetlock()
{
	__malloc_lock[0] = 0;
	__mallocmap_lock[0] = 0;
}

static inline void malloc_atfork(int who)
{
	if (who<0) { rdlock(); mallocmap_rdlock(); }
	else if (who>0) resetlock();
	else { unlock(); mallocmap_unlock(); };
}

void *malloc_aligned(size_t n, size_t align);

#endif
