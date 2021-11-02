#include <stdlib.h>
#include <string.h>

static int test_large_allocation();
static int test_many_allocations();

int main(int argc, char *argv[]) {
	switch (argv[1][0]) {
	case '0': return test_large_allocation();
	case '1': return test_many_allocations();
	default: return 1; // bad test number
	}
}

static int test_large_allocation()
{
	void *p = malloc(1024 * 1024 * 10);
	if (p == NULL) {
		return 3;
	}
	memset(p, 4, 1024 * 1024 * 10);
	free(p);
	return 0;
}

static int test_many_allocations()
{
	for (int k = 0; k < 10000; k++) {
		void *q = malloc(517);
		if (q == NULL) {
			return 2;
		}
		void *p = malloc(1024 * (k % 100 + 1));
		if (p == NULL) {
			return 2;
		}
		if ((k % 30) == 0) {
			free(p);
		}
		free(q);
	}
	return 0;
}
