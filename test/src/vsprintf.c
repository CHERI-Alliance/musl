#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static int format(char *buffer, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int z = vsprintf(buffer, fmt, args);
	va_end(args);
	return z;
}

int main (int argc, char *argv[]) {
	char buffer[64] = {};
	format(buffer, "hello %s %d %p", "morello", argc, argv);
	printf("%s\n", buffer);
	return strncmp(buffer, "hello morello 2 0x", 18);
}
