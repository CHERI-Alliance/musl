#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>

#define test_success 0
#define copied_value_not_equal 1
#define null_capability 2
#define capability_tag_cleared 3
#define bad_alignment 6

int main(int argc, char **argv)
{

    int dummy_var = 0;
    int * ptr = &dummy_var;

    // use mmap to allocate some int* array
    int **p = (int **) mmap(0, 3072UL, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);

    // check that p is aligned to some 16B boundary so it can hold a capability
    if ((__intcap_t)p & 0xF) {
        return bad_alignment;
    }

    p[0] = ptr;
    *(p[0]) += 1;

    int **new_p = (int **) mremap(p, 3072UL, 16384UL, MREMAP_MAYMOVE);

    if (!__builtin_cheri_tag_get(new_p[0])) {
        return capability_tag_cleared;
    }

    if (new_p[0] != ptr ) {
        return copied_value_not_equal;
    }

    if (*new_p[0] == 1) {
        *(new_p[0]) += 1;
    }

    if (*new_p[0] == 2) {
        printf("mremap copied the capability correctly\n");
    } else {
        printf("Something weird went wrong when mremap copied the capability\n");
        // in fact, I don't even know how we can reach this code, but with a bit of luck
        // this will avoid some code to be optimised out.
        return copied_value_not_equal;
    }

    return test_success;
}
