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
	clw   t0, 0(ca0)
	bnez  t0, __cp_cancel

	cmove ct0, ca1
	cmove ca0, ca2
	cmove ca1, ca3
	cmove ca2, ca4
	cmove ca3, ca5
	cmove ca4, ca6
	cmove ca5, ca7
	clc   ca6, 0(csp)
	cmove ca7, ct0
	ecall
__cp_end:
	ret
__cp_cancel:
	ctail __cancel
	.size __cp_cancel, .-__cp_cancel
	.size __cp_begin, __cp_end-__cp_begin

