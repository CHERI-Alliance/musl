#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define test_success 0
#define unexpected_value 1
#define null_capability 2
#define capability_tag_cleared 3
#define incorrect_permission 4
#define incorrect_bound 5
#define bad_alignment 6
#define bad_test_number 10

void testptr_size(void* ptr,size_t min_size){
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

    if(__builtin_cheri_length_get(ptr) < min_size){
        exit(incorrect_bound);
    }
}

int main(int argc, char **argv) {

    size_t size = 10*sizeof(int);
    int * array = (int*) calloc(sizeof(int), 10);

    // first make sure calloc worked properly
    testptr_size((void*)array,size);
    for(int i = 0; i < 10; i++) {
        if (array[i] != 0) {
            return unexpected_value;
        }
        array[i] = i;
    }

    array = reallocarray(array,sizeof(int), 30);

    testptr_size((void*)array,size*3);
    for(int i = 0; i < 10; i++) {
        if (array[i] != i) {
            return unexpected_value;
        }
    }
    return test_success;
}
