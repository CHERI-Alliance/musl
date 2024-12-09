#ifndef _SYS_CHERI_H
#define _SYS_CHERI_H

#if defined(__riscv_zcheripurecap)
#ifdef __CHERI_BW_CAP_PERMISSION_CAPABILITY__
#define CHERI_PERM_SW_VMEM    (1U << 16)
#else
#define CHERI_PERM_SW_VMEM    (1U << 6)
#endif
#else
#define CHERI_PERM_SW_VMEM    (1 << 2) /* User[0] permission */
#endif

#endif // _SYS_CHERI_H
