# __clone(func, stack, flags, arg, ptid, tls, ctid)
#           a0,    a1,    a2,  a3,   a4,  a5,   a6

# syscall(SYS_clone, flags, stack, ptid, tls, ctid)
#                a7     a0,    a1,   a2,  a3,   a4

// TODO: Check if varargs should be copied, like morello

.global __clone
.type  __clone, %function
__clone:
	# Save func and arg to stack
	cincoffset ca1, ca1, -32
	csc ca0, 0(ca1)
	csc ca3, 16(ca1)

	# Call SYS_clone
	mv a0, a2
	cmove ca2, ca4
	cmove ca3, ca5
	cmove ca4, ca6
	li a7, 220 # SYS_clone
	ecall

	beqz a0, 1f
	# Parent
	cret

	# Child
1:  clc ca1, 0(csp)
	clc ca0, 16(csp)
	cjalr ca1

	# Exit
	li a7, 93 # SYS_exit
	ecall
	.size __clone, .-__clone
