.global dlsym
.hidden __dlsym
.type dlsym, %function
dlsym:
#ifdef __CHERI_PURE_CAPABILITY__
	cmv ca2, cra
#else
	mv a2, ra
#endif
	tail __dlsym
