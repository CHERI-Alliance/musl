#include <libintl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static int test_bind_textdomain_codeset(void);
static int test_textdomain(void);

int main(int argc, char *argv[])
{
	if (argc < 2) {
		return 1;
	}
	switch (argv[1][0]) {
	case '0': return test_bind_textdomain_codeset();
	case '1': return test_textdomain();
	default: return 2;
	}
}

static int test_bind_textdomain_codeset(void)
{
	char *res = bind_textdomain_codeset("cheri-alliance.org", 0);
	if (!res || strcmp(res, "UTF-8")) {
		fprintf(stderr,
				"bind_textdomain_codeset(\"cheri-alliance.org\", 0): expected 'UTF-8', actual %s\n",
				res ? res : "NULL");
		return 1;
	}
	res = bind_textdomain_codeset("cheri-alliance.org", "utf-8");
	if (!res || strcmp(res, "UTF-8")) {
		fprintf(stderr,
				"bind_textdomain_codeset(\"cheri-alliance.org\", \"utf-8\"): expected 'UTF-8', actual %s\n",
				res ? res : "NULL");
		return 1;
	}
	res = bind_textdomain_codeset("cheri-alliance.org", 0);
	if (!res || strcmp(res, "UTF-8")) {
		fprintf(stderr,
				"bind_textdomain_codeset(\"cheri-alliance.org\", 0): expected 'UTF-8', actual %s\n",
				res ? res : "NULL");
		return 1;
	}
	res = bind_textdomain_codeset("cheri-alliance.org", "C");
	if (res) {
		fprintf(stderr,
				"bind_textdomain_codeset(\"cheri-alliance.org\", \"c\"): expected NULL, actual %s\n",
				res);
		return 1;
	}
	if (errno != EINVAL) {
		fprintf(stderr,
				"bind_textdomain_codeset(\"cheri-alliance.org\", \"c\"): expected errno=EINVAL, actual %d\n",
				errno);
		return 1;
	}

	return 0;
}

static int test_textdomain(void)
{
	char *res = textdomain(0);
	if (__builtin_cheri_tag_get(res) == 0) {
		return 1;
	}
	printf("res = %s\n", res);
	res = textdomain("cheri-alliance.org");
	printf("res = %s\n", res);
	if (__builtin_cheri_tag_get(res) == 0) {
		return 1;
	}
	return 0;
}
