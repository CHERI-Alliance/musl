#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"

int fchown(int fd, uid_t uid, gid_t gid)
{
	int ret = __syscall(SYS_fchown, fd, uid, gid);
	return __syscall_ret(ret);
}
