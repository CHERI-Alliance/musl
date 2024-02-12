#ifdef __riscv_float_abi_soft
typedef __uintcap_t __jmp_buf[14];
#else
typedef __uintcap_t __jmp_buf[20];
#endif
