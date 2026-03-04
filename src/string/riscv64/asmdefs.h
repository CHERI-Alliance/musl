#pragma once

#include "asmdefs.h"
#if __riscv_xlen == 64
#define __REG_SEL(a, b) a
#elif __riscv_xlen == 32
#define __REG_SEL(a, b) b
#else
#error "Unexpected __riscv_xlen"
#endif

#define REG_L           __REG_SEL(ld, lw)
#define REG_S           __REG_SEL(sd, sw)

#define SZREG           __REG_SEL(8, 4)
#define LGREG           __REG_SEL(3, 2)

#if defined(__CHERI_PURE_CAPABILITY__)
#define CREG(X)		c##X
#define CREGN(X)	c##X
#define CINSN(X)	c##X
#define CSZREG          __REG_SEL(16, 8)
#define CLGREG          __REG_SEL(4, 3)
#define CREG_L		lc
#define CREG_S		sc
#else
#define CREG(X)		X
#define CREGN(X)	x##X
#define CINSN(X)	X
#define CSZREG          SZREG
#define CLGREG          LGREG
#define CREG_L		REG_L
#define CREG_S		REG_S
#endif

#define SYM_FUNC_START(N) .global N; .type N,%function;N:
#define SYM_FUNC_END(N) .size N,.-N

