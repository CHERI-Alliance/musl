#pragma once

#include <stddef.h>
#include <stdint.h>

#define COMPARE_BYTES(_q, _p, n) ({ \
	char *pb = (char *)_p;          \
	char *qb = (char *)_q;          \
	int u = 0;                      \
	for(size_t k = 0; k < n; k++) { \
		if (pb[k] != qb[k]) {       \
			u = k + 1; break;       \
		}                           \
	}                               \
	u;                              \
})

/* Get memory tag for aligned address */
static inline uint64_t get_mem_tag(ptraddr_t address)
{
	uint64_t res;
	__asm__ volatile (
		"mrs      c1, DDC\n"
		"scvalue  c1, c1, %1\n"
		"ldr      c1, [c1]\n"
		"gctag    %0, c1\n"
		: "=r"(res) : "r"(address));
	return res;
}

#define CHECK_MEM_TAGS(_q, _p, n) ({                            \
	int u = 0;                                                  \
	for(size_t k = 0; k < n && n >=16; k++) {                   \
		ptraddr_t  x = (uintptr_t )_p + k;                      \
		ptraddr_t  y = (uintptr_t )_q + k;                      \
		if ((x & 0xf) == 0 && (y & 0xf) == 0) {                 \
			uint64_t tx = get_mem_tag(x);                       \
			uint64_t ty = get_mem_tag(y);                       \
			if (tx != ty) {                                     \
				printf("tag: %016lx %016lx - %lu %lu\n",        \
					y, x, ty, tx);                              \
				u = k + 1; break;                               \
			} else {                                            \
				printf("tag: %016lx %016lx - %lu %lu\n",        \
					y, x, ty, tx);                              \
			}                                                   \
		}                                                       \
	}                                                           \
	u;                                                          \
})
