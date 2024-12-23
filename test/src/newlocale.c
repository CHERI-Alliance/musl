#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#define test_success 0
#define bad_value 1
#define null_capability 2
#define capability_tag_cleared 3
#define incorrect_permission 4
#define incorrect_bound 5
#define bad_test_number 10

static void test_ptr_size(void** ptr, int is_read_only)
{
    if(*ptr == NULL){
        exit(null_capability);
    }
    if(!__builtin_cheri_tag_get(*ptr)){
        exit(capability_tag_cleared);
    }
    unsigned long perm = __builtin_cheri_perms_get(*ptr);
#ifdef __riscv_zcheripurecap
    unsigned long perm_primitive_type = __CHERI_CAP_PERMISSION_READ__ | __CHERI_CAP_PERMISSION_WRITE__;
    unsigned long perm_capability_type = __CHERI_CAP_PERMISSION_CAPABILITY__;
#else
    unsigned long perm_primitive_type = __CHERI_CAP_PERMISSION_PERMIT_LOAD__ | __CHERI_CAP_PERMISSION_PERMIT_STORE__;
    unsigned long perm_capability_type = __CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__ | __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__;
#endif
    unsigned long minimal_perm = perm_primitive_type | perm_capability_type;

#ifdef __riscv_zcheripurecap
    if (is_read_only) minimal_perm &= !(__CHERI_CAP_PERMISSION_WRITE__ | __CHERI_CAP_PERMISSION_CAPABILITY__);
#else
    if (is_read_only) minimal_perm &= !(__CHERI_CAP_PERMISSION_PERMIT_STORE__ | __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__);
#endif

    if((perm & minimal_perm) != minimal_perm){
#ifdef __riscv_zcheripurecap
        if(!(perm & __CHERI_CAP_PERMISSION_READ__)) {
            printf("ptr is missing load\n");
        }
        if(!(perm & __CHERI_CAP_PERMISSION_WRITE__)) {
            printf("ptr is missing store\n");
        }
        if(!(perm & __CHERI_CAP_PERMISSION_CAPABILITY__)) {
            printf("ptr is missing capability load\n");
        }
#else
        if(!(perm & __CHERI_CAP_PERMISSION_PERMIT_LOAD__)) {
            printf("ptr is missing load\n");
        }
        if(!(perm & __CHERI_CAP_PERMISSION_PERMIT_STORE__)) {
            printf("ptr is missing store\n");
        }
        if(!(perm & __CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__)) {
            printf("ptr is missing capability load\n");
        }
        if(!(perm & __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__)) {
            printf("ptr is missing capability store\n");
        }
#endif
        exit(incorrect_permission);
    }
}

int main(void)
{
    locale_t loc;

    loc = newlocale(LC_NUMERIC_MASK, "en_GB.utf8", (locale_t) 0);
    test_ptr_size((void**)(&loc),0);
    if (loc == (locale_t) 0)
        return null_capability;

    /* Apply the newly created locale to this thread. */
    uselocale(loc);

    /* Test effect of LC_NUMERIC. */
    printf("%8.3f\n", 123456.789);

    loc = newlocale(LC_ALL_MASK, "C", 0);
    test_ptr_size((void**)(&loc),1);
    if (loc == (locale_t) 0)
        return null_capability;

    uselocale(loc);

    printf("%8.3f\n", 123456.789);

    uselocale(LC_GLOBAL_LOCALE);    /* So 'loc' is no longer in use */
    freelocale(loc);    /* Free the locale object. */

    return test_success;
}
