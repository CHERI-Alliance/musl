#include "crt_regs.S"

.section .init
	ldp r_FP,r_LR,[r_SP],#INC
	ret

.section .fini
	ldp r_FP,r_LR,[r_SP],#INC
	ret
