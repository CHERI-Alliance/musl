#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_perror();
int test_ctermid();
int test_fopencookie();
int test_setvbuf();
int test_getdelim();

int main(int argc, char **argv) {
    if (argc < 2) return -1;

    switch(argv[1][0]) {
        case '0': // perror
            return test_perror();
        case '1': // ctermid
            return test_ctermid();
        case '2': // fopencookie
            return test_fopencookie();
        case '3': // setvbuf
            return test_setvbuf();
        case '4': // getdelim
            return test_getdelim();
    }

    return -1;
}

int test_getdelim() {
    FILE* fp = tmpfile();
    if(!fp) return 1;

    if (fprintf(fp, "test|delimited|string") != 21) return 2;
    rewind(fp);

    char *str1 = NULL;
    size_t n = 0;
    if (getdelim(&str1, &n, '|', fp) != 5) return 3;
    if (strcmp("test|", str1)) return 4;

    char *str2 = malloc(16);
    n = 16;
    if (getdelim((char **) &str2, &n, '|', fp) != 10) return 5;
    if (strcmp("delimited|", str2)) return 6;

    return 0;
}

int test_setvbuf() {
    FILE* fp = tmpfile();
    if(!fp) return 1;

    if (setvbuf(fp, NULL, _IOFBF, 64)) return 2;
    while(fgetc(fp) != EOF); // read whole file

    if (fclose(fp)) return 3;

    return 0;
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
