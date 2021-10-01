#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define test_success 0
#define unexpected_value 1
#define null_capability 2
#define capability_tag_cleared 3
#define incorrect_permission 4
#define incorrect_bound 5
#define bad_alignment 6
#define bad_test_number 10

#include "caplength.h"

void testptr_size(void* ptr, size_t alloc_size)
{
    if(ptr == NULL){
        exit(null_capability);
    }
    if(!__builtin_cheri_tag_get(ptr)){
        exit(capability_tag_cleared);
    }
    unsigned long perm = __builtin_cheri_perms_get(ptr);
    unsigned long minimal_perm =
        __CHERI_CAP_PERMISSION_PERMIT_LOAD__ |
        __CHERI_CAP_PERMISSION_PERMIT_STORE__ |
        __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ |
        __CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__;

    if((perm & minimal_perm) != minimal_perm){
        exit(incorrect_permission);
    }

    int key_map_offset = 0;
    size_t morello_size = WOULD_BE_LENGTH(alloc_size + key_map_offset, ptr);
    if(__builtin_cheri_length_get(ptr) != morello_size) {
        printf("expected length: %zu (%zu) actual length: %zu\n", morello_size, alloc_size, __builtin_cheri_length_get(ptr));
        exit(incorrect_bound);
    }
}

int main(int argc, char **argv) {
    size_t alignment = 64 * 4; // must be a power of 2
    void * aligned_memory = aligned_alloc(alignment, alignment);
    testptr_size(aligned_memory, alignment);
    if (((__intcap_t)aligned_memory & (alignment - 1)) != 0) {
        printf("expected alignment: %zu, actual alignment: %zu\n", alignment, (size_t)((uintcap_t)aligned_memory & (alignment - 1)));
        return bad_alignment;
    }
    return test_success;
}
