#define FUN(fn) \
    .global fn; \
    .balign 16; \
    .type fn, %function; \
    fn

#define END(fn) \
    .size fn, .-fn;

#ifdef MORELLO
FUN(__expand_ddc):
    CVTD c0, x0
    ret
END(__expand_ddc)
#else
FUN(__expand_ddc):
    ret
END(__expand_ddc)
#endif
