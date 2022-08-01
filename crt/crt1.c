#include <sys/auxv.h>
#include <features.h>
#include "libc.h"
#include "cap_aux.h"

#define START "_start"

#include "crt_arch.h"

int main();
int __libc_start_main(int (*)(), int, char **, char **, auxv_entry *);

void _start_c(intptr_t *p)
{
	int argc = p[0];

	char **argv = (void *)(p+1);
	IF_CHERI_GET_ARGV(p, argv);
	char **envp = argv+argc+1;
	IF_CHERI_GET_ENVP(p, envp);
	int envp_i;
	for (envp_i=0; envp[envp_i]; envp_i++);
	auxv_entry *auxv = (void *)(envp+envp_i+1);
	IF_CHERI_GET_AUXV(p, auxv);
	__libc_start_main(main, argc, argv, envp, auxv);
}
