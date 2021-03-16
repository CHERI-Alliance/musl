#ifdef MORELLO

#include <stdint.h>

void morello_set_bounds_on_cap_array(void ***a) {
  int n = 0;
  for (; (*a)[n]; n++);
  uintptr_t tmp;
  __asm__ volatile ("scbnds %0, %1, %2" : "=C"(tmp) : "C"(*a), "r"((n + 1) * sizeof(uintptr_t)));
  __asm__ volatile ("str %0, [%1]" :: "C"(tmp), "C"(a));
}

#endif
