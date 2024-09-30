#ifndef _SYS_CHERI_H
#define _SYS_CHERI_H

#if defined(__riscv_zcheripurecap)
/* FIXCHERI: Legacy pro 0.9 encoding. */
#define CHERI_PERM_SW_VMEM    (1U << 16)
#else
#define CHERI_PERM_SW_VMEM    (1 << 2) /* User[0] permission */
#endif

#endif // _SYS_CHERI_H
