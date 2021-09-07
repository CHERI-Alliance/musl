#include <stdlib.h>
#include <stdio.h>

#define test_success 0
#define unexpected_value 1
#define null_capability 2
#define capability_tag_cleared 3
#define incorrect_permission 4
#define incorrect_bound 5
#define bad_alignment 6
#define bad_test_number 10

void testptr_size(void** ptr,long long min_size){
    if(*ptr == NULL){
        exit(null_capability);
    }
    if(!__builtin_cheri_tag_get(*ptr)){
        exit(capability_tag_cleared);
    }
    unsigned long perm = __builtin_cheri_perms_get(*ptr);
    unsigned long minimal_perm =
        __CHERI_CAP_PERMISSION_PERMIT_LOAD__ |
        __CHERI_CAP_PERMISSION_PERMIT_STORE__ |
        __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ |
        __CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__;

    if((perm & minimal_perm) != minimal_perm){
        exit(incorrect_permission);
    }

    if(__builtin_cheri_length_get(*ptr) < min_size){
        exit(incorrect_bound);
    }
}

int main(int argc, char **argv) {
    size_t size = 64;
    void * alligned_memory = valloc(size);
    testptr_size(&alligned_memory,size);
    if (((__intcap_t)alligned_memory % 4096) != 0) {
        return bad_alignment;
    }
    //TODO: for now valloc return a wide bound that spans more than the alloc-ed memory
    // eventually, we want to test that the bounds returned are narrowed both when the requested
    // length is representable and when the requested length is not representable
    return test_success;
}
