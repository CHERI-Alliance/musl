#ifdef MORELLO

#include <memory.h>
#include <stdint.h>
#include <string.h>

#include "elf.h"

inline void load_convert_store(long*src, char**dst, int len)  {
	uintptr_t tmp_cap;
	uint64_t tmp_ptr;
	__asm__ volatile ("ldr %0, [%1]" : "=r"(tmp_ptr) : "C"(src));
	__asm__ volatile ("scvalue %0, csp, %1" : "=C"(tmp_cap) : "r"(tmp_ptr));
	if (len == -1) len = strlen((char*)tmp_cap) + 1;
	__asm__ volatile ("scbnds %0, %0, %1" : "+C"(tmp_cap) : "r"(len));
	__asm__ volatile ("str %0, [%1]" :: "C"(tmp_cap), "r"(dst));
}

int morello_get_init_stack_num_bytes(long* argv_ptr) {
	int nargv = 0;
	for(; argv_ptr[nargv]; nargv++);

	int nenvp = 0;
	for (; argv_ptr[nargv + 1 + nenvp]; nenvp++);

	int nauxv = 0;
	for (; ((uintptr_t*)(argv_ptr + nargv + nenvp + 2))[nauxv]; nauxv++);

	return sizeof(long) * ((nargv + 1) * 2 +
	                       (nenvp + 1) * 2 +
	                       (nauxv + 1) * 8);
}

void morello_init_stack_args(long* argv_ptr, char **argv_cap) {
	int i = 0;
	// First two null-terminated blocks are pointers to strings
	for (int c = 0; c < 2; c++) {
	  for (; argv_ptr[i]; i++) {
	    load_convert_store(argv_ptr + i, argv_cap + i, -1);
	  }
	  argv_cap[i++] = 0;
	}
	int nargv_nenvp = i;

	/* Third block is
	 * |  type   |   val   |
	 * +---------+---------+
	 * | 8 bytes | 8 bytes |
	 * |   etc   |   etc   |
	 *
	 * We need val to be 16 bytes wide so it can store a
	 * capability, so convert to:
	 * |  type   |   pad   |        val      |
	 * +---------+---------+-----------------+
	 * | 8 bytes | 8 bytes |     16 bytes    |
	 */
	uintptr_t* auxv_cap = argv_cap + i;
	uint64_t* auxv_ptr = argv_ptr + i;
	char tmp[16];
	memset(tmp + 8, 0, 8);
	i = 0;
	for (;auxv_ptr[i]; i+= 2) {
	  memcpy(tmp, auxv_ptr + i, 8);
	  memcpy(auxv_cap + i, tmp, 16);
	  int len = -1;
	  switch((long)auxv_ptr[i]) {
	  case AT_EXECFN:
	  case AT_PHDR:
	    len = auxv_ptr[AT_PHNUM * 2 + 1] * auxv_ptr[AT_PHENT * 2 + 1];
	  case AT_ENTRY:
	  case AT_PLATFORM:
	  case AT_BASE_PLATFORM:
	  case AT_RANDOM:
	    load_convert_store(auxv_ptr + i + 1, auxv_cap + i + 1, len);
	    break;
	  default:
	    auxv_cap[i + 1] = auxv_ptr[i + 1];
	    break;
	  }
	}
	auxv_cap[i] = 0;
	auxv_cap[i + 1] = 0;
        // Tighten bounds on argv to the end of auxv - we will set bounds on the
        // sub-vectors (e.g. argv, envp) later
        __asm__ volatile ("scbnds %0, %0, %1"
                          : "+C"(argv_cap)
                          : "r"((nargv_nenvp + i) * sizeof(uintptr_t)));
}

#endif // MORELLO
