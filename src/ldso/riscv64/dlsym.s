.global dlsym
.hidden __dlsym
.type dlsym, %function
dlsym:
	cmv ca2, cra
	tail __dlsym
