# __clone(func, stack, flags, arg, ptid, tls, ctid)
#           a0,    a1,    a2,  a3,   a4,  a5,   a6
# cheri:   ca0,   ca1,    a2, ca3, 0(csp), 16(csp), 32(csp)

# syscall(SYS_clone, flags, stack, ptid, tls, ctid)
#                a7     a0,    a1,   a2,  a3,   a4

.global __clone
.type  __clone, %function
__clone:
#ifndef __CHERI_PURE_CAPABILITY__
	# Save func and arg to stack
	addi a1, a1, -16
	sd a0, 0(a1)
	sd a3, 8(a1)

	# Call SYS_clone
	mv a0, a2
	mv a2, a4
	mv a3, a5
	mv a4, a6
	li a7, 220 # SYS_clone
	ecall

	beqz a0, 1f
	# Parent
	ret

	# Child
1:      ld a1, 0(sp)
	ld a0, 8(sp)
	jalr a1

	# Exit
	li a7, 93 # SYS_exit
	ecall
#else
	# Save func and arg to stack
	caddi ca1, ca1, -32
	sc ca0, 0(ca1)
	sc ca3, 16(ca1)

	# Call SYS_clone
	mv a0, a2
	lc ca2, 0(csp)
	lc ca3, 16(csp)
	lc ca4, 32(csp)
	li a7, 220 # SYS_clone
	ecall

	beqz a0, 1f
	# Parent
	ret

	# Child
1:      lc ca1, 0(csp)
	lc ca0, 16(csp)
	jalr ca1

	# Exit
	li a7, 93 # SYS_exit
	ecall
#endif
	.size __clone, .-__clone
	
