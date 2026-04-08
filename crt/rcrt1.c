#define START "_start"
#define _dlstart_c _start_c
#include "../ldso/dlstart.c"

int main();
weak void _init();
weak void _fini();

#if defined(__CHERI_PURE_CAPABILITY__)

int __libc_start_main(int (*main)(int,char **,char **, char**), int argc,
	char **argv, char **envp, auxv_entry *auxv,
	void (*init_dummy)(), void(*fini_dummy)(), void(*ldso_dummy)());

hidden void __dls2(size_t base, void *map, void *rw, void *sp,
                   unsigned int argc, char **argv, void **envp, void **auxv)
{
	__libc_start_main(main, argc, argv, envp, auxv, _init, _fini,
			  (void *)0);
}
#else
int __libc_start_main(int (*)(), int, char **,
	void (*)(), void(*)(), void(*)());

hidden void __dls2(size_t base, size_t *sp)
{
	__libc_start_main(main, *sp, (void *)(sp+1), _init, _fini, 0);
}

#endif
