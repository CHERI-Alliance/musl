#ifdef __CHERI_PURE_CAPABILITY__
#include <sys/auxv.h>
#include <sys/mman.h>
#include <cap_perms.h>
#include "libc.h"
#include "cmpt.h"

#define LABEL(name) \
".global " name " \n" \
".hidden " name "\n" \
".size " name ",16\n" \
".type " name ",%function\n" \
name ":\n"

void trampoline_start(void);
void trampoline_end(void);

__attribute__ ((naked))
void call_to_cap_pair(cap_pair_t* pair)
{
	__asm__ volatile
	(
	 	"ldp c2, c3, [c0, #(0)]\n"
		"ldr c4, [c0, #(32)]\n"
		"mov c0, c2\n"
		"mov c1, c3\n"
		"mov c2, c4\n"
		"sub csp, csp, #(32)\n"
		"stp c29, c30, [csp, #(0)]\n"
		"blrs c29, c0, c1\n"
		LABEL("trampoline_start")
		"mov c0, c29\n"
		"blr c2\n"
		"ldp c29, c30, [csp, #(0)]\n"
		"add csp, csp, #(32)\n"
		"ret\n"
		LABEL("trampoline_end")
	);
}

int create_cap_pair(cap_pair_t* pair, char* data, size_t data_size, void* target_func)
{
	//Max otype is 2^15-1 so keep lowest 14 bits to ensure otype never greater, +4 to prevent otype of 1, 2 or 3 and explicitly prevent 15-bit overflow
	size_t seal_otype = ((((size_t)target_func) & 0x3fffLU) + 4LU) & 0x7fffLU;
	void* root_seal_cap =   __builtin_cheri_offset_set(getauxptr(AT_CHERI_SEAL_CAP), seal_otype);
	void* root_unseal_cap = __builtin_cheri_offset_set(getauxptr(AT_CHERI_SEAL_CAP), 1);

	//STEP: Create code map
	
	//+9 to both prevent off-by-one of trampoline size and +8 to allocate more mem than needed for trampoline code
	size_t trampoline_code_size = (&(trampoline_end) - &(trampoline_start) + 9);
	void* code_map = mmap(NULL, trampoline_code_size, PROT_READ | PROT_WRITE | PROT_EXEC | PROT_CAP_INVOKE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if(code_map == MAP_FAILED)
	{
		return -1;
	}
	//TODO: Change this +4 from being hardcoded, currently arbitrarily chosen value
	pair->code = code_map + 4; //Copy trampoline code into map at offset of 4 (ensures code in exec map surrounded by zeroes)

	//STEP: Copy trampoline code into code map

	void* tramp_code_ptr = (void*)trampoline_start;
	if(__builtin_cheri_sealed_get(tramp_code_ptr)) 
	{
		//If check necessary since &(trampoline_start) not a sentry in dynamic test
		tramp_code_ptr = __builtin_cheri_unseal(tramp_code_ptr, root_unseal_cap);
	}
	//Clear bottom 2 bits to align function pointer on 4-byte boundary
	size_t aligned_tramp_code_ptr = __builtin_cheri_address_get(tramp_code_ptr) & (~(0x3LU));
	tramp_code_ptr = __builtin_cheri_address_set(tramp_code_ptr, aligned_tramp_code_ptr);
	memcpy(pair->code, tramp_code_ptr, trampoline_code_size);
	pair->code = __builtin_cheri_perms_and(pair->code, READ_CAP_PERMS | EXEC_CAP_PERMS | __ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR__);
	pair->code = __builtin_cheri_offset_increment(pair->code, 1); //Inc to prevent switch to A64
	pair->code = __builtin_cheri_seal(pair->code, root_seal_cap);

	//STEP: Create data map and copy user provided data
	
	pair->data = mmap(NULL, data_size, PROT_READ | PROT_WRITE | PROT_CAP_INVOKE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if(pair->data == MAP_FAILED)
	{
		munmap(code_map, trampoline_code_size);
		return -2;
	}
	//fully PCuABI compliant, perms setting necessary because map'd buffer has executable permission on Morello board
	pair->data = __builtin_cheri_perms_and(pair->data, READ_CAP_PERMS | WRITE_CAP_PERMS | __ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR__);
	memcpy(pair->data, data, data_size);
	pair->data = __builtin_cheri_seal(pair->data, root_seal_cap);

	//STEP: Set target func
	
	pair->target = target_func;

	return 0;
}
#endif
