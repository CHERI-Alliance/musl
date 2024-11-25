.global __cp_begin
.hidden __cp_begin
.global __cp_end
.hidden __cp_end
.global __cp_cancel
.hidden __cp_cancel
.hidden __cancel
.global __syscall_cp_asm
.hidden __syscall_cp_asm
.type __syscall_cp_asm, %function
__syscall_cp_asm:
__cp_begin:
	.cfi_startproc
#ifndef __CHERI_PURE_CAPABILITY__
	lw t0, 0(a0)
	bnez t0, __cp_cancel

	mv t0, a1
	mv a0, a2
	mv a1, a3
	mv a2, a4
	mv a3, a5
	mv a4, a6
	mv a5, a7
	ld a6, 0(sp)
	mv a7, t0
	ecall
__cp_end:
	ret
__cp_cancel:
	tail __cancel
#else
	lw t0, 0(ca0)
	bnez t0, __cp_cancel

	mv t0, a1
	cmv ca0, ca2
	cmv ca1, ca3
	cmv ca2, ca4
	cmv ca3, ca5
	cmv ca4, ca6
	cmv ca5, ca7
	ld a6, 0(csp)
	mv a7, t0
	ecall
__cp_end:
	ret
__cp_cancel:
	lgc ct0, __cancel
	jr ct0
#endif
	.cfi_endproc
	.size __cp_cancel, .-__cp_cancel
	.size __cp_begin, __cp_end-__cp_begin
	
