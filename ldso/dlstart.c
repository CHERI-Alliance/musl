#include <stddef.h>
#include <sys/dynv.h>
#include "dynlink.h"
#include "libc.h"

#ifndef START
#define START "_dlstart"
#endif

#define SHARED

#include "crt_arch.h"

#ifndef GETFUNCSYM
#define GETFUNCSYM(fp, sym, got) do { \
	hidden void sym(); \
	static void (*static_func_ptr)() = sym; \
	__asm__ __volatile__ ( "" : "+m"(static_func_ptr) : : "memory"); \
	*(fp) = static_func_ptr; } while(0)
#endif

#define AUX_TYPE(p) ((p)->a_type)
#define AUX_VAL(p) ((p)->a_un.a_val)
#ifdef MORELLO
#define AUX_PTR(p) ((p)->a_un.a_ptr)
#else
#define AUX_PTR(p) ((p)->a_un.a_val)
#endif
#define DYN_VAL(p) (p.d_un.d_val)
#define DYN_PTR(p) (p.d_un.d_ptr)
#define DYN_TAG(p) ((p).d_tag)
#define DYN_VAL(p) ((p).d_un.d_val)
#define DYN_PTR(p) ((p).d_un.d_ptr)

hidden void _dlstart_c(size_t *sp, size_t *dynv_raw)
{
	size_t i;
	auxv_entry aux_null = {0}, *aux[AUX_CNT];
	dynv_entry dyn[DYN_CNT] = {{0}};
	size_t *rel, rel_size, base;

	int argc = *sp;
	char **argv = (void *)(sp+1);

	for (i=argc+1; argv[i]; i++);
	auxv_entry *auxv = (void *)(argv+i+1);
	dynv_entry *dynv = (void *)dynv_raw;

	for (i=0; i<AUX_CNT; i++) aux[i] = &aux_null;
	for (i=0; auxv[i].a_type; i++) if (auxv[i].a_type<AUX_CNT)
		aux[auxv[i].a_type] = auxv + i;

#if DL_FDPIC
	struct fdpic_loadseg *segs, fakeseg;
	size_t j;
	if (dynv) {
		/* crt_arch.h entry point asm is responsible for reserving
		 * space and moving the extra fdpic arguments to the stack
		 * vector where they are easily accessible from C. */
		segs = ((struct fdpic_loadmap *)(sp[-1] ? sp[-1] : sp[-2]))->segs;
	} else {
		/* If dynv is null, the entry point was started from loader
		 * that is not fdpic-aware. We can assume normal fixed-
		 * displacement ELF loading was performed, but when ldso was
		 * run as a command, finding the Ehdr is a heursitic: we
		 * have to assume Phdrs start in the first 4k of the file. */
		base = AUX_PTR(aux[AT_BASE]);
		if (!base) base = __builtin_align_down(AUX_PTR(aux[AT_PHDR]), 4096);
		segs = &fakeseg;
		segs[0].addr = base;
		segs[0].p_vaddr = 0;
		segs[0].p_memsz = -1;
		Ehdr *eh = (void *)base;
		Phdr *ph = (void *)(base + eh->e_phoff);
		size_t phnum = eh->e_phnum;
		size_t phent = eh->e_phentsize;
		while (phnum-- && ph->p_type != PT_DYNAMIC)
			ph = (void *)((size_t)ph + phent);
		dynv = (void *)(base + ph->p_vaddr);
	}
#endif

	for (i=0; dynv[i].d_tag; i++) if (dynv[i].d_tag<DYN_CNT)
		dyn[dynv[i].d_tag] = dynv[i];

#if DL_FDPIC
	for (i=0; i<DYN_CNT; i++) {
		if (i==DT_RELASZ || i==DT_RELSZ) continue;
		if (!dyn[i].d_tag) continue;
		for (j=0; DYN_VAL(dyn[i])-segs[j].p_vaddr >= segs[j].p_memsz; j++);
		DYN_VAL(dyn[i]) += segs[j].addr - segs[j].p_vaddr;
	}
	base = 0;

	const Sym *syms = DYN_PTR(dyn[DT_SYMTAB]);

	rel = DYN_PTR(dyn[DT_RELA]);
	rel_size = DYN_VAL(dyn[DT_RELASZ]);
	for (; rel_size; rel+=3, rel_size-=3*sizeof(size_t)) {
		if (!IS_RELATIVE(rel[1], syms)) continue;
		for (j=0; rel[0]-segs[j].p_vaddr >= segs[j].p_memsz; j++);
		size_t *rel_addr = (void *)
			(rel[0] + segs[j].addr - segs[j].p_vaddr);
		if (R_TYPE(rel[1]) == REL_FUNCDESC_VAL) {
			*rel_addr += segs[rel_addr[1]].addr
				- segs[rel_addr[1]].p_vaddr
				+ syms[R_SYM(rel[1])].st_value;
			rel_addr[1] = DYN_PTR(dyn[DT_PLTGOT]);
		} else {
			size_t val = syms[R_SYM(rel[1])].st_value;
			for (j=0; val-segs[j].p_vaddr >= segs[j].p_memsz; j++);
			*rel_addptr = rel[2] + segs[j].addr - segs[j].p_vaddr + val;
		}
	}
#else
	/* If the dynamic linker is invoked as a command, its load
	 * address is not available in the aux vector. Instead, compute
	 * the load address as the difference between &_DYNAMIC and the
	 * virtual address in the PT_DYNAMIC program header. */
	base = AUX_PTR(aux[AT_BASE]);
	if (!base) {
		size_t phnum = AUX_VAL(aux[AT_PHNUM]);
		size_t phentsize = AUX_VAL(aux[AT_PHENT]);
		Phdr *ph = AUX_PTR(aux[AT_PHDR]);
		for (i=phnum; i--; ph = (void *)((char *)ph + phentsize)) {
			if (ph->p_type == PT_DYNAMIC) {
				base = dynv - ph->p_vaddr;
				break;
			}
		}
	}

	/* MIPS uses an ugly packed form for GOT relocations. Since we
	 * can't make function calls yet and the code is tiny anyway,
	 * it's simply inlined here. */
	if (NEED_MIPS_GOT_RELOCS) {
		size_t local_cnt = 0;
		size_t *got = base + DYN_VAL(dyn[DT_PLTGOT]);
		for (i=0; dynv[i].d_tag; i++) if (dynv[i].d_tag==DT_MIPS_LOCAL_GOTNO)
			local_cnt = dynv[i].d_tag;
		for (i=0; i<local_cnt; i++) got[i] += base;
	}

	rel = base+DYN_PTR(dyn[DT_REL]);
	rel_size = DYN_VAL(dyn[DT_RELSZ]);
	for (; rel_size; rel+=2, rel_size-=2*sizeof(size_t)) {
		if (!IS_RELATIVE(rel[1], 0)) continue;
		size_t *rel_addr = base + rel[0];
		*rel_addr += base;
	}

	rel = base+DYN_PTR(dyn[DT_RELA]);
	rel_size = DYN_PTR(dyn[DT_RELASZ]);
	for (; rel_size; rel+=3, rel_size-=3*sizeof(size_t)) {
		if (!IS_RELATIVE(rel[1], 0)) continue;
		size_t *rel_addr = base + rel[0];
		*rel_addr = base + rel[2];
	}
#endif

	stage2_func dls2;
	GETFUNCSYM(&dls2, __dls2, DYN_VAL(base+dyn[DT_PLTGOT]));
	dls2((void *)base, sp);
}
