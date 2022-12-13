#ifndef MUSL_CHERI_HELPERS_H
#define MUSL_CHERI_HELPERS_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __CHERI_PURE_CAPABILITY__

#define RESTRICT_BNDS_IF_CHERI(c, w) __builtin_cheri_bounds_set(c, w)
#define CAP_TAIL_LENGTH(cap) __builtin_cheri_length_get(cap) - __builtin_cheri_offset_get(cap)
#define LT_IF_CHERI_ELSE(a, b, e) (a < b)
#define VA_ARG_IF_IN_BOUNDS(v, l) ( \
	((v) != ((void*)0) && __builtin_cheri_length_get(v) >= (__builtin_cheri_offset_get(v) + sizeof(l))) \
	? va_arg(v, l) : ((l){0})    \
)

/**
 * Change a cap with base B, value V and length L to a cap with base V, value V
 * and length L-(V-B)
 */
inline void *restrict_bounds_to_tail(void *cap) {
  size_t off = __builtin_cheri_offset_get(cap);
  size_t len = __builtin_cheri_length_get(cap);
  return __builtin_cheri_bounds_set(cap, len - off);
}

#else

#define RESTRICT_BNDS_IF_CHERI(c, w) c
#define CAP_TAIL_LENGTH(cap) SIZE_MAX
#define LT_IF_CHERI_ELSE(a, b, e) e
#define VA_ARG_IF_IN_BOUNDS(v, l) va_arg(v, l)

#endif // __CHERI_PURE_CAPABILITY__

#endif // MUSL_CHERI_HELPERS_H
