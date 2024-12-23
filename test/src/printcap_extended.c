#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
#include <stdio.h>
#include <stdint.h>
#include <sys/auxv.h>
#include <cheriintrin.h>

int test_csp(void) {
	void *csp = __builtin_cheri_stack_get();
	int n = printf("%+#p\n", csp);
	if(n < 0) return 1;

	return 0;
}

int test_pcc(void) {
	void *pcc = cheri_pcc_get();
	int n = printf("%#+p\n", pcc);
	if (n < 0) return 1;

	return 0;
}

int test_local(void) {
	int x;
	int *p = &x;
#if defined(__riscv_zcherilevels)
	p = cheri_perms_clear(p, CHERI_PERM_CAPABILITY_LEVEL);
#elif !defined (__riscv_zcheripurecap)
	p = cheri_perms_clear(p, CHERI_PERM_GLOBAL);
#endif
	int n = printf("%+#p\n", (void *) p); 
	if (n < 0) return 1;

	return 0;
}

int test_sentry(void) {
	void *sentry = (void *)test_local;
	int m =  printf("%+#lp\n", sentry);
	if(m < 0) return 2;
#ifdef __riscv_zcheripurecap
  void *sentry_invalid = cheri_high_set(sentry, cheri_high_get(sentry));
#else
	void *sentry_invalid = cheri_tag_clear(sentry);
#endif
	m =  printf("%+#lp\n", sentry_invalid);
	if(m < 0) return 2;
#if defined(__riscv_zcherilevels)
	void *sentry_invalid_local = cheri_perms_clear(sentry_invalid, CHERI_PERM_CAPABILITY_LEVEL);
	m =  printf("%+#lp\n", sentry_invalid_local);
	if(m < 0) return 2;
#elif !defined (__riscv_zcheripurecap)
	void *sentry_invalid_local = cheri_perms_clear(sentry_invalid, CHERI_PERM_GLOBAL);
	m =  printf("%+#lp\n", sentry_invalid_local);
	if(m < 0) return 2;
#endif
	return 0;
}

int test_auxv(void) {
	void *entry;
	(void)entry;
#ifndef __riscv_zcheripurecap
	entry = getauxptr(AT_CHERI_SEAL_CAP);
	int m =  printf("%+#lp\n", entry);
	if(m < 0) return 2;
#endif
#ifdef CMPT_ID_PERMS
	entry = getauxptr(AT_CHERI_CID_CAP);
	m =  printf("%+#lp\n", entry);
	if(m < 0) return 2;
#endif
	return 0;
}

int test_cap_null(void) {
	int n = printf("%+#p\n", NULL);
	if(n < 0) return 1;

	int m =  printf("%+#lp\n", NULL);
	if(m < 0) return 2;

	return 0;
}

int main (int argc, char **argv) {
	switch (argv[1][0]) {
	case '0':
		return test_csp();
	case '1':
		return test_pcc();
	case '2':
		return test_local();
	case '3':
		return test_sentry();
	case '4':
		return test_auxv();
	case '5':
		return test_cap_null();
	}

	return 1;
}

#pragma clang diagnostic pop
