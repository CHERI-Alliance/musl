/* SPDX-License-Identifier: GPL-2.0-only */

#include "asmdefs.h"

#define FUNCNAME	__strchrnul

#define BYTEREG		a1
#define EPILOG		epilog

.macro epilog
.Lnot_found:
.Lfound:
	/* strchnul always returns the pointer */
	CINSN(addi)	CREG(a0), CREG(a0), -1
	ret
.endm

#include "strchr_template.h"

.global strchrnul
.set strchrnul, __strchrnul
