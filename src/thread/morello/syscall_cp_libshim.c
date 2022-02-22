#ifdef LIBSHIM

// Special syscall_cp implementation for when libshim is enabled.
// This calls the syscall() function instead of a direct svc instr.
// It also sets a in_syscall_cp boolean value in the current pthread's
//  struct to 1 to indicate that we are inside the syscall_cp function.
//  This is required as the invocation of syscall() will cause us to
//  jump outside the range [__cp_begin, __cp_end], and musl inspects
//  the PC to check if we are in this range - this boolean is used as
//  a replacement for this check when libshim is enabled.

#include "syscall.h"
#include "pthread_impl.h"

intptr_t __syscall_cp_asm(volatile void *cp, syscall_arg_t nr,
                      syscall_arg_t u, syscall_arg_t v, syscall_arg_t w,
                      syscall_arg_t x, syscall_arg_t y, syscall_arg_t z) {
    volatile char* cancel = cp;
    if (*cancel) {
        goto __cp_cancel;
    }

    pthread_t self = __pthread_self();
    self->in_syscall_cp = 1;

    __asm__ volatile(
        ".global __cp_begin\n" \
        ".hidden __cp_begin\n" \
        ".size __cp_begin, 1\n" \
        "__cp_begin:":
    );
    intptr_t ret = syscall(nr, u, v, w, x, y, z);

    __asm__ volatile(
        ".global __cp_end\n" \
        ".hidden __cp_end\n" \
        ".size __cp_end, 1\n" \
        "__cp_end:":
    );
    self->in_syscall_cp = 0;

    return ret;

__cp_cancel:
    __asm__ volatile(
        ".global __cp_cancel\n" \
        ".hidden __cp_cancel\n" \
        ".size __cp_cancel, 1\n" \
        "__cp_cancel:\n" \
        "b __cancel\n":
    );
}

#endif