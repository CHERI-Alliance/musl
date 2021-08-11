#define _GNU_SOURCE

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#define TMP_DIR "/tmp/morello-musl-tests-open/"
#define IN_TMP_DIR(f) TMP_DIR f

mode_t mode = 0666;

int test_open() {
  // set up root dir in /tmp
  if (mkdir(TMP_DIR, 0777) && errno != EEXIST) return -2;

  // open temp file
  int fd = open(".", O_TMPFILE | O_RDWR | O_EXCL, mode);
  if (fd < 0) return 1;
  if (close(fd)) return -3;

  // open our own exe (read only)
  fd = open("/proc/self/exe", O_RDONLY, mode);
  if (fd < 0) return 2;
  if (close(fd)) return -3;

  // generate name for file in /tmp/...
  char template[] = IN_TMP_DIR("openXXXXXX");
  char *filename;
  filename = mktemp(template);

  // create file with generated name
  fd = open(filename, O_CREAT, mode);
  if (fd < 0) return 3;
  if (close(fd)) return -3;

  // try to create same file with O_EXCL
  fd = open(filename, O_CREAT | O_EXCL, mode);
  if (fd >= 0) return 4;

  // open same file again, and unlink to delete
  fd = open(filename, O_RDWR, mode);
  if (unlink(filename)) return -4;
  if (fd < 0) return 5;
  if (close(fd)) return -3;

  return 0;
}

int test_openat() {
  int dirfd = open("/proc/self/", O_DIRECTORY);

  // try to open ourself (read-only)
  int fd = openat(dirfd, "exe", O_RDONLY);
  if (fd < 0) return 1;
  if (close(fd)) return -3;

  return 0;
}

int test_creat() {
  // set up root dir in /tmp
  if (mkdir(TMP_DIR, 0777) && errno != EEXIST) return -2;

  // generate name for file in /tmp/...
  char template[] = IN_TMP_DIR("openXXXXXX");
  char *filename;
  filename = mktemp(template);

  int fd = creat(filename, mode);

  if (unlink(filename)) return -4;
  if (fd < 0) return 1;
  if (close(fd)) return -3;

  return 0;
}

int main(int argc, char **argv) {
  umask(0);

  switch (argv[1][0]) {
    case '0': // open
      return test_open();
    case '1': // openat
      return test_openat();
    case '2': // creat
      return test_creat();
  }

  return -1;
}
