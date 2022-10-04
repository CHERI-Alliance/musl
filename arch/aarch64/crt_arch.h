#if defined(__SANITIZE_CHERISEED__)

__asm__(
".text\n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
"	mov x29, #0\n"
"	mov x30, #0\n"
// If x0 is not zero, then this is the dynamically-linked application.
"	cbnz x0, .Ldynamic_loaded\n"
// Initialize the sanitizer
"	mov x0, sp\n"
"	bl __cheriseed_static_init\n"
// SHARED means this is the dynamic linker.
// If this is not the dynamic linker then process relocations.
#if !defined(SHARED)
"	mov x0, xzr\n"
"	mov x1, xzr\n"
"	bl __cheriseed_relocate\n"
#endif
// Prepare arguments for the rest of the program.
"	mov x0, sp\n"
"	bl __shim_marshal_program_arguments\n"
// [sp] 1st parameter is in x0
// [_DYNAMIC] 2nd parameter is in x1, see below
// [argv] 3rd parameter
// [argc] 4th parameter
// [envp] 5th parameter
// [auxv] 6th parameter
"	ldr x2, [x0, #0]\n"
"	add x3, x2, #16\n"
"	add x4, x2, #32\n"
"	add x5, x2, #48\n"
"	ldr x2, [x2, #0]\n"
"	b .Ldynamic\n"
// This is the effective entry when a dynamically-linked
// application is started: process relocations.
".Ldynamic_loaded:\n"
"	stp x0, x1, [sp, #-32]!\n"
"	stp x2, x3, [sp, #16]\n"
".weak __start___cheriseed_initializers\n"
".hidden __start___cheriseed_initializers\n"
"	adrp x0, __start___cheriseed_initializers\n"
"	add x0, x0, :lo12:__start___cheriseed_initializers\n"
".weak __stop___cheriseed_initializers\n"
".hidden __stop___cheriseed_initializers\n"
"	adrp x1, __stop___cheriseed_initializers\n"
"	add x1, x1, :lo12:__stop___cheriseed_initializers\n"
"	bl __cheriseed_relocate\n"
"	ldp x4, x5, [sp, #16]\n"
"	ldp x2, x3, [sp], #32\n"
"	mov x0, sp\n"
".Ldynamic:\n"
// Store _DYNAMIC on the stack.
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n"
"	adrp x1, _DYNAMIC\n"
"	add x1, x1, :lo12:_DYNAMIC\n"
"	str x1, [sp, #-16]!\n"
"	mov x1, sp\n"
// Align the stack down to 16 bytes.
"	mov x6, sp\n"
"	and sp, x6, #-16\n"
"	b " START "_c\n"
);

#else  // defined(__SANITIZE_CHERISEED__)

__asm__(
".text \n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
"	mov x29, #0\n"
"	mov x30, #0\n"
"	mov x0, sp\n"
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n"
"	adrp x1, _DYNAMIC\n"
"	add x1, x1, #:lo12:_DYNAMIC\n"
"	and sp, x0, #-16\n"
"	b " START "_c\n"
);

#endif  // defined(__SANITIZE_CHERISEED__)
