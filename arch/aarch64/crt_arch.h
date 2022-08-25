__asm__(
".text \n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
"	mov x29, #0\n"
"	mov x30, #0\n"
"	mov x0, sp\n"
#if defined(__SANITIZE_CHERISEED__)
"	bl __cheriseed_static_init \n"
"	mov x0, sp\n"
"	bl __shim_marshal_program_arguments \n"
// [sp] 1st parameter is in x0
// [_DYNAMIC] 2nd parameter is in x1, see below
// [argv] 3rd parameter
// [argc] 4th parameter
// [envp] 5th parameter
// [auxv] 6th parameter
"	ldr x2, [x0, #0] \n"
"	add x3, x2, #16 \n"
"	add x4, x2, #32 \n"
"	add x5, x2, #48 \n"
"	ldr x2, [x2, #0] \n"
#endif
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n"
"	adrp x1, _DYNAMIC\n"
"	add x1, x1, #:lo12:_DYNAMIC\n"
"	and sp, x0, #-16\n"
"	b " START "_c\n"
);
