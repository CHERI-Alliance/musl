#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * This test should pass on a real Morello system
 **/

extern const char __jump_to[1];

static void jump_to_c() {
	__asm__ volatile(
		".global __jump_to\n" \
		".hidden __jump_to\n" \
		".size __jump_to, 1\n" \
		"__jump_to:");

	printf("reached jump_to!\n");

	exit(0);
}

static void catcher(int sig, siginfo_t *si, void *ctx) {
	printf("inside catcher() function\n");

	ucontext_t *uc = ctx;

	for (int k = 0; k < 31; k++) {
		printf("r%02d = %016lx\n", k, uc->uc_mcontext.regs[k]);
	}
	printf("xsp = %016lx\n", uc->uc_mcontext.sp);
	printf(" pc = %016lx\n", uc->uc_mcontext.pc);

	unsigned long *data = (unsigned long *)uc->uc_mcontext.__reserved;
	for (int k = 0; k < 256; k+=2) {
		printf("reserved %03d = %016lx %016lx\n", k, data[k + 1], data[k]);
	}

	struct morello_context *morello_ctx = (struct morello_context *) uc->uc_mcontext.__reserved;
	while (morello_ctx->head.magic && morello_ctx->head.size && morello_ctx->head.magic != MORELLO_MAGIC) {
		morello_ctx = (struct morello_context *) (((unsigned char *) morello_ctx) + morello_ctx->head.size);
	}

//	This should work on a real Morello system with Morello kernel
//	if (!morello_ctx->head.magic || !morello_ctx->head.size) {
//		printf("morello context entry (magic number 0x%x) not found\n", MORELLO_MAGIC);
//		exit(1);
//	}
//	if (!__builtin_cheri_tag_get(morello_ctx->pcc)) {
//		printf("PCC tag in morello context not set (pcc=%#p)\n", (void *) morello_ctx->pcc);
//		exit(2);
//	}

	uc->uc_mcontext.pc = (unsigned long) __builtin_cheri_address_get(__jump_to);
}

int main() {
	struct sigaction sigact;

	printf("jump_to_c addr: %#p\n", (void *) jump_to_c);

	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_flags = sigact.sa_flags | SA_SIGINFO | SA_RESTART;
	sigact.sa_sigaction = catcher;
	sigaction(SIGUSR1, &sigact, NULL);

	printf("raise SIGUSR1 signal\n");
	kill(getpid(), SIGUSR1);

	return 1;
}
