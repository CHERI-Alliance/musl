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
	csc cs0,    0(ca0)
	csc cs1,    16(ca0)
	csc cs2,    32(ca0)
	csc cs3,    48(ca0)
	csc cs4,    64(ca0)
	csc cs5,    80(ca0)
	csc cs6,    96(ca0)
	csc cs7,    112(ca0)
	csc cs8,    128(ca0)
	csc cs9,    144(ca0)
	csc cs10,   160(ca0)
	csc cs11,   176(ca0)
	csc csp,    192(ca0)
	csc cra,    208(ca0)

#ifndef __riscv_float_abi_soft
	cfsd fs0,  224(ca0)
	cfsd fs1,  232(ca0)
	cfsd fs2,  240(ca0)
	cfsd fs3,  248(ca0)
	cfsd fs4,  256(ca0)
	cfsd fs5,  264(ca0)
	cfsd fs6,  272(ca0)
	cfsd fs7,  280(ca0)
	cfsd fs8,  288(ca0)
	cfsd fs9,  296(ca0)
	cfsd fs10, 304(ca0)
	cfsd fs11, 312(ca0)
#endif

	li a0, 0
	ret
