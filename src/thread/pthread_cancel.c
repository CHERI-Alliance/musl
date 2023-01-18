#define _GNU_SOURCE
#include <string.h>
#include "pthread_impl.h"
#include "stdbool.h"
#include "syscall.h"

// cancel_pc_t is the type of PC: it changes if the sanitizer is enabled.
#ifdef __SANITIZE_CHERISEED__
#define cancel_pc_t long
#else
#define cancel_pc_t uintptr_t
#endif

hidden cancel_pc_t __cancel();
#ifdef __SANITIZE_CHERISEED__
// In cancel_handler it might happen that the interrupted SP is not
// 16-bytes aligned. CHERIseed requires correct capability alignment.
// This attribute ensures that the stack is always re-aligned in __cancel().
__attribute__((force_align_arg_pointer))
#endif
cancel_pc_t __cancel()
{
	pthread_t self = __pthread_self();
	if (self->canceldisable == PTHREAD_CANCEL_ENABLE || self->cancelasync)
		pthread_exit(PTHREAD_CANCELED);
	self->canceldisable = PTHREAD_CANCEL_DISABLE;
	return -ECANCELED;
}

#if defined(__SANITIZE_CHERISEED__)

static intptr_t __syscall_cp_asm(volatile int *cp, long nr,
                    syscall_arg_t u, syscall_arg_t v, syscall_arg_t w,
                    syscall_arg_t x, syscall_arg_t y, syscall_arg_t z)
{
	if (*cp)
		return __cancel();
	return __shim_syscall(cp, nr, u, v, w, x, y, z);
}

extern const char __shim_cp_begin[1], __shim_cp_end[1];
#define __cp_cancel __cancel

static bool is_pc_cancellable(pthread_t self, cancel_pc_t pc)
{
	// Only cancel if the system call is cancellable and PC is within the
	// the cancellable range.
	return (cancel_pc_t)__shim_cp_begin <= pc && pc < (cancel_pc_t)__shim_cp_end;
}

cancel_pc_t __shim_cancel_syscall(void) __attribute__((alias("__cancel")));

#else  // #if defined(__SANITIZE_CHERISEED__)

hidden intptr_t __syscall_cp_asm();
intptr_t __syscall_cp_asm(volatile int *, long,
                      syscall_arg_t, syscall_arg_t, syscall_arg_t,
                      syscall_arg_t, syscall_arg_t, syscall_arg_t);

extern hidden const char __cp_begin[1], __cp_end[1], __cp_cancel[1];

static bool is_pc_cancellable(pthread_t, cancel_pc_t pc)
{
	return pc >= (cancel_pc_t)__cp_begin && pc < (cancel_pc_t)__cp_end;
}

#endif  // #if defined(__SANITIZE_CHERISEED__)

hidden intptr_t __syscall_cp_c();
intptr_t __syscall_cp_c(long nr,
                    syscall_arg_t u, syscall_arg_t v, syscall_arg_t w,
                    syscall_arg_t x, syscall_arg_t y, syscall_arg_t z)
{
	pthread_t self;
	intptr_t r;
	int st;

	if ((st=(self=__pthread_self())->canceldisable)
	    && (st==PTHREAD_CANCEL_DISABLE || nr==SYS_close))
		return __syscall(nr, u, v, w, x, y, z);

	r = __syscall_cp_asm(&self->cancel, nr, u, v, w, x, y, z);
	if (r==-EINTR && nr!=SYS_close && self->cancel &&
	    self->canceldisable != PTHREAD_CANCEL_DISABLE)
		r = __cancel();
	return r;
}

static void _sigaddset(sigset_t *set, int sig)
{
	unsigned s = sig-1;
	set->__bits[s/8/sizeof *set->__bits] |= 1UL<<(s&8*sizeof *set->__bits-1);
}

static void cancel_handler(int sig, siginfo_t *si, void *ctx)
{
	pthread_t self = __pthread_self();
	ucontext_t *uc = ctx;
	cancel_pc_t pc = uc->uc_mcontext.MC_PC;

	a_barrier();
	if (!self->cancel || self->canceldisable == PTHREAD_CANCEL_DISABLE) return;

	_sigaddset(&uc->uc_sigmask, SIGCANCEL);

	if (self->cancelasync || is_pc_cancellable(self, pc)) {
		uc->uc_mcontext.MC_PC = (cancel_pc_t)__cp_cancel;
#ifdef CANCEL_GOT
		uc->uc_mcontext.MC_GOT = CANCEL_GOT;
#endif
		return;
	}

	__syscall(SYS_tkill, self->tid, SIGCANCEL);
}

void __testcancel()
{
	pthread_t self = __pthread_self();
	if (self->cancel && !self->canceldisable)
		__cancel();
}

static void init_cancellation()
{
	struct sigaction sa = {
		.sa_flags = SA_SIGINFO | SA_RESTART,
		.sa_sigaction = cancel_handler
	};
	memset(&sa.sa_mask, -1, _NSIG/8);
	__libc_sigaction(SIGCANCEL, &sa, 0);
}

int pthread_cancel(pthread_t t)
{
	static int init;
	if (!init) {
		init_cancellation();
		init = 1;
	}
	a_store(&t->cancel, 1);
	if (t == pthread_self()) {
		if (t->canceldisable == PTHREAD_CANCEL_ENABLE && t->cancelasync)
			pthread_exit(PTHREAD_CANCELED);
		return 0;
	}
	return pthread_kill(t, SIGCANCEL);
}
