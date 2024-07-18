#include <stdio.h>

void ctor(void) __attribute__((constructor));
void ctor2(void) __attribute__((constructor));

void ctor(void) {
	printf("ctor call ");
}

void ctor2(void) {
	fprintf(stderr, "ctor call2 ");
}
void dtor(void) __attribute__((destructor));
void dtor2(void) __attribute__((destructor));

void dtor(void) {
	printf("dtor call\n");
}

void dtor2(void) {
	fprintf(stderr,"dtor call2\n");
}

int main(int argc, char *argv[]) {
	return 0;
}
