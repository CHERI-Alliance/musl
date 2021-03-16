.global __set_thread_area
.hidden __set_thread_area
.type   __set_thread_area,@function
__set_thread_area:
#ifdef MORELLO
	msr ctpidr_el0,c0
#else
	msr tpidr_el0,x0
#endif
	mov w0,#0
	ret
