#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>

#define test_success 0
#define unexpected_value 1
#define null_capability 2
#define capability_tag_cleared 3
#define incorrect_permission 4
#define incorrect_bound 5
#define bad_alignment 6
#define bad_morello_alignment 7
#define bad_test_number 10

int group_map_not_cleaned_on_create()
{
    int **list = NULL;
    int len = 0;
    for (int cnt = 0; cnt < 64; cnt++) {
        if (cnt >= len) {
            len = (len * 2) + 1;
            list = realloc(list, len * sizeof(int *));
        }
        list[cnt] = malloc(sizeof(int));
        *list[cnt] = cnt;
    }
    return test_success;
}

int unmap_move_corrupt_nested_group()
{
    struct dirent **namelist = NULL;

    int n = scandir("/proc/self", &namelist, NULL, alphasort);
    printf("%d\n", n);
    return test_success;
}

int main(int argc, char **argv) {

    switch (argv[1][0]) {
    case '0':
        return group_map_not_cleaned_on_create();
    break;
    case '1':
        return unmap_move_corrupt_nested_group();
    break;
    default:
        return bad_test_number;
    }
    return test_success;
}
