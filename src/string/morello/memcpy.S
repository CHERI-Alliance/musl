/*
 * memcpy - copy memory area
 *
 * Copyright (c) 2012-2020, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

/* Assumptions:
 *
 * ARMv8-a, AArch64, unaligned accesses.
 *
 */

#define dstin   c0
#define src     c1
#define dst     c3
#define srcend  c4
#define dstend  c5
#define tmp2    x15
#define tmpw    w15
#define tmpc    c15

#define count   x2
#define A_l     x6
#define A_lw    w6
#define A_h     x7
#define B_l     x8
#define B_lw    w8
#define B_h     x9
#define C_l     x10
#define C_lw    w10
#define C_h     x11
#define D_l     x12
#define D_h     x13
#define E_l     x14
#define E_h     x15
#define F_l     x16
#define F_h     x17
#define G_l     count
#define G_h     dst
#define H_l     src
#define H_h     srcend
#define tmp1    x14


.global memcpy
.type memcpy,%function
memcpy:

    cmp     count, #16
    b.lt    .Lcopy_misaligned   // size less than 16: fallback to non-capability copy
    gcvalue tmp1, dstin
    gcvalue tmp2, src
    and     tmp1, tmp1, #15
    and     tmp2, tmp2, #15
    cmp     tmp1, tmp2
    b.ne    .Lcopy_misaligned   // different alignment: fallback to non-capability copy

.Lcopy_aligned:
    alignu  dstend, dstin, #4
    mov     dst, dstin
.Lcopy_pre:
    cmp     dstend, dst
    b.eq    .Lcopy_main
    ldrb    tmpw, [src], #1
    strb    tmpw, [dst], #1
    b       .Lcopy_pre
.Lcopy_main:
    add     dstend, dstin, count
    alignd  dstend, dstend, #4
.Lcopy_capabilities:
    cmp     dstend, dst
    b.eq    .Lcopy_tail
    ldr     tmpc, [src], #16
    str     tmpc, [dst], #16
    b       .Lcopy_capabilities
.Lcopy_tail:
    add     dstend, dstin, count
.Lcopy_post:
    cmp     dstend, dst
    b.eq    .Ldone
    ldrb    tmpw, [src], #1
    strb    tmpw, [dst], #1
    b       .Lcopy_post

.Lcopy_misaligned:
    add     dstend, dstin, count
    mov     dst, dstin
.Lcopy_bytes:
    cmp     dstend, dst
    b.eq    .Ldone
    ldrb    tmpw, [src], #1
    strb    tmpw, [dst], #1
    b       .Lcopy_bytes
.Ldone:
    ret

.size memcpy,.-memcpy
