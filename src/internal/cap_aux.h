#ifndef CHERI_HELPERS_H
#define CHERI_HELPERS_H

#if defined(__CHERI_PURE_CAPABILITY__) && defined(LIBSHIM)
#define IF_CHERI_LOAD_PTR(v,s,n) v = *(s + n);
#else
#define IF_CHERI_LOAD_PTR(v,s,n) /* no-op */
#endif

/* Retrieve int argc from stack */
#define IF_CHERI_GET_ARGC(sp, var) IF_CHERI_LOAD_PTR(var, sp, 0)
/* Retrieve char **argv from stack */
#define IF_CHERI_GET_ARGV(sp, var) IF_CHERI_LOAD_PTR(var, sp, 1)
/* Retrieve char **envp from stack */
#define IF_CHERI_GET_ENVP(sp, var) IF_CHERI_LOAD_PTR(var, sp, 2)
/* Retrieve char *auxv from stack */
#define IF_CHERI_GET_AUXV(sp, var) IF_CHERI_LOAD_PTR(var, sp, 3)

#endif