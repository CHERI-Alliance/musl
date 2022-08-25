__asm__(
".text \n"
".global " START " \n"
START ": \n"
"	xor %rbp,%rbp \n"
"	mov %rsp,%rdi \n"
#if defined(__SANITIZE_CHERISEED__)
"	call __cheriseed_static_init \n"
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
#endif
".weak _DYNAMIC \n"
".hidden _DYNAMIC \n"
"	lea _DYNAMIC(%rip),%rsi \n"
"	andq $-16,%rsp \n"
"	call " START "_c \n"
);
