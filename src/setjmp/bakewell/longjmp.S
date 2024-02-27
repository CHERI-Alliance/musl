.option capmode
.global __longjmp
.global _longjmp
.global longjmp
.type __longjmp, %function
.type _longjmp,  %function
.type longjmp,   %function
__longjmp:
_longjmp:
longjmp:
	clc cs0,    0(ca0)
	clc cs1,    16(ca0)
	clc cs2,    32(ca0)
	clc cs3,    48(ca0)
	clc cs4,    64(ca0)
	clc cs5,    80(ca0)
	clc cs6,    96(ca0)
	clc cs7,    112(ca0)
	clc cs8,    128(ca0)
	clc cs9,    144(ca0)
	clc cs10,   160(ca0)
	clc cs11,   176(ca0)
	clc csp,    192(ca0)
	clc cra,    208(ca0)

#ifndef __riscv_float_abi_soft
	cfld fs0,  224(ca0)
	cfld fs1,  232(ca0)
	cfld fs2,  240(ca0)
	cfld fs3,  248(ca0)
	cfld fs4,  256(ca0)
	cfld fs5,  264(ca0)
	cfld fs6,  272(ca0)
	cfld fs7,  280(ca0)
	cfld fs8,  288(ca0)
	cfld fs9,  296(ca0)
	cfld fs10, 304(ca0)
	cfld fs11, 212(ca0)
#endif

	seqz a0, a1
	add a0, a0, a1
	ret
