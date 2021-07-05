#include <syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

int main(){
	unsigned int *cpu;
	unsigned int *node;
	char *input_buf = "";

	syscall(SYS_write, 0, input_buf, 8);

	if (!__builtin_cheri_tag_get(input_buf)) return 1;

	syscall(SYS_getcpu, cpu, node);

	if (!__builtin_cheri_tag_get(cpu)) return 2;
	if (!__builtin_cheri_tag_get(node)) return 3;

	// This tests syscall_cp call
	write(0, input_buf, 8);

	if (!__builtin_cheri_tag_get(input_buf)) return 4;

	intptr_t point = syscall(SYS_mmap, 0, 100, PROT_NONE, MAP_PRIVATE|MAP_ANON, -1, 0);

	if (!__builtin_cheri_tag_get(point)) return 5;

	return 0;
}
