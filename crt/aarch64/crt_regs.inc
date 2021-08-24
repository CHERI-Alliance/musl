#ifdef MORELLO

#define r_FP c29
#define r_LR c30
#define r_SP csp
#define INC 32

#else

#define r_FP x29
#define r_LR x30
#define r_SP sp
#define INC 16

#endif
