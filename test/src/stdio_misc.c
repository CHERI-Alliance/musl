#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_perror();
int test_ctermid();
int test_fopencookie();

int main(int argc, char **argv) {
    if (argc < 2) return -1;

    switch(argv[1][0]) {
        case '0': // perror
            return test_perror();
        case '1': // ctermid
            return test_ctermid();
        case '2': // fopencookie
            return test_fopencookie();
    }

    return -1;
}

int test_perror() {
    char template[] = "testXXXXXX";
    char *filename = mktemp(template);

    // file should not exist and should throw error
    FILE *f = fopen(filename, "r");
    if (f) return 1;

    perror("Error");

    return 0;
}

int test_ctermid() {
    char termid[64];
    ctermid(termid);

    if (strncmp("/dev/tty", termid, strlen("/dev/tty"))) return 1;

    return 0;
}

int test_fopencookie() {
    // very simplified test, may need more in depth testing in future if issues arise

    cookie_io_functions_t io_funcs = (cookie_io_functions_t) {
        NULL, NULL,NULL, NULL
    };
    int testcookie = 4;

    FILE *f = fopencookie((void *) &testcookie, "r", io_funcs);
    if (!f) return 1;

    return 0;
}
