#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
	char buffer[64] = {};
	sprintf(buffer, "hello %s %d %p", "morello", argc, (void *)argv);
	printf("%s\n", buffer);
	return strncmp(buffer, "hello morello 1 0x", 18);
}
