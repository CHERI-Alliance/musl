__asm__(
".text \n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
"	mov x29, #0\n"
"	mov x30, #0\n"
#ifndef SHARED
"	mov c20, c0\n"
"	mov c21, c1\n"
"	mov c22, c2\n"
"	mov c23, c3\n"
"	bl __morello_init_static\n"
#else
"	mov c5, c3\n"
"	mov c4, c2\n"
"	mov c3, c1\n"
"	mov c2, c0\n"
#endif
"	mov c0, csp\n"
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n"
"	adrp c1, _DYNAMIC\n"
"	add c1, c1, #:lo12:_DYNAMIC\n"
"	alignd csp, csp, #4\n"
#ifndef SHARED
"	mov c2, c20\n"
"	mov c3, c21\n"
"	mov c4, c22\n"
"	mov c5, c23\n"
#endif
"	b " START "_c\n"
".size " START ", .-" START "\n"
);

#ifndef SHARED

typedef struct {
	uint64_t location;	/* Capability location */
	uint64_t base;		/* Object referred to by the capability */
	size_t offset;		/* Offset in the object */
	size_t size;		/* Size */
	size_t permissions;	/* Inverted permissions mask */
} cap_relocs_entry;

__attribute__((used))
inline static void
__morello_init_static(int argc, char **argv, char **envp, auxv_entry *auxv)
{
	cap_relocs_entry *__cap_relocs_start = NULL;
	cap_relocs_entry *__cap_relocs_end = NULL;
	__asm__ (
		".weak __cap_relocs_start\n"
		"adrp 	%0, __cap_relocs_start\n"
		"add	%0, %0, :lo12:__cap_relocs_start" : "=C"(__cap_relocs_start));
	__asm__ (
		".weak __cap_relocs_end\n"
		"adrp 	%0, __cap_relocs_end\n"
		"add	%0, %0, :lo12:__cap_relocs_end" : "=C"(__cap_relocs_end));
	size_t n = __cap_relocs_end - __cap_relocs_start; /* number of cap_relocs entries */
	if (n == 0 || __cap_relocs_start == NULL || __cap_relocs_end == NULL) {
		return;
	}
	cap_relocs_entry *r = __cap_relocs_start;
	void *rw = NULL;
	void *rx = NULL;
	for (; auxv->a_type; auxv++) {
		if (auxv->a_type == AT_CHERI_EXEC_RW_CAP) {
			rw = auxv->a_un.a_ptr; // used to derive read-only and rw objects
		} else if (auxv->a_type == AT_CHERI_EXEC_RX_CAP) {
			rx = auxv->a_un.a_ptr; // used to derive function pointers
		}
		if (rw && rx) {
			break;
		}
	}
	for(size_t k = 0; k < n; k++) {
		if (r[k].base) { // if capability is not null
			void *cap = NULL;
			size_t perm = ~r[k].permissions;
			_Bool is_fun_ptr = perm & __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__;
			_Bool is_writable = perm & __CHERI_CAP_PERMISSION_PERMIT_STORE__;
			if (is_writable) {
				cap = __builtin_cheri_address_set(rw, r[k].base);
			} else {
				cap = __builtin_cheri_address_set(rx, r[k].base);
			}
			cap = __builtin_cheri_perms_and(cap, perm);
			cap = __builtin_cheri_bounds_set_exact(cap, r[k].size);
			cap = __builtin_cheri_offset_set(cap, r[k].offset);
			if (is_fun_ptr) {
				// RB-seal function pointer
				cap = __builtin_cheri_seal_entry(cap);
			}
			// store capability
			void **loc = __builtin_cheri_address_set(rw, r[k].location);
			loc = __builtin_cheri_bounds_set_exact(loc, sizeof(void *));
			*loc = cap;
		}
	}
}
#endif
