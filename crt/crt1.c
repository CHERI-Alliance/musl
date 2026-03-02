#include <sys/auxv.h>
#include <features.h>
#include "libc.h"

#define START "_start"

#include "crt_arch.h"

int main();
int __libc_start_main(int (*)(), int, char **, char **, auxv_entry *);

#if defined(__CHERI_PURE_CAPABILITY__)
hidden void _start_c(intptr_t *p, void *x, int argc, char *argv[], char *envp[], auxv_entry *auxv)
{
	/* Prefer argc from aux vector. The register value is not reliable. */
	for (unsigned int i = 0; auxv[i].a_type != AT_NULL; ++i) {
		if (auxv[i].a_type == AT_ARGC) {
			argc = auxv[i].a_un.a_val;
			break;
		}
	}
	__libc_start_main(main, argc, argv, envp, auxv);
}
#else
hidden void _start_c(intptr_t *p)
{
	int argc = p[0];
	char **argv = (void *)(p+1);
	char **envp = argv+argc+1;
	int envp_i;
	for (envp_i=0; envp[envp_i]; envp_i++);
	auxv_entry *auxv = (void *)(envp+envp_i+1);
	__libc_start_main(main, argc, argv, envp, auxv);
}
#endif
