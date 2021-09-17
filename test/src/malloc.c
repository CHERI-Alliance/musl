#include <stdlib.h>
#include <stdio.h>

#define test_success 0
#define unexpected_value 1
#define null_capability 2
#define capability_tag_cleared 3
#define incorrect_permission 4
#define incorrect_bound 5
#define bad_alignment 6
#define bad_morello_alignment 7
#define bad_test_number 10

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

    int key_map_offset = sizeof(void*);
    size_t morello_size = __builtin_cheri_round_representable_length(alloc_size + key_map_offset);
    if(alloc_size != -1 && __builtin_cheri_length_get(ptr) != morello_size){
        exit(incorrect_bound);
    }
}

void testptr(void* ptr){
    testptr_size(ptr,-1);
}

#define max_malloc 300 //min 33. Mainly used to size test #3

int main(int argc, char **argv) {
    void* all_ptr[max_malloc];

    switch (argv[1][0]) {
    case '0'://basic test to run malloc and check the capability validity
        // test allocating seveal slot of the same size
        all_ptr[0] = malloc(150);
        all_ptr[1] = malloc(150);
        testptr_size(all_ptr[0],150);
        testptr(all_ptr[1]);

        // test the limit of a slot's size
        all_ptr[2] = malloc(511);
        testptr(all_ptr[2]);

        // test a dedicated mmap
        all_ptr[3] = malloc(32768);
        testptr(all_ptr[3]);

        break;
    case '1'://basic test for free
        all_ptr[0] = malloc(150);
        //printf("malloc return : %p\n",all_ptr[0]);
        free(all_ptr[0]);
        all_ptr[1] = malloc(16*32768);
        //printf("malloc return : %p\n",all_ptr[0]);
        free(all_ptr[1]);
        break;

    case '2':// tries to fill a few group, make some space, refill. In case there is some corruption of the metadata on free
        for (int i = 0; i < max_malloc; i++){
            all_ptr[i] = malloc(63);
        }
        for (int i = 0; i < max_malloc; i+=2){
            free(all_ptr[i]);
        }
        for (int i = 0; i < max_malloc; i+=2){
            all_ptr[i] = malloc(63);
        }
        break;
    case '3':// Check that the data don't overlaps with the metadata or some other data
        // allocate all slots
        for (int i = 0; i < 33; i++){
            all_ptr[i] = malloc(63);
        }
        // fill them with two set of data, in case some corruption match one set
        for (int i = 0; i < 33; i++){
            for (int j = 0; j < 63; j++){
                ((char *)all_ptr[i])[j] = j%2 ? j : -j;
            }
        }
        // check there is no overlap
        for (int i = 0; i < 33; i++){
            for (int j = 0; j < 63; j++){
                char val = ((char *)all_ptr[i])[j];
                char expected = (j%2 ? j : -j); //I have to put this value in a variable. Making a direct comparison fails.
                if (val != expected){
                    printf("Some byte seems corrupted. Value : [%hhd, %#02X], j : %d, expected : %d\n", val, val, j, expected);
                    return unexpected_value;
                }
            }
        }
        for (int i = 0; i < 33; i+=3){
            free(all_ptr[i]);
        }
        // check again the sets, in case free overwrote some neighbour slots
        for (int i = 0; i < 33; i++){
            if (i%3==0)
                continue;
            for (int j = 0; j < 63; j++){
                char val = ((char *)all_ptr[i])[j];
                char expected = (j%2 ? j : -j);
                if (val != expected){
                    printf("Some byte seems corrupted after some free(). Value : [%hhd, %#02X], j : %d, expected : %d\n", val, val, j, expected);
                    return unexpected_value;
                }
            }
        }
        break;
    case '4'://segfault on use after free
        all_ptr[0] = malloc(150);
        free(all_ptr[0]);
        *(int*)(all_ptr[0]) = 42;

        break;
    case '5'://segfault on double free
        all_ptr[0] = malloc(16*32768);
        free(all_ptr[0]);
        free(all_ptr[0]);

        break;
        //TODO needs some tests on the narrow bounds of malloc. Ideally, we want the user to only be able to access the slot they requested,
        //and not be able to mess with the metadata, not with other adjacent slots. Currently this narrow bound mechanism is not implemented
        //because free need the wide capability to work. Some mechanism have to be added to find the wide capability from the narrowed one.
    default:
        return bad_test_number;
    }
    return test_success;
}
