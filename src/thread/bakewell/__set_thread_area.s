.global __set_thread_area
.type   __set_thread_area, %function
__set_thread_area:
	cmove ctp, ca0
	li a0, 0
	cret
.size __set_thread_area, .-__set_thread_area;
