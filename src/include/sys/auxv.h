#ifndef SYS_AUXV_H
#define SYS_AUXV_H

#include "../../../include/sys/auxv.h"

#include <features.h>

hidden unsigned long __getauxval(unsigned long);
hidden void *__getauxptr(unsigned long);

#endif
