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

void _start_c(long *p)
{
	int argc = p[0];

#ifdef MORELLO
	// If Morello is present then we need to convert pointers in
	// the initial stack to capabilities, which means all elements
	// need to be widened
	long* argv_ptr = p + 1;
	char argv[morello_get_init_stack_num_bytes(argv_ptr)];
	morello_init_stack_args(argv_ptr, argv);
#else
	char **argv = (void *)(p+1);
#endif
	__libc_start_main(main, argc, argv, _init, _fini, 0);
}
