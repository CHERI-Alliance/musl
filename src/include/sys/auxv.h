#ifndef SYS_AUXV_H
#define SYS_AUXV_H

#include "../../../include/sys/auxv.h"

#include <features.h>

hidden uintptr_t __getauxval(unsigned long);

#endif
