#ifdef __riscv_flen

.global feclearexcept
.type feclearexcept, %function
feclearexcept:
	csrc fflags, a0
	li a0, 0
	ret
	.size feclearexcept, .-feclearexcept

.global feraiseexcept
.type feraiseexcept, %function
feraiseexcept:
	csrs fflags, a0
	li a0, 0
	ret
	.size feraiseexcept, .-feraiseexcept

.global fetestexcept
.type fetestexcept, %function
fetestexcept:
	frflags t0
	and a0, t0, a0
	ret
	.size fetestexcept, .-fetestexcept

.global fegetround
.type fegetround, %function
fegetround:
	frrm a0
	ret
	.size fegetround, .-fegetround

.global __fesetround
.type __fesetround, %function
__fesetround:
	fsrm t0, a0
	li a0, 0
	ret
	.size __fesetround, .-__fesetround

.global fegetenv
.type fegetenv, %function
fegetenv:
	frcsr t0
	csw t0, 0(ca0)
	li a0, 0
	ret
	.size fegetenv, .-fegetenv

.global fesetenv
.type fesetenv, %function
fesetenv:
	li t2, -1
	li t1, 0
	beq a0, t2, 1f
	clw t1, 0(ca0)
1:	fscsr t1
	li a0, 0
	ret
	.size fesetenv, .-fesetenv

#endif
