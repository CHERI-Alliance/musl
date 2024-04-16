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
	lc cs0,    0(ca0)
	lc cs1,    16(ca0)
	lc cs2,    32(ca0)
	lc cs3,    48(ca0)
	lc cs4,    64(ca0)
	lc cs5,    80(ca0)
	lc cs6,    96(ca0)
	lc cs7,    112(ca0)
	lc cs8,    128(ca0)
	lc cs9,    144(ca0)
	lc cs10,   160(ca0)
	lc cs11,   176(ca0)
	lc csp,    192(ca0)
	lc cra,    208(ca0)

#ifndef __riscv_float_abi_soft
	fld fs0,  224(ca0)
	fld fs1,  232(ca0)
	fld fs2,  240(ca0)
	fld fs3,  248(ca0)
	fld fs4,  256(ca0)
	fld fs5,  264(ca0)
	fld fs6,  272(ca0)
	fld fs7,  280(ca0)
	fld fs8,  288(ca0)
	fld fs9,  296(ca0)
	fld fs10, 304(ca0)
	fld fs11, 212(ca0)
#endif

	seqz a0, a1
	add a0, a0, a1
	ret
