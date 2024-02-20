#define SETJMPSIZE (20 * 16) // Size of jmp_buf

.global sigsetjmp
.global __sigsetjmp
.type sigsetjmp, %function
.type __sigsetjmp, %function
sigsetjmp:
__sigsetjmp:
	bnez a1, 1f
	ctail setjmp
1:

	csc cra, SETJMPSIZE(ca0)
	csc cs0, SETJMPSIZE+16+16(ca0)
	cmove cs0, ca0

	ccall setjmp

	cmove ca1, ca0
	cmove ca0, cs0
	clc cs0, 224(ca0)
	clc cra, SETJMPSIZE+16+16(ca0)

.hidden __sigsetjmp_tail
	ctail __sigsetjmp_tail
