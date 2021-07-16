#include <features.h>
#include "libc.h"

#ifdef MORELLO
void morello_init_stack_args(long *argv_ptr, char **argv_cap);
#endif

#define START "_start"

#include "crt_arch.h"

int main();
weak void _init();
weak void _fini();
int __libc_start_main(int (*)(), int, char **,
	void (*)(), void(*)(), void(*)());

void _start_c(intptr_t *p)
{
	int argc = p[0];

	char **argv = (void *)(p+1);

	__libc_start_main(main, argc, argv, _init, _fini, 0);
}
