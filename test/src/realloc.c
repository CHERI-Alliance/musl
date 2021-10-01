#include <stdlib.h>
#include <stdio.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

// To check for corruption, we fill the start and end of alloc-ed chunk with some data.
// This define how many byte at maximum we fill on each end. This is so big malloc don't
// take tens of seconds to run during the test. If there is enough space to put two
// capabilities, then fill will add them regardless if it is more than this maximum
#define FILL_THICKNESS 128

#define test_success 0
#define unexpected_value 1
#define null_capability 2
#define capability_tag_cleared 3
#define incorrect_permission 4
#define incorrect_bound 5
#define bad_alignment 6
#define bad_morello_alignment 7
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
    if(__builtin_cheri_length_get(ptr) != morello_size){
        printf("expected length: %zu (%zu) actual length: %zu\n", morello_size, alloc_size, __builtin_cheri_length_get(ptr));
        exit(incorrect_bound);
    }
}

void fill(char* ptr, size_t size)
{
    // fill with "random" data
    int effective_thickness = MIN(size/2,FILL_THICKNESS);
    for(int i = 0; i < effective_thickness; i++){
        ptr[i] = (i ? i : -i) & 255;
    }
    for(int i = size-1; i >= size-1-effective_thickness; i--){
        ptr[i] = (i ? i : -i) & 255;
    }

    // add a capability at the start and at the end
    if(size >= 48){
        size_t offset = ((__uintcap_t) ptr % 16) ? 16 - ((__uintcap_t) ptr % 16): 0;
        void** first_slot = (void**) (ptr + offset);
        void** last_slot = (void**) (ptr + ((size - 16) & ~15));

        // be aware that with a realloc, those capability might be pointing to the old memory.
        // The only thing we can reliably check is the validity tag. Accessing them can segfault
        *first_slot = first_slot;
        *last_slot = last_slot;
    }
}

void check_fill(char* ptr, size_t size)
{
    void** first_slot;
    void** last_slot;
    int effective_thickness = MIN(size/2,FILL_THICKNESS);

    // check the capability at the start and at the end
    if(size >= 48){
        size_t offset = ((__uintcap_t) ptr % 16) ? 16 - ((__uintcap_t) ptr % 16): 0;
        first_slot = (void**) (ptr + offset);
        last_slot = (void**) (ptr + ((size - 16) & ~15));

        // be aware that with a realloc, those capability might be pointing to the old memory.
        // The only thing we can reliably check is the validity tag. Accessing them can segfault
        if(!__builtin_cheri_tag_get(*first_slot) || !__builtin_cheri_tag_get(*last_slot)){
            printf("tag first slot : %d\ntag last slot : %d\n",__builtin_cheri_tag_get(*first_slot),__builtin_cheri_tag_get(*last_slot));
            exit(capability_tag_cleared);
        }
    }

    // check the "random" data
    for(int i = 0; i < effective_thickness; i++){
        if (ptr[i] != ((i ? i : -i) & 255)){
            // the two capability are legit missmatch, don't report them
            if ( size>=48 && (ptr+i) >= (char*)first_slot && (ptr+i) < ((char*)first_slot+16) )
                continue;
            exit(unexpected_value);
        }
    }
    for(int i = size-1; i >= size-1-effective_thickness; i--){
        if (ptr[i] != ((i ? i : -i) & 255)){
            // the two capability are legit missmatch, don't report them
            if ( size>=48 && (ptr+i) >= (char*)last_slot && (ptr+i) < ((char*)last_slot+16) )
                continue;
            exit(unexpected_value);
        }
    }
}

void* basic_test(size_t size1, size_t size2)
{
    size_t smaller_size = size1 < size2 ? size1 : size2;
    void* ptr = malloc(size1);
    fill(ptr,smaller_size);
    ptr = realloc(ptr,size2);
    testptr_size(ptr,size2);
    check_fill(ptr,smaller_size);
    return ptr;
}

void test_fit_in_slot()
{
    void* all_ptr[2];
    all_ptr[0] = basic_test(175*16, 180*16);
    all_ptr[1] = basic_test(16384*16+1, 16384*16+500);

    free(all_ptr[0]);
    free(all_ptr[1]);
}

void test_increase_size()
{
    void* all_ptr[2];
    all_ptr[0] = basic_test(175*16, 300*16);

    all_ptr[1] = basic_test(16384*16+1, 16384*16*2);

    free(all_ptr[0]);
    free(all_ptr[1]);
}

void test_reduce_size()
{
    void* all_ptr[3];
    // just one size class
    all_ptr[0] = basic_test(175*16, 150*16);

    // several size classes
    all_ptr[1] = basic_test(175*16, 40*16);

    all_ptr[2] = basic_test(16384*16+1, 16384*16+500);

    free(all_ptr[0]);
    free(all_ptr[1]);
    free(all_ptr[2]);
}

void test_change_regime()
{
    void* all_ptr[2];
    all_ptr[0] = basic_test(175*16, 16384*16+1);

    all_ptr[1] = basic_test(16384*16+1, 175*16);

    free(all_ptr[0]);
    free(all_ptr[1]);
}

int main(int argc, char **argv)
{
    switch (argv[1][0]) {
    case '0': //realloc, but actually fits within the already allocated slot
        test_fit_in_slot();
        break;
    case '1': // realloc, but bigger
        test_increase_size();
        break;
    case '2': // realloc, but smaller
        test_reduce_size();
        break;
    case '3': // realloc, but change of regime (sizeclass <-> big alloc)
        test_change_regime();
        break;
    default:
        return bad_test_number;
    }
    return test_success;
}
