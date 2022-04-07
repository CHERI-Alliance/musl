#ifdef __SANITIZE_CHERISEED__

#define _GNU_SOURCE
#include <unistd.h>
#include "pthread_impl.h"
#include "syscall.h"

#undef __clone

hidden int __clone(ptraddr_t, ptraddr_t, int, ptraddr_t, ptraddr_t, ptraddr_t, ptraddr_t);

int __clone_cheriseed(int (*func)(void *), void *stack, int flags, void *arg, void *ptid, void *tls, void *ctid) {
	// Arguments pointer is a capability, which is a pointer to some value.
	// Preserve the capability on the child's stack and update the call accordingly.
	// __clone will put 'arg' onto the child stack and pop it back in the child.
	uintptr_t *arg_location = (uintptr_t*)stack - 1;
	*arg_location = (uintptr_t)arg;
	return __clone(
		__builtin_cheri_address_get(func),
		__builtin_cheri_address_get(arg_location),
		flags,
		__builtin_cheri_address_get(arg_location),
		__builtin_cheri_address_get(ptid),
		__builtin_cheri_address_get(tls),
		__builtin_cheri_address_get(ctid));
}

#endif
