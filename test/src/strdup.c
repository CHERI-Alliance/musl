#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

int test_strdup_strndup(void);
int test_wcsdup(void);

int main(int argc, char **argv) {
    if (argc < 2) return -1;

    switch(argv[1][0]) {
        case '0':
            return test_strdup_strndup();
        case '1':
            return test_wcsdup();
    }

    return -1;
}

int test_strdup_strndup(void) {
    const char str[] = "this is a test string.";
    char *dup = strdup(str);

    if (strcmp(dup, "this is a test string.")) return 1;
    if (strcmp(dup, str)) return 2;

    free(dup);

    dup = strndup(str, 10);
    if (strcmp(dup, "this is a ")) return 3;

    free(dup);

    return 0;
}

int test_wcsdup(void) {
    const wchar_t str[] = L"this is a test string.";

    wchar_t *dup = wcsdup(str);
    if (wcscmp(dup, L"this is a test string.")) return 1;
    if (wcscmp(dup, str)) return 2;

    return 0;
}
