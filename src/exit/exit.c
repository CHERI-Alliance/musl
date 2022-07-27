#include <stdlib.h>
#include <stdint.h>
#include "libc.h"

static void dummy()
{
}

/* atexit.c and __stdio_exit.c override these. the latter is linked
 * as a consequence of linking either __toread.c or __towrite.c. */
weak_alias(dummy, __funcs_on_exit);
weak_alias(dummy, __stdio_exit);
weak_alias(dummy, _fini);

extern weak hidden void (*const __fini_array_start)(void), (*const __fini_array_end)(void);

static void libc_exit_fini(void)
{
#ifdef __SANITIZE_CHERISEED__
	// CHERIseed puts raw function pointers into fini_array.
	ptraddr_t *fini_addr = (ptraddr_t*)&__fini_array_end - 1 /* null */;
	ptraddr_t *fini_addr_start = (ptraddr_t*)&__fini_array_start;
	void *const pcc = __builtin_cheri_program_counter_get();
	for (; fini_addr >= fini_addr_start; --fini_addr) {
		void *fini = __builtin_cheri_address_set(pcc, *fini_addr);
		((void (*)(void))fini)();
	}
#else
	uintptr_t a = (uintptr_t)&__fini_array_start;
	a += (ptraddr_t)&__fini_array_end - (ptraddr_t)a;
	for (; a>(uintptr_t)&__fini_array_start; a-=sizeof(void(*)()))
		(*(void (**)())(a-sizeof(void(*)())))();
#endif
	_fini();
}

weak_alias(libc_exit_fini, __libc_exit_fini);

_Noreturn void exit(int code)
{
	__funcs_on_exit();
	__libc_exit_fini();
	__stdio_exit();
	_Exit(code);
}
