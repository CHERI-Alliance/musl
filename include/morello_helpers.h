#ifndef MUSL_MORELLO_HELPERS_H
#define MUSL_MORELLO_HELPERS_H

#include <stdint.h>

#define RESTRICT_BNDS_IF_MORELLO(c, w) c
#define RESTRICT_BNDS_IF_MORELLO_SUBOBJ(c, w) c
#define RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ(c) c
#define CAP_TAIL_LENGTH(cap) SIZE_MAX
#define LT_IF_MORELLO_ELSE(a, b, e) e

#ifdef MORELLO

#undef RESTRICT_BNDS_IF_MORELLO
#define RESTRICT_BNDS_IF_MORELLO(c, w) __builtin_cheri_bounds_set(c, w)

#undef CAP_TAIL_LENGTH
#define CAP_TAIL_LENGTH(cap) __builtin_cheri_length_get(cap) - __builtin_cheri_offset_get(cap)

#undef LT_IF_MORELLO
#define LT_IF_MORELLO_ELSE(a, b, e) (a < b)

/**
 * Change a cap with base B, value V and length L to a cap with base V, value V
 * and length L-(V-B)
 */
inline void *restrict_bounds_to_tail(void *cap) {
  size_t addr, off, len;
  __asm__ volatile("gcvalue %0, %1" : "=r"(addr) : "C"(cap));
  void *new;
  __asm__ volatile("scvalue %0, %1, %2" : "=C"(new) : "C"(cap), "r"(addr));
  __asm__ volatile("gcoff %0, %1" : "=r"(off) : "C"(cap));
  __asm__ volatile("gclen %0, %1" : "=r"(len) : "C"(cap));
  __asm__ volatile("scbnds %0, %1, %2" : "=C"(new) : "C"(new), "r"(len - off));
  return new;
}

#ifdef MORELLO_ENABLE_SUBOBJECT_BOUNDS

#undef RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ
#define RESTRICT_BOUNDS_TO_TAIL_IF_MORELLO_SUBOBJ(c) restrict_bounds_to_tail(c)

#undef RESTRICT_BNDS_IF_MORELLO_SUBOBJ
#define RESTRICT_BNDS_IF_MORELLO_SUBOBJ(c, w) RESTRICT_BNDS_IF_MORELLO(c, w)

#endif // MORELLO_ENABLE_SUBOBJECT_BOUNDS

#endif // MORELLO

#endif // MUSL_MORELLO_HELPERS_H
