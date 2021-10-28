#pragma once

#define TEST_SUCCESS 0
#define UNEXPECTED_VALUE 1
#define NULL_CAPABILITY 2
#define CAPABILITY_TAG_CLEARED 3
#define INCORRECT_PERMISSION 4
#define INCORRECT_BOUND 5
#define BAD_ALIGNMENT 6
#define BAD_MORELLO_ALIGNMENT 7
#define BAD_TEST_NUMBER 10

// TODO: make sure these are the minimal permissions we want
#define MINIMAL_PERMS (__CHERI_CAP_PERMISSION_PERMIT_LOAD__ | \
        			   __CHERI_CAP_PERMISSION_PERMIT_STORE__ | \
        		       __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ | \
        			   __CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__)

inline static void testptr_aligned(void* ptr, size_t alloc_size, size_t alignment) {
    if (!ptr) {
        printf("user pointer is null!\n");
        exit(NULL_CAPABILITY);
    }

    if (!__builtin_cheri_tag_get(ptr)) {
        printf("tag is unset for ptr: %p\n", ptr);
        exit(CAPABILITY_TAG_CLEARED);
    }

    unsigned long perm = __builtin_cheri_perms_get(ptr);

    if ((perm & MINIMAL_PERMS) != MINIMAL_PERMS) {
        printf("invalid permissions on user pointer. ptr: %p, perms: %lx\n", ptr, perm);
        exit(INCORRECT_PERMISSION);
    }

    // This gives us the best representable length for the bounds, assuming the pointer is aligned
    //   as required by Morello.
    size_t morello_size = __builtin_cheri_round_representable_length(alloc_size);

    // This gives us the alignment required to represent the above length correctly.
    size_t morello_alignment_mask = __builtin_cheri_representable_alignment_mask(morello_size);

    if (((__uintcap_t) ptr & morello_alignment_mask) != (__uintcap_t) ptr) {
        printf("expected a morello alignment: %zu, actual address: %p\n", ~morello_alignment_mask + 1, ptr);
        exit(BAD_MORELLO_ALIGNMENT);
    }

    if(__builtin_cheri_length_get(ptr) != morello_size) {
        printf("expected length: %zu (requested: %zu), actual length: %zu\n", morello_size, alloc_size, __builtin_cheri_length_get(ptr));
        exit(INCORRECT_BOUND);
    }

    if (alignment && ((__uintcap_t)ptr & (alignment - 1)) != 0) {
        printf("bad aligned_alloc alignment : %#p\nshould be aligned to %zu\n", ptr, alignment);
        exit(BAD_ALIGNMENT);
    }
}

inline static void testptr(void *ptr, size_t alloc_size) {
	testptr_aligned(ptr, alloc_size, 0);
}
