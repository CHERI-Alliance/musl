__asm__(
".text \n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
"	mov x29, #0\n"
"	mov x30, #0\n"
#ifdef MORELLO
#if MUSL_USE_LIBSHIM
"	mov x0, sp\n"
"	cvtd c0, x0\n"
"	mov csp, c0\n"
#endif
"	bl __morello_init_static\n"
"	mov c0, csp\n"
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n"
"	adrp c1, _DYNAMIC\n"
"	add c1, c1, #:lo12:_DYNAMIC\n"
"	alignd csp, csp, #4\n"
#else
"	mov x0, sp\n"
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n"
"	adrp x1, _DYNAMIC\n"
"	add x1, x1, #:lo12:_DYNAMIC\n"
"	and sp, x0, #-16\n"
#endif
"	b " START "_c\n"
);

#ifdef MORELLO
__asm__ (".text \n"
".global __morello_init_static\n"
".type __morello_init_static,%function\n"
"__morello_init_static:\n"
"	adrp    c0, __cap_relocs_start\n"
"	add     c0, c0, :lo12:__cap_relocs_start\n"
"	adrp    c1, __cap_relocs_end\n"
"	add     c1, c1, :lo12:__cap_relocs_end\n"
"1:\n"
"	cmp     c0, c1\n"
"	b.eq    2f\n"
"	ldr     x2, [c0, #0]\n"
"	cvtd    c2, x2\n"
"	ldp     x3, x4, [c0, #8]\n"
"	ldp     x5, x6, [c0, #24]\n"
"	orr     x6, x6, #~(0b111111111111111111)\n"
"	cvtd    c3, x3\n"
"	scbndse c3, c3, x5\n"
"	add     c3, c3, x4\n"
"	clrperm c3, c3, x6\n"
"	str     c3, [c2]\n"
"	add     c0, c0, #40\n"
"	b       1b\n"
"2:\n"
"	ret\n");
#endif
