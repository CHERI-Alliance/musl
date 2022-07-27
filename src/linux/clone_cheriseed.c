#ifdef __SANITIZE_CHERISEED__

#include "pthread_impl.h"

#undef __clone
hidden int __clone(ptraddr_t, ptraddr_t, int, ptraddr_t, ptraddr_t, ptraddr_t, ptraddr_t);

#undef TO_ADDR
#define TO_ADDR(__v) __builtin_cheri_address_get(__v)

#undef VA_ARG
#define VA_ARG(__lst, __ty)                            \
	(__builtin_cheri_length_get(__lst) >=                \
		(__builtin_cheri_offset_get(__lst) + sizeof(__ty)) \
			? __builtin_va_arg(__lst, __ty)                  \
			: ((__ty)0))

int __clone_cheriseed(int (*func)(void *), void *stack, int flags, void *arg, ...) {
	__builtin_va_list lst;
	__builtin_va_start(lst, arg);
	// Arguments pointer is a capability, which is a pointer to some value.
	// Preserve the capability on the child's stack and update the call
	// accordingly.
	// __clone will put 'arg' onto the child stack and pop it back in the child.
	uintptr_t *arg_location = (uintptr_t *)stack - 1;
	*arg_location = (uintptr_t)arg;
	int ret = __clone(
		TO_ADDR(func),
		TO_ADDR(arg_location),
		flags,
		TO_ADDR(arg_location),
		TO_ADDR(VA_ARG(lst, void *)),
		TO_ADDR(VA_ARG(lst, void *)),
		TO_ADDR(VA_ARG(lst, void *))
	);
	__builtin_va_end(lst);
	return ret;
}

#endif
