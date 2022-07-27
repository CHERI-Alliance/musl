.global __unmapself
.type   __unmapself,%function
__unmapself:
#ifdef __SANITIZE_CHERISEED__
	stp x29, x30, [sp, #-32]!
	str x1, [sp, #16]
	mov x29, sp
	bl __cheriseed_address_get
	ldr x1, [sp, #16]
	ldp x29, x30, [sp], #32
#endif
	mov x8,#215 // SYS_munmap
	svc 0
	mov x8,#93 // SYS_exit
	svc 0
