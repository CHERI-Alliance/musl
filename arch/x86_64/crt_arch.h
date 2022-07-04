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
#endif
".weak _DYNAMIC \n"
".hidden _DYNAMIC \n"
"	lea _DYNAMIC(%rip),%rsi \n"
"	andq $-16,%rsp \n"
"	call " START "_c \n"
);
