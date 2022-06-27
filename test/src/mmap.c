#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define MEM_PROT    PROT_READ | PROT_WRITE
#define MEM_FLAGS   MAP_PRIVATE | MAP_ANONYMOUS

int test_mmap();
int test_mmap_offset();

int main(int argc, char **argv) {
	if (argc < 2) return -1;

	switch(argv[1][0]) {
		case '0':
			return test_mmap();
		case '1':
			return test_mmap_offset();
	}

	return -1;
}

int test_mmap() {
	size_t len = 128;
	int *p = mmap(NULL, len, MEM_PROT, MEM_FLAGS, 0, 0);
	if (p == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	if (__builtin_cheri_tag_get(p) != 1ul) {
		return 2;
	}
	size_t expected_size = len + (-len & (getpagesize() - 1));
	if (__builtin_cheri_length_get(p) != expected_size) {
		return 3;
	}
	p[0] = 0;
	int z = p[0];
	munmap(p, len);
	return z;
}

int test_mmap_offset() {
	int fd = open(".", O_TMPFILE | O_RDWR | O_EXCL, 600);
	if (fd < 0) return -1;
	off_t offset = getpagesize();
	size_t len = 128;
	if (posix_fallocate(fd, offset, len)) return -2;
	int *p = mmap(NULL, len, MEM_PROT, MAP_PRIVATE, fd, offset);
	if (p == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	if (__builtin_cheri_tag_get(p) != 1ul) {
		return 2;
	}
	size_t expected_size = len + (-len & (getpagesize() - 1));
	if (__builtin_cheri_length_get(p) != expected_size) {
		return 3;
	}
	p[0] = 0;
	int z = p[0];
	munmap(p, len);
	if (close(fd)) return -3;
	return z;
}
