#ifdef __riscv_flen

#ifdef __CHERI_PURE_CAPABILITY__
#define CA0 ca0
#else
#define CA0 a0
#endif
	
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
	sw t0, 0(CA0)
	li a0, 0
	ret
	.size fegetenv, .-fegetenv

.global fesetenv
.type fesetenv, %function
fesetenv:
	li t2, -1
	li t1, 0
	beq a0, t2, 1f
	lw t1, 0(CA0)
1:	fscsr t1
	li a0, 0
	ret
	.size fesetenv, .-fesetenv

#endif
