.global __setjmp
.global _setjmp
.global setjmp
.type __setjmp, %function
.type _setjmp,  %function
.type setjmp,   %function
__setjmp:
_setjmp:
setjmp:
#ifndef __CHERI_PURE_CAPABILITY__
	sd s0,    0(a0)
	sd s1,    8(a0)
	sd s2,    16(a0)
	sd s3,    24(a0)
	sd s4,    32(a0)
	sd s5,    40(a0)
	sd s6,    48(a0)
	sd s7,    56(a0)
	sd s8,    64(a0)
	sd s9,    72(a0)
	sd s10,   80(a0)
	sd s11,   88(a0)
	sd sp,    96(a0)
	sd ra,    104(a0)

#ifndef __riscv_float_abi_soft
	fsd fs0,  112(a0)
	fsd fs1,  120(a0)
	fsd fs2,  128(a0)
	fsd fs3,  136(a0)
	fsd fs4,  144(a0)
	fsd fs5,  152(a0)
	fsd fs6,  160(a0)
	fsd fs7,  168(a0)
	fsd fs8,  176(a0)
	fsd fs9,  184(a0)
	fsd fs10, 192(a0)
	fsd fs11, 200(a0)
#endif

	li a0, 0
	ret
#else
	sc cs0,    (0*16)(ca0)
	sc cs1,    (1*16)(ca0)
	sc cs2,    (2*16)(ca0)
	sc cs3,    (3*16)(ca0)
	sc cs4,    (4*16)(ca0)
	sc cs5,    (5*16)(ca0)
	sc cs6,    (6*16)(ca0)
	sc cs7,    (7*16)(ca0)
	sc cs8,    (8*16)(ca0)
	sc cs9,    (9*16)(ca0)
	sc cs10,   (10*16)(ca0)
	sc cs11,   (11*16)(ca0)
	sc csp,    (12*16)(ca0)
	sc cra,    (13*16)(ca0)
	caddi ca0, ca0, (14 * 16)

#ifndef __riscv_float_abi_soft
	fsd fs0,  (0*8)(ca0)
	fsd fs1,  (1*8)(ca0)
	fsd fs2,  (2*8)(ca0)
	fsd fs3,  (3*8)(ca0)
	fsd fs4,  (4*8)(ca0)
	fsd fs5,  (5*8)(ca0)
	fsd fs6,  (6*8)(ca0)
	fsd fs7,  (7*8)(ca0)
	fsd fs8,  (8*8)(ca0)
	fsd fs9,  (9*8)(ca0)
	fsd fs10, (10*8)(ca0)
	fsd fs11, (11*8)(ca0)
	caddi ca0, ca0, (12 * 8)
#endif

	li a0, 0
	ret
#endif
.size __setjmp, .-__setjmp
.size _setjmp,  .-_setjmp
.size setjmp,   .-setjmp
