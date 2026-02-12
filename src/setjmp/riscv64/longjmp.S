.global __longjmp
.global _longjmp
.global longjmp
.type __longjmp, %function
.type _longjmp,  %function
.type longjmp,   %function
__longjmp:
_longjmp:
longjmp:
#ifndef __CHERI_PURE_CAPABILITY__
	ld s0,    0(a0)
	ld s1,    8(a0)
	ld s2,    16(a0)
	ld s3,    24(a0)
	ld s4,    32(a0)
	ld s5,    40(a0)
	ld s6,    48(a0)
	ld s7,    56(a0)
	ld s8,    64(a0)
	ld s9,    72(a0)
	ld s10,   80(a0)
	ld s11,   88(a0)
	ld sp,    96(a0)
	ld ra,    104(a0)

#ifndef __riscv_float_abi_soft
#ifdef __riscv_float_abi_double
#define FLX fld
#else
#define FLX flw
#endif

	FLX fs0,  112(a0)
	FLX fs1,  120(a0)
	FLX fs2,  128(a0)
	FLX fs3,  136(a0)
	FLX fs4,  144(a0)
	FLX fs5,  152(a0)
	FLX fs6,  160(a0)
	FLX fs7,  168(a0)
	FLX fs8,  176(a0)
	FLX fs9,  184(a0)
	FLX fs10, 192(a0)
	FLX fs11, 200(a0)
#endif

	seqz a0, a1
	add a0, a0, a1
	ret
#else
	lc cs0,    (0*16)(ca0)
	lc cs1,    (1*16)(ca0)
	lc cs2,    (2*16)(ca0)
	lc cs3,    (3*16)(ca0)
	lc cs4,    (4*16)(ca0)
	lc cs5,    (5*16)(ca0)
	lc cs6,    (6*16)(ca0)
	lc cs7,    (7*16)(ca0)
	lc cs8,    (8*16)(ca0)
	lc cs9,    (9*16)(ca0)
	lc cs10,   (10*16)(ca0)
	lc cs11,   (11*16)(ca0)
	lc csp,    (12*16)(ca0)
	lc cra,    (13*16)(ca0)
	caddi ca0, ca0, (14 * 16)

#ifndef __riscv_float_abi_soft
	fld fs0,  (0*8)(ca0)
	fld fs1,  (1*8)(ca0)
	fld fs2,  (2*8)(ca0)
	fld fs3,  (3*8)(ca0)
	fld fs4,  (4*8)(ca0)
	fld fs5,  (5*8)(ca0)
	fld fs6,  (6*8)(ca0)
	fld fs7,  (7*8)(ca0)
	fld fs8,  (8*8)(ca0)
	fld fs9,  (9*8)(ca0)
	fld fs10, (10*8)(ca0)
	fld fs11, (11*8)(ca0)
	caddi ca0, ca0, (12 * 8)
#endif

	seqz a0, a1
	add a0, a0, a1
	ret
#endif
