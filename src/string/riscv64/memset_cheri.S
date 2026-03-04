/*
 * Copyright (c) 2025 Codasip GmbH.
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */
#if defined(__CHERI_PURE_CAPABILITY__)

#include "asmdefs.h"

.global memset
.type memset,%function
memset:
	mv	ca3, ca0
	beqz	a2, .Lexit_memset

	li	t0, CSZREG
	bltu	a2, t0, .Ltail_memset

	andi	t0, a3, (CSZREG-1)
	beqz	t0, .Lcap_aligned_memset_setup

.Lcap_unaligned_head_memset:
	andi	t0, a3, ~(CSZREG-1)
	addi	t0, t0, CSZREG
	sub	t1, t0, a3
	sub	a2, a2, t1
1:
	sb	a1, (ca3)
	add	ca3, ca3, 1
	bltu	a3, t0, 1b

	li	t0, CSZREG
	bltu	a2, t0, .Ltail_memset

.Lcap_aligned_memset_setup:
	andi	a1, a1, 0xFF
	slli	t0, a1, 8
	or	a1, a1, t0
	slli	t0, a1, 16
	or	a1, a1, t0
#if __riscv_xlen == 64
	slli	t0, a1, 32
	or	a1, a1, t0
#endif
	schi	ca1, ca1, a1

.Lcap_aligned_bulk_memset:
	andi	t0, a2, ~(8*CSZREG-1)
	beqz	t0, .Lcap_aligned_single_memset
	sub	a2, a2, t0
	add	t0, a3, t0
1:
	sc	ca1, 0*CSZREG(ca3)
	sc	ca1, 1*CSZREG(ca3)
	sc	ca1, 2*CSZREG(ca3)
	sc	ca1, 3*CSZREG(ca3)
	sc	ca1, 4*CSZREG(ca3)
	sc	ca1, 5*CSZREG(ca3)
	sc	ca1, 6*CSZREG(ca3)
	sc	ca1, 7*CSZREG(ca3)
	add	ca3, ca3, (8*CSZREG)
	bltu	a3, t0, 1b

.Lcap_aligned_single_memset:
	andi	t0, a2, ~(CSZREG-1)
	beqz	t0, .Ltail_memset
	sub	a2, a2, t0
	add	t0, a3, t0
1:
	sc	ca1, 0(ca3)
	add	ca3, ca3, CSZREG
	bltu	a3, t0, 1b

.Ltail_memset:
	beqz	a2, .Lexit_memset
	add	t0, a3, a2
	li	a2, 0
1:
	sb	a1, (ca3)
	add	ca3, ca3, 1
	bne	a3, t0, 1b

.Lexit_memset:
	ret

.size memset,.-memset

#endif
