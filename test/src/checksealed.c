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
	return 0;
}
