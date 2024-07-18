#include <wchar.h>

int main(void) {
  if (wcscasecmp(L"isithere?", L"NO") == 0) return 1;
  if (wcscasecmp(L"blahBLAHblah", L"BLAHblahBLAH") != 0) return 2;
}
