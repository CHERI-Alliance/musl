.global __set_thread_area
.type   __set_thread_area,@function
__set_thread_area:
#ifdef __SANITIZE_CHERISEED__
	stp x29, x30, [sp, #-16]!
	mov x29, sp
	bl __cheriseed_address_get
	ldp x29, x30, [sp], #16
#endif
	msr tpidr_el0,x0
	mov w0,#0
	ret
