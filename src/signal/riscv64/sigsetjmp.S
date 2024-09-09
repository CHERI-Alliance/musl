.global sigsetjmp
.global __sigsetjmp
.type sigsetjmp, %function
.type __sigsetjmp, %function
sigsetjmp:
__sigsetjmp:
#ifndef __CHERI_PURE_CAPABILITY__
	bnez a1, 1f
	tail setjmp
1:

	sd ra, 208(a0)
	sd s0, 224(a0)
	mv s0, a0

	call setjmp

	mv a1, a0
	mv a0, s0
	ld s0, 224(a0)
	ld ra, 208(a0)

.hidden __sigsetjmp_tail
	tail __sigsetjmp_tail
#else
	bnez a1, 1f
	lgc ct0, setjmp
	jr ct0
1:

	sc cra, ((14 * 16) + (12 * 8) + 0)(ca0)
	sc cs0, ((14 * 16) + (12 * 8) + 16)(ca0)
	cmv cs0, ca0

	lgc ct0, setjmp
	jalr ct0

	cmv ca1, ca0
	cmv ca0, cs0
	lc cs0, ((14 * 16) + (12 * 8) + 16)(ca0)
	lc cra, ((14 * 16) + (12 * 8) + 0)(ca0)

.hidden __sigsetjmp_tail
	lgc ct0, __sigsetjmp_tail
	jr ct0
#endif
