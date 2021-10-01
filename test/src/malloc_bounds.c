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
#define bad_morello_alignment 7
#define bad_test_number 10

#include "caplength.h"

void testptr_size(void* ptr,size_t alloc_size, size_t alignement){
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
    if(alignement && ((uintcap_t)ptr & (alignement - 1)) != 0) {
        printf("bad alignment : %#p\nshould be aligned to %zu\n", ptr, alignement);
        exit(bad_alignment);
    }
}

// Those test don't care about the data corruption,
// they just do a basic validation of the returned capability
int main(int argc, char **argv) {
    unsigned int small_size = 150;
    unsigned int big_size = 16*32768;
    // todo: something feels wrong with these alignment requirements -- needs cheecking
    size_t morello_alignment_small = ~__builtin_cheri_representable_alignment_mask(small_size) + 1;
    size_t morello_alignment_big = ~__builtin_cheri_representable_alignment_mask(big_size) + 1;
    unsigned int MAP_KEY_OFFSET = 0; // mirror the value in meta.h
    void* p;
    switch (argv[1][0]) {
    case '0': // malloc_bounds_small
        p = malloc(small_size);
        testptr_size(p,small_size,0);
        if(((uintcap_t)p-MAP_KEY_OFFSET) % morello_alignment_small != 0) {
            printf("bad alignment : %#p\nshould be aligned to %zu + 16\n", p, morello_alignment_small);
            exit(bad_morello_alignment);
        }
        free(p);
        break;
    case '1': // malloc_bounds_large
        p = malloc(big_size);
        testptr_size(p,big_size,0);
//        if(((uintcap_t)p-MAP_KEY_OFFSET) % morello_alignment_big != 0) {
//            printf("bad alignment : %#p\nshould be aligned to %zu + 16\n", p, morello_alignment_big);
//            exit(bad_morello_alignment);
//        }
        free(p);
        break;
    case '2': // aligned_malloc_bounds_small
        p = aligned_alloc(64,small_size);
        testptr_size(p,small_size,64);
        if(((uintcap_t)p) % morello_alignment_small != 0) {
            printf("bad alignment : %#p\nshould be aligned to %zu\n", p, morello_alignment_small);
            exit(bad_morello_alignment);
        }
        free(p);
        break;
    case '3': // aligned_malloc_bounds_large
        p = aligned_alloc(morello_alignment_big,big_size);
        testptr_size(p,big_size,morello_alignment_big);
        if(((uintcap_t)p) % morello_alignment_big != 0) {
            printf("bad alignment : %#p\nshould be aligned to %zu\n", p, morello_alignment_big);
            exit(bad_morello_alignment);
        }
        free(p);
        break;
    case '4': // realloc_bounds_small
        p = malloc(small_size);
        p = realloc(p,small_size+1);
        testptr_size(p,small_size+1,0);
        if(((uintcap_t)p-MAP_KEY_OFFSET) % morello_alignment_small != 0) {
            printf("bad alignment : %#p\nshould be aligned to %zu + 16\n", p, morello_alignment_small);
            exit(bad_morello_alignment);
        }
        break;
        free(p);
        break;
    case '5': // realloc_bounds_large
        p = malloc(big_size);
        p = realloc(p,big_size+65536+1);
        testptr_size(p,big_size+65536+1,0);
//        if(((uintcap_t)p-MAP_KEY_OFFSET) % morello_alignment_big != 0) {
//            printf("bad alignment : %#p\nshould be aligned to %zu + 16\n", p, morello_alignment_big);
//            exit(bad_morello_alignment);
//        }
        free(p);
        break;
    case '6': // realloc_bounds_change_regime
        p = malloc(big_size);
        p = realloc(p,small_size);
        testptr_size(p,small_size,0);
        if(((uintcap_t)p-MAP_KEY_OFFSET) % morello_alignment_small != 0) {
            printf("bad alignment : %#p\nshould be aligned to %zu + 16\n", p, morello_alignment_small);
            exit(bad_morello_alignment);
        }
        free(p);
        break;
    default:
        return bad_test_number;
    }
    return test_success;
}
