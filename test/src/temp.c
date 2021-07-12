#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define NUMITERS 10

#define TMP_DIR "/tmp/morello-musl-tests-temp/"
#define IN_TMP_DIR(f) TMP_DIR f

int test_mktemp();
int test_mkstemp();
int test_mkostemp();
int test_mkstemps();
int test_mkostemps();
int test_mkdtemp();

int main(int argc, char **argv) {
    if (argc < 2) return -1;

    // set up root dir in /tmp
    if (mkdir(TMP_DIR, 0700) && errno != EEXIST) return -2;

    switch(argv[1][0]) {
        case '0': // temp-mktemp
            return test_mktemp();
        case '1': // temp-mkstemp
            return test_mkstemp();
        case '2': // temp-mkostemp
            return test_mkostemp();
        case '3': // temp-mkstemps
            return test_mkstemps();
        case '4': // temp-mkostemps
            return test_mkostemps();
        case '5': // temp-mkdtemp
            return test_mkdtemp();
    }

    return -1;
}

int test_mktemp() {
    struct stat statbuf;

    for (int i = 0; i < NUMITERS; i++) {
        char template[] = IN_TMP_DIR("testXXXXXX");
        char *filename;

        filename = mktemp(template);

        if(!stat(filename, &statbuf)) return 1; // file shouldn't exist

        if (__builtin_cheri_base_get(filename) != __builtin_cheri_base_get(template)) return 2;
        if (__builtin_cheri_length_get(filename) != __builtin_cheri_length_get(template)) return 3;
        if (__builtin_cheri_perms_get(filename) != __builtin_cheri_perms_get(template)) return 4;
        if (__builtin_cheri_tag_get(filename) != __builtin_cheri_tag_get(template)) return 5;

        if(strncmp("test", filename + strlen(TMP_DIR), 4)) return 6;
    }

    return 0;
}

int test_mkstemp() {
    struct stat statbuf;

    for (int i = 0; i < NUMITERS; i++) {
        char template[] = IN_TMP_DIR("testXXXXXX");

        int fd = mkstemp(template);

        if (fd < 0) return 1;
        if (stat(template, &statbuf)) return 2; // file should exist
        if (strncmp("test", template  + strlen(TMP_DIR), 4)) return 3;

        unlink(template); // unlink to ensure delete on close
        close(fd);

        if (!stat(template, &statbuf)) return 4; // file should be gone
    }

    return 0;
}

int test_mkostemp() {
    struct stat statbuf;

    for (int i = 0; i < NUMITERS; i++) {
        char template[] = IN_TMP_DIR("testXXXXXX");

        int fd = mkostemp(template, O_CLOEXEC);

        if (fd < 0) return 1;
        if (stat(template, &statbuf)) return 2; // file should exist
        if (strncmp("test", template + strlen(TMP_DIR), 4)) return 3;
        if (!(fcntl(fd, F_GETFD) & FD_CLOEXEC)) return 4; // check flag

        unlink(template); // unlink to ensure delete on close
        close(fd);

        if (!stat(template, &statbuf)) return 5; // file should be gone
    }

    return 0;
}

int test_mkstemps() {
    struct stat statbuf;

    for (int i = 0; i < NUMITERS; i++) {
        char template[] = IN_TMP_DIR("testXXXXXXsuffix");

        int fd = mkstemps(template, 6);

        if (fd < 0) return 1;
        if (stat(template, &statbuf)) return 2; // file should exist
        if (strncmp("test", template + strlen(TMP_DIR), 4)) return 3;
        if (strncmp("suffix", template + strlen(TMP_DIR) + 10, 6)) return 4;

        unlink(template); // unlink to ensure delete on close
        close(fd);

        if (!stat(template, &statbuf)) return 5; // file should be gone
    }

    return 0;
}

int test_mkostemps() {
    struct stat statbuf;

    for (int i = 0; i < NUMITERS; i++) {
        char template[] = IN_TMP_DIR("testXXXXXXsuffix");

        int fd = mkostemps(template, 6, O_CLOEXEC);

        if (fd < 0) return 1;
        if (stat(template, &statbuf)) return 2; // file should exist
        if (strncmp("test", template + strlen(TMP_DIR), 4)) return 3;
        if (!(fcntl(fd, F_GETFD) & FD_CLOEXEC)) return 4; // check flag
        if (strncmp("suffix", template + strlen(TMP_DIR) + 10, 6)) return 5;

        unlink(template); // unlink to ensure delete on close
        close(fd);

        if (!stat(template, &statbuf)) return 6; // file should be gone
    }

    return 0;
}

int test_mkdtemp() {
    struct stat statbuf;

    for (int i = 0; i < NUMITERS; i++) {
        char template[] = IN_TMP_DIR("testXXXXXX");
        char *dirname;

        dirname = mkdtemp(template);

        if (stat(dirname, &statbuf) || !S_ISDIR(statbuf.st_mode)) return 1;
        if (strncmp("test", dirname + strlen(TMP_DIR), 4)) return 2;

        rmdir(dirname);

        if (!stat(dirname, &statbuf)) return 3;
    }

    return 0;
}