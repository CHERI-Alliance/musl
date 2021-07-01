#define _GNU_SOURCE

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

int test_open() {
  // open temp file
  int fd = open(".", O_TMPFILE | O_RDWR | O_EXCL, mode);
  if (fd < 0) {
    return 2;
  }

  // open our own exe (read only)
  int fd2 = open("/proc/self/exe", O_RDONLY, mode);
  if (fd2 < 0) {
    return 3;
  }

  if (close(fd) || close(fd2)) {
    return 1;
  }

  // TODO: pending mktemp(), use open() to create named file,
  // and then try to create again with O_EXCL and check it fails

  return 0;
}

int test_openat() {
  int dirfd = open("/proc/self/", O_DIRECTORY);

  // try to open ourself (read-only)
  int fd = openat(dirfd, "exe", O_RDONLY);
  if (fd < 0) {
    return 4;
  }

  if (close(fd)) {
    return 1;
  }

  return 0;
}


// TODO: pending mktemp()

// creat() is just a wrapper around open()
// int test_creat() {
//   int fd = creat("tmpfile", mode);
//   if (fd < 0) {
//     return 5;
//   }
//   return 0;
// }

int main(int argc, char **argv) {
  switch (argv[1][0]) {
    case '0': // open
      return test_open();
    case '1': // openat
      return test_openat();
    // case '2': // creat
    //   return test_creat();
  }

  return -1;
}