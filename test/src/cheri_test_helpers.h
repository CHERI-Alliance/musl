#pragma once

#include <stdint.h>

#if defined(__riscv_zcheripurecap)
#define __CHERI_CAP_PERMISSION_USER0__	(1u << 6u)
#define __CHERI_CAP_PERMISSION_USER1__	(1u << 7u)
#define __CHERI_CAP_PERMISSION_USER2__	(1u << 8u)
#define __CHERI_CAP_PERMISSION_USER3__	(1u << 9u)
#else
#define __CHERI_CAP_PERMISSION_USER0__	(1u << 2u)
#define __CHERI_CAP_PERMISSION_USER1__	(1u << 3u)
#define __CHERI_CAP_PERMISSION_USER2__	(1u << 4u)
#define __CHERI_CAP_PERMISSION_USER3__	(1u << 5u)
#endif
#define __CHERI_CAP_PERMISSION_VMEM__	__CHERI_CAP_PERMISSION_USER0__

static const unsigned long READ_CAP_PERMS =
#if defined(__riscv_zcheripurecap)
							__CHERI_CAP_PERMISSION_READ__ |
							__CHERI_CAP_PERMISSION_CAPABILITY__ |
							__CHERI_CAP_PERMISSION_LOAD_MUTABLE__;
#else
#ifdef __ARM_CAP_PERMISSION_MUTABLE_LOAD__
							__ARM_CAP_PERMISSION_MUTABLE_LOAD__ |
#endif
							__CHERI_CAP_PERMISSION_PERMIT_LOAD__ |
							__CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__;
#endif

static unsigned long WRITE_CAP_PERMS =
#if defined(__riscv_zcheripurecap)
#if defined(__riscv_zcherilevels)
							__CHERI_CAP_PERMISSION_STORE_LEVEL__ |
#endif
							__CHERI_CAP_PERMISSION_WRITE__ |
							__CHERI_CAP_PERMISSION_CAPABILITY__;
#else
							__CHERI_CAP_PERMISSION_PERMIT_STORE__ |
							__CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ |
							__CHERI_CAP_PERMISSION_PERMIT_STORE_LOCAL__;
#endif

static unsigned long EXEC_CAP_PERMS =
#if defined(__riscv_zcheripurecap)
							__CHERI_CAP_PERMISSION_EXECUTE__ |
							__CHERI_CAP_PERMISSION_ACCESS_SYSTEM_REGISTERS__;
#else
#ifdef __ARM_CAP_PERMISSION_EXECUTIVE__
							__ARM_CAP_PERMISSION_EXECUTIVE__ |
#endif
							__CHERI_CAP_PERMISSION_PERMIT_EXECUTE__ |
							__CHERI_CAP_PERMISSION_ACCESS_SYSTEM_REGISTERS__;
#endif

static unsigned long ROOT_CAP_PERMS =
#if !defined(__riscv_zcheripurecap)
							__CHERI_CAP_PERMISSION_GLOBAL__ |
#endif
							__CHERI_CAP_PERMISSION_VMEM__;

static unsigned long SEAL_CAP_PERMS =
#if defined(__riscv_zcheripurecap)
							0ul;
#else
							__CHERI_CAP_PERMISSION_GLOBAL__ |
							__CHERI_CAP_PERMISSION_PERMIT_SEAL__ |
							__CHERI_CAP_PERMISSION_PERMIT_UNSEAL__;
#endif

#define TEST(mask, var) (((mask) & (var)) != 0)

inline static const char *
__print_perms(uint64_t perms, char *__perms_buffer)
{
#if defined(__riscv_zcheripurecap)
  static uint64_t __permissions[] = {
    __CHERI_CAP_PERMISSION_CAPABILITY__,
    __CHERI_CAP_PERMISSION_WRITE__,
    __CHERI_CAP_PERMISSION_READ__,
    __CHERI_CAP_PERMISSION_EXECUTE__,
    __CHERI_CAP_PERMISSION_ACCESS_SYSTEM_REGISTERS__,
    __CHERI_CAP_PERMISSION_LOAD_MUTABLE__,
#if defined(__riscv_zcherilevels)
    __CHERI_CAP_PERMISSION_ELEVATE_LEVEL__,
    __CHERI_CAP_PERMISSION_STORE_LEVEL__,
    __CHERI_CAP_PERMISSION_CAPABILITY_LEVEL__,
#else
		0,0,0,
#endif
		__CHERI_CAP_PERMISSION_VMEM__,
		__CHERI_CAP_PERMISSION_USER1__,
		__CHERI_CAP_PERMISSION_USER2__,
		__CHERI_CAP_PERMISSION_USER3__,
  };
  static char __permission_names[] = {'c', 'w', 'r', 'x', 'S', 'M', 'e', 's', 'G', 'V', '1', '2', '3'};
#else
	static uint64_t __permissions[] = {
		__CHERI_CAP_PERMISSION_GLOBAL__,
		__CHERI_CAP_PERMISSION_PERMIT_LOAD__,
		__CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__,
#ifdef __ARM_CAP_PERMISSION_MUTABLE_LOAD__
		__ARM_CAP_PERMISSION_MUTABLE_LOAD__,
#else
		0,
#endif
		__CHERI_CAP_PERMISSION_PERMIT_STORE__,
		__CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__,
		__CHERI_CAP_PERMISSION_PERMIT_STORE_LOCAL__,
		__CHERI_CAP_PERMISSION_PERMIT_EXECUTE__,
#ifdef __ARM_CAP_PERMISSION_EXECUTIVE__
		__ARM_CAP_PERMISSION_EXECUTIVE__,
#else
		0,
#endif
		__CHERI_CAP_PERMISSION_ACCESS_SYSTEM_REGISTERS__,
		__CHERI_CAP_PERMISSION_PERMIT_SEAL__,
		__CHERI_CAP_PERMISSION_PERMIT_UNSEAL__,
#ifdef __ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR__
		__ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR__,
#else
		0,
#endif
#ifdef __ARM_CAP_PERMISSION_COMPARTMENT_ID__
		__ARM_CAP_PERMISSION_COMPARTMENT_ID__,
#else
		0,
#endif
		__CHERI_CAP_PERMISSION_VMEM__,
		__CHERI_CAP_PERMISSION_USER1__,
		__CHERI_CAP_PERMISSION_USER2__,
		__CHERI_CAP_PERMISSION_USER3__
	};
	static char __permission_names[] = {'G', 'r', 'R', 'M', 'w', 'W', 'L', 'x', 'X', 'S', 's', 'u', 'B', 'C', 'V', '1', '2', '3'};
#endif
	static int q = sizeof(__permissions) / sizeof(__permissions[0]);
	for (int n = 0; n < q; n++) {
		int has = TEST(__permissions[n], perms);
		snprintf(__perms_buffer + n, 2, "%c", has ? __permission_names[n] : '-');
	}
	return __perms_buffer;
}

#ifdef __CHERI_PURE_CAPABILITY__

#define CAP_TAIL_LENGTH(cap) __builtin_cheri_length_get(cap) - __builtin_cheri_offset_get(cap)

#else

#define CAP_TAIL_LENGTH(cap) SIZE_MAX

#endif
