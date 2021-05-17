#include <wchar.h>

int main() {
  wchar_t a[30];
  wmemset(a, L'a', sizeof(a) / sizeof(wchar_t) - 1);
  a[sizeof(a) / sizeof(wchar_t) - 1] = 0;
  if (__builtin_cheri_tag_get(a) != 1) return 1;
  if (__builtin_cheri_length_get(a) != sizeof(a)) return 2;
  if (wcscmp(a, L"aaaaaaaaaaaaaaaaaaaaaaaaaaaaa")) return 3;
  return 0;
}
