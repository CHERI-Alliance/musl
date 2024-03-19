#define SETJMPSIZE (20 * 16) // Size of jmp_buf

.global sigsetjmp
.global __sigsetjmp
.type sigsetjmp, %function
.type __sigsetjmp, %function
sigsetjmp:
__sigsetjmp:
	bnez a1, 1f
	tail setjmp
1:

	sc cra, SETJMPSIZE(ca0)
	sc cs0, SETJMPSIZE+16+16(ca0)
	cmv cs0, ca0

	call setjmp

	cmv ca1, ca0
	cmv ca0, cs0
	lc cs0, 224(ca0)
	lc cra, SETJMPSIZE+16+16(ca0)

.hidden __sigsetjmp_tail
	tail __sigsetjmp_tail
