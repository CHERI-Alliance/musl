#include <stdlib.h>
#include <stdio.h>

int main(void) {
  char* val = getenv("HELLO");
  printf("%s\n", val);
  printf("%lu\n", __builtin_cheri_length_get(val));
  return 0;
}
