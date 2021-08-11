#include <sys/auxv.h>

int main (int argc, char *argv[]) {
    char *platform = (char *) getauxptr(AT_PLATFORM);
    unsigned long perms = __builtin_cheri_perms_get(platform);

    if (!__builtin_cheri_tag_get(platform)) return 1;
    if (!(perms & __CHERI_CAP_PERMISSION_PERMIT_LOAD__)) return 2;

    return 0;
}
