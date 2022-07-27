/* Copyright 2011-2012 Nicholas J. Kain, licensed under standard MIT license */
.text
.global __unmapself
.type   __unmapself,@function
__unmapself:
#ifdef __SANITIZE_CHERISEED__
	push %rbp
	push %rsi
	call __cheriseed_address_get
	mov %rax, %rdi
	pop %rsi
	pop %rbp
#endif
	movl $11,%eax   /* SYS_munmap */
	syscall         /* munmap(arg2,arg3) */
	xor %rdi,%rdi   /* exit() args: always return success */
	movl $60,%eax   /* SYS_exit */
	syscall         /* exit(0) */
