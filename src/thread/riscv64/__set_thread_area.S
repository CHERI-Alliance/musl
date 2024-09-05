.global __set_thread_area
.type   __set_thread_area, %function
__set_thread_area:
#ifndef __CHERI_PURE_CAPABILITY__
	mv tp, a0
#else
	cmv ctp, ca0
#endif
	li a0, 0
	ret
.size __set_thread_area, .-__set_thread_area
