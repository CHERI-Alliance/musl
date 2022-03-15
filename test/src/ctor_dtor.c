#include <stdio.h>

void ctor() __attribute__((constructor));

void ctor() {
	printf("ctor call ");
}

void dtor() __attribute__((destructor));

void dtor() {
	printf("dtor call\n");
}

int main(int argc, char *argv[]) {
	return 0;
}
