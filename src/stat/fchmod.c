#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"

int fchmod(int fd, mode_t mode)
{
	int ret = __syscall(SYS_fchmod, fd, mode);
	return __syscall_ret(ret);
}
