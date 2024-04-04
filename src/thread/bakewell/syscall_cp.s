.option capmode
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
	lw    t0, 0(ca0)
	bnez  t0, __cp_cancel

	cmv   ct0, ca1
	cmv   ca0, ca2
	cmv   ca1, ca3
	cmv   ca2, ca4
	cmv   ca3, ca5
	cmv   ca4, ca6
	cmv   ca5, ca7
	lc   ca6, 0(csp)
	cmv   ca7, ct0
	ecall
__cp_end:
	ret
__cp_cancel:
	tail  __cancel
	.size __cp_cancel, .-__cp_cancel
	.size __cp_begin, __cp_end-__cp_begin

