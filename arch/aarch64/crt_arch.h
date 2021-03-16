__asm__(
".text \n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
"	mov x29, #0\n"
"	mov x30, #0\n"
#ifdef MORELLO
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
