#include <string.h>

int main() {
  char *words[] = {"where", "daffodil", "and", "lily", "wave"};
  size_t ns[] = {1, 4, 2, 4, 4};

  char *exp = "wdaffanlilywave";

  char str[strlen(exp)];
  char* dst = str;
  for (int i = 0; i < 5; i++) {
    dst = stpncpy(dst, words[i], ns[i]);
    for (int j = (dst - str) + ns[i]; j < strlen(exp) + 1; j++) {
      if (str[j] != 0) return 1;
    }
  }

  if (__builtin_cheri_tag_get(dst) != 1) return 2;
  if (__builtin_cheri_length_get(dst) != strlen(exp) + 1) return 3;
  if (strcmp(str, exp)) return 4;

  return 0;
}
