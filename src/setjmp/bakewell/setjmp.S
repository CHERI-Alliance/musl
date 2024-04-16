.option capmode
.global __setjmp
.global _setjmp
.global setjmp
.type __setjmp, %function
.type _setjmp,  %function
.type setjmp,   %function
__setjmp:
_setjmp:
setjmp:
	sc cs0,    0(ca0)
	sc cs1,    16(ca0)
	sc cs2,    32(ca0)
	sc cs3,    48(ca0)
	sc cs4,    64(ca0)
	sc cs5,    80(ca0)
	sc cs6,    96(ca0)
	sc cs7,    112(ca0)
	sc cs8,    128(ca0)
	sc cs9,    144(ca0)
	sc cs10,   160(ca0)
	sc cs11,   176(ca0)
	sc csp,    192(ca0)
	sc cra,    208(ca0)

#ifndef __riscv_float_abi_soft
	fsd fs0,  224(ca0)
	fsd fs1,  232(ca0)
	fsd fs2,  240(ca0)
	fsd fs3,  248(ca0)
	fsd fs4,  256(ca0)
	fsd fs5,  264(ca0)
	fsd fs6,  272(ca0)
	fsd fs7,  280(ca0)
	fsd fs8,  288(ca0)
	fsd fs9,  296(ca0)
	fsd fs10, 304(ca0)
	fsd fs11, 312(ca0)
#endif

	li a0, 0
	ret
