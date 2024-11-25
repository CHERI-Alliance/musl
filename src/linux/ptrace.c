#include <sys/ptrace.h>
#include <stdarg.h>
#include <unistd.h>
#include "syscall.h"

long ptrace(int req, pid_t pid, void *addr, void *data)
{
	long ret, result;

	if (req-1U < 3) data = &result;
	ret = syscall(SYS_ptrace, req, pid, addr, data);

	if (ret < 0 || req-1U >= 3) return ret;
	return result;
}
