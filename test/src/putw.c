#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#define TMP_DIR "/tmp/morello-musl-tests-open/"
#define IN_TMP_DIR(f) TMP_DIR f

static int test_putw_read_only();
static int test_putw_write_only();

int main(int argc, char *argv[]) {
    if (argc < 2) return -1;

    switch(argv[1][0]) {
        case '0':
            return test_putw_read_only();
        case '1':
            return test_putw_write_only();
    }

    return -1;
}

static int test_putw_read_only() {
    if (putw(0, stdin) != EOF) return 1;

    return 0;
}

static int test_putw_write_only() {
    char template[] = IN_TMP_DIR("testXXXXXX");
    char *filename = mktemp(template);

    FILE *f = fopen(filename, "w");
    if (!f) return 1;

    if (putw(0, f) == EOF) return 1;

    return 0;
}
