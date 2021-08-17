// __clone(func, stack, flags, arg, ptid, tls, ctid)
//         x0,   x1,    w2,    x3,  x4,   x5,  x6

// syscall(SYS_clone, flags, stack, ptid, tls, ctid)
//         x8,        x0,    x1,    x2,   x3,  x4

.global __clone
.hidden __clone
.type   __clone,%function
__clone:
	// align stack
#ifdef MORELLO
	alignd c1,c1,#4
#else
	and x1,x1,#-16
#endif

#ifdef MORELLO

#ifdef MUSL_USE_LIBSHIM
	// store tls, flags
	stp c5,c2,[c1,#-32]!
#endif

	// store func, arg
	stp c0,c3,[c1,#-32]!
#else
	stp x0,x3,[x1,#-16]!
#endif // MORELLO

	uxtw x0,w2
#ifdef MORELLO
	mov c2,c4
	mov c3,c5
	mov c4,c6
#else
	mov x2,x4
	mov x3,x5
	mov x4,x6
#endif // MORELLO
	mov x8,#220 // SYS_clone

	svc #0

	cbz x0,1f
	// parent
	ret

	// child
#ifdef MORELLO
1:	ldp c1,c0,[csp],#32

#ifdef MUSL_USE_LIBSHIM
	// for now we have to manually set tls, as the kernel is not going to
	// set a capability properly

	// load tls, flags
	ldp c2,c3,[csp],#32

	and x3,x3,0x00080000 // CLONE_SETTLS
	cbz x3,2f
	msr ctpidr_el0, c2
#endif

2:	blr c1
#else
1:	ldp x1,x0,[sp],#16

	blr x1
#endif

	mov x8,#93 // SYS_exit
	svc #0