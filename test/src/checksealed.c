#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (!__builtin_cheri_sealed_get(printf)) {
		return 1;
	}
	if (!__builtin_cheri_sealed_get(scanf)) {
		return 2;
	}
	if (!__builtin_cheri_sealed_get(malloc)) {
		return 3;
	}
	if (!__builtin_cheri_sealed_get(qsort)) {
		return 4;
	}

#if defined(DYNAMIC)
	printf(""); /* Ensure printf appears in PLT */

	/**
	 * Only __dso_handle is available for loading and on the same page as the PLT,
	 * so we load it and keep going forward to reach the PLT
	 */
	char **plt;
#if defined(__arch64__)
	__asm__ (
	".weak __dso_handle\n"
	".hidden __dso_handle\n"
	"	adrp %0, __dso_handle\n"
	"	add %0, %0, #:lo12:__dso_handle\n"
	: "=C" (plt) : : "memory");
#elif defined(__riscv)
  __asm__(
	".weak __dso_handle\n"
	".hidden __dso_handle\n"
	" llc %0, __dso_handle\n"
	: "=C"(plt) : : "memory");
#else
#error "Not implemented"
#endif

	plt = __builtin_align_up(plt, 16);
	while (*plt != (char *) printf) {plt++;}
	if (__builtin_cheri_sealed_get(*plt)) {
		printf("sealed plt: printf = %#p -- plt entry = %#p\n", (void *)printf, (void *)*plt);
	} else {
		printf("unsealed plt: printf = %#p -- plt entry = %#p\n", (void *)printf, (void *)*plt);
		return 5;
	}

#endif // defined(DYNAMIC)
	return 0;
}
