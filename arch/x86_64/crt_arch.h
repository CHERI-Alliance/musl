#if defined(__SANITIZE_CHERISEED__)

__asm__(
".text \n"
".global " START " \n"
START ": \n"
"	xor %rbp,%rbp \n"
// If %rdi is not zero, then this is the dynamically-linked application.
"	test %rdi,%rdi \n"
"	jne .Ldynamic_loaded \n"
// Initialize the sanitizer
"	mov %rsp,%rdi \n"
"	call __cheriseed_static_init \n"
// SHARED means this is the dynamic linker.
// If this is not the dynamic linker then process relocations.
#if !defined(SHARED)
"	xor %rdi,%rdi \n"
"	xor %rsi,%rsi \n"
"	call __cheriseed_relocate \n"
#endif
// Prepare arguments for the rest of the program.
"	mov %rsp,%rdi \n"
"	call __shim_marshal_program_arguments \n"
// [sp] 1st parameter is in rdi
// [_DYNAMIC] 2nd parameter is in rsi, see below
// [argv] 3rd parameter
"	mov (%rsp),%rdx \n"
"	mov (%rdx),%rdx \n"
// [argc] 4th parameter
"	mov (%rsp),%rcx \n"
"	add $0x10,%rcx \n"
// [envp] 5th parameter
"	mov %rcx,%r8 \n"
"	add $0x10,%r8 \n"
// [auxv] 6th parameter
"	mov %r8,%r9 \n"
"	add $0x10,%r9 \n"
"	jmp .Ldynamic \n"
// This is the effective entry when a dynamically-linked
// application is started: process relocations.
".Ldynamic_loaded:\n"
"	push %rcx \n"
"	push %rdx \n"
"	push %rsi \n"
"	push %rdi \n"
".weak __start___cheriseed_initializers \n"
".hidden __start___cheriseed_initializers \n"
"	lea __start___cheriseed_initializers(%rip),%rdi \n"
".weak __stop___cheriseed_initializers \n"
".hidden __stop___cheriseed_initializers \n"
"	lea __stop___cheriseed_initializers(%rip),%rsi \n"
"	call __cheriseed_relocate \n"
"	pop %rdx \n"
"	pop %rcx \n"
"	pop %r8 \n"
"	pop %r9 \n"
"	mov %rsp,%rdi \n"
".Ldynamic:\n"
// Store _DYNAMIC on the stack.
".weak _DYNAMIC \n"
".hidden _DYNAMIC \n"
"	lea _DYNAMIC(%rip),%rsi \n"
"	sub $16,%rsp \n"
"	mov %rsi,(%rsp) \n"
"	mov %rsp,%rsi \n"
// Align the stack down to 16 bytes.
"	andq $-16,%rsp \n"
"	call " START "_c \n"
);

#else  // defined(__SANITIZE_CHERISEED__)

__asm__(
".text \n"
".global " START " \n"
START ": \n"
"	xor %rbp,%rbp \n"
"	mov %rsp,%rdi \n"
".weak _DYNAMIC \n"
".hidden _DYNAMIC \n"
"	lea _DYNAMIC(%rip),%rsi \n"
"	andq $-16,%rsp \n"
"	call " START "_c \n"
);

#endif  // defined(__SANITIZE_CHERISEED__)
