#define _GNU_SOURCE
#include <stddef.h>
#include <endian.h>

#include "dynlink.h"
#include "libc.h"

#ifndef START
#define START "_dlstart"
#endif

#define SHARED

#include "crt_arch.h"
#include "cheri.h"

#ifndef GETFUNCSYM
#define GETFUNCSYM(fp, sym, got) do { \
	hidden void sym(); \
	static void (*static_func_ptr)() = sym; \
	__asm__ __volatile__ ( "" : "+m"(static_func_ptr) : : "memory"); \
	*(fp) = static_func_ptr; } while(0)
#endif

#define ALIGN_L(x, a)			__ALIGN_MASK_L(x, a - 1)
#define __ALIGN_MASK_L(x, mask)		(((x) + (mask)) & ~(mask))

#include "syscall.h"

ssize_t __write(int fd, const void *buf, size_t count)
{
	return __syscall(SYS_write, fd, buf, count);
}

size_t strlen(const char *s);

#ifndef __CHERI_PURE_CAPABILITY__

hidden void _dlstart_c(size_t *sp, size_t *dynv)
{
	size_t i, aux[AUX_CNT], dyn[DYN_CNT];
	size_t *rel, rel_size, base;

	int argc = *sp;
	char **argv = (void *)(sp+1);

	for (i=argc+1; argv[i]; i++);
	size_t *auxv = (void *)(argv+i+1);

	for (i=0; i<AUX_CNT; i++) aux[i] = 0;
	for (i=0; auxv[i]; i+=2) if (auxv[i]<AUX_CNT)
		aux[auxv[i]] = auxv[i+1];

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
		base = aux[AT_BASE];
		if (!base) base = aux[AT_PHDR] & -4096;
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

	for (i=0; i<DYN_CNT; i++) dyn[i] = 0;
	for (i=0; dynv[i]; i+=2) if (dynv[i]<DYN_CNT)
		dyn[dynv[i]] = dynv[i+1];

#if DL_FDPIC
	for (i=0; i<DYN_CNT; i++) {
		if (i==DT_RELASZ || i==DT_RELSZ) continue;
		if (!dyn[i]) continue;
		for (j=0; dyn[i]-segs[j].p_vaddr >= segs[j].p_memsz; j++);
		dyn[i] += segs[j].addr - segs[j].p_vaddr;
	}
	base = 0;

	const Sym *syms = (void *)dyn[DT_SYMTAB];

	rel = (void *)dyn[DT_RELA];
	rel_size = dyn[DT_RELASZ];
	for (; rel_size; rel+=3, rel_size-=3*sizeof(size_t)) {
		if (!IS_RELATIVE(rel[1], syms)) continue;
		for (j=0; rel[0]-segs[j].p_vaddr >= segs[j].p_memsz; j++);
		size_t *rel_addr = (void *)
			(rel[0] + segs[j].addr - segs[j].p_vaddr);
		if (R_TYPE(rel[1]) == REL_FUNCDESC_VAL) {
			*rel_addr += segs[rel_addr[1]].addr
				- segs[rel_addr[1]].p_vaddr
				+ syms[R_SYM(rel[1])].st_value;
			rel_addr[1] = dyn[DT_PLTGOT];
		} else {
			size_t val = syms[R_SYM(rel[1])].st_value;
			for (j=0; val-segs[j].p_vaddr >= segs[j].p_memsz; j++);
			*rel_addr = rel[2] + segs[j].addr - segs[j].p_vaddr + val;
		}
	}
#else
	/* If the dynamic linker is invoked as a command, its load
	 * address is not available in the aux vector. Instead, compute
	 * the load address as the difference between &_DYNAMIC and the
	 * virtual address in the PT_DYNAMIC program header. */
	base = aux[AT_BASE];
	if (!base) {
		size_t phnum = aux[AT_PHNUM];
		size_t phentsize = aux[AT_PHENT];
		Phdr *ph = (void *)aux[AT_PHDR];
		for (i=phnum; i--; ph = (void *)((char *)ph + phentsize)) {
			if (ph->p_type == PT_DYNAMIC) {
				base = (size_t)dynv - ph->p_vaddr;
				break;
			}
		}
	}

	/* MIPS uses an ugly packed form for GOT relocations. Since we
	 * can't make function calls yet and the code is tiny anyway,
	 * it's simply inlined here. */
	if (NEED_MIPS_GOT_RELOCS) {
		size_t local_cnt = 0;
		size_t *got = (void *)(base + dyn[DT_PLTGOT]);
		for (i=0; dynv[i]; i+=2) if (dynv[i]==DT_MIPS_LOCAL_GOTNO)
			local_cnt = dynv[i+1];
		for (i=0; i<local_cnt; i++) got[i] += base;
	}

	rel = (void *)(base+dyn[DT_REL]);
	rel_size = dyn[DT_RELSZ];
	for (; rel_size; rel+=2, rel_size-=2*sizeof(size_t)) {
		if (!IS_RELATIVE(rel[1], 0)) continue;
		size_t *rel_addr = (void *)(base + rel[0]);
		*rel_addr += base;
	}

	rel = (void *)(base+dyn[DT_RELA]);
	rel_size = dyn[DT_RELASZ];
	for (; rel_size; rel+=3, rel_size-=3*sizeof(size_t)) {
		if (!IS_RELATIVE(rel[1], 0)) continue;
		size_t *rel_addr = (void *)(base + rel[0]);
		*rel_addr = base + rel[2];
	}
#endif

	stage2_func dls2;
	GETFUNCSYM(&dls2, __dls2, base+dyn[DT_PLTGOT]);
	dls2((void *)base, sp);
}

#else // __CHERI_PURE_CAPABILITY__

hidden void _dlstart_c(size_t *sp, long dynvp)
{
	size_t *sporig, *dynv;
	size_t i, aux[AUX_CNT], dyn[DYN_CNT];
	size_t *rel, rel_size, base;
	int argc, envc;
	size_t *argvp, *envp, *auxv;
	int phnum = 0;
	char *c;
	void *default_cap;
	const Phdr *phdr = NULL;

	// FIXME: mlock memory for gdb access
	// gdb8 cannot set break points if memory is not accessible
	//__syscall(SYS_mlockall, 3);

	default_cap = cheri_getdefault();

	sporig = sp;
	argc = *sp++;
	long sp_a = cheri_getaddress(sp);
	sp_a = ALIGN_L(sp_a, (long)16);
	sp = cheri_setaddress(sp, sp_a);
	argvp = sp;
	envp = argvp + argc*2 + 2;
	for (sp = envp, envc = 0; *sp; envc++, sp += 2) ;

	auxv = sp + 1;

	for (i = 0; i < AUX_CNT; i++)
		aux[i] = 0;

	for (i = 0; auxv[i]; i += 2) {
		if (auxv[i] < AUX_CNT)
			aux[auxv[i]] = auxv[i+1];
	}

	for (i = 0; i < DYN_CNT; i++)
		dyn[i] = 0;

	dynv = cheri_setaddress(default_cap, dynvp);

	for (i = 0; dynv[i]; i += 2) {
		if (dynv[i] < DYN_CNT)
			dyn[dynv[i]] = dynv[i+1];
	}

	/* If the dynamic linker is invoked as a command, its load
	 * address is not available in the aux vector. Instead, compute
	 * the load address as the difference between &_DYNAMIC and the
	 * virtual address in the PT_DYNAMIC program header. */
	base = aux[AT_BASE];
	if (!base) {
		size_t phentsize = aux[AT_PHENT];
		phdr = cheri_setaddress(default_cap, aux[AT_PHDR]);
		phnum = aux[AT_PHNUM];
		const Phdr *ph = phdr;
		for (i = phnum; i--; ph = (void *)((char *)ph + phentsize)) {
			if (ph->p_type == PT_DYNAMIC) {
				base = dynvp - ph->p_vaddr;
				break;
			}
		}
	} else {
		Elf_Ehdr *eh = cheri_setaddress(default_cap, base);
		phdr = cheri_setaddress(default_cap, base + eh->e_phoff);
		phnum = eh->e_phnum;
	}

	/* MIPS uses an ugly packed form for GOT relocations. Since we
	 * can't make function calls yet and the code is tiny anyway,
	 * it's simply inlined here. */
	if (NEED_MIPS_GOT_RELOCS) {
		size_t local_cnt = 0;
		for (i = 0; dynv[i]; i += 2) {
			if (dynv[i] == DT_MIPS_LOCAL_GOTNO) {
				local_cnt = dynv[i+1];
				break;
			}
		}
		size_t *got = cast_to_ptr(size_t, base + dyn[DT_PLTGOT], local_cnt * sizeof(long));
		for (i = 0; i < local_cnt; i++)
			got[i] += base;
	}

	rel_size = dyn[DT_RELSZ];
	rel = cast_to_ptr(size_t, base + dyn[DT_REL], rel_size);
	for (; rel_size; rel += 2, rel_size -= 2 * sizeof(size_t)) {
		if (!IS_RELATIVE(rel[1], 0))
			continue;
		size_t *rel_addr = cast_to_ptr(size_t, base + rel[0], sizeof(size_t));
		*rel_addr += base;
	}

	rel_size = dyn[DT_RELASZ];
	rel = cast_to_ptr(size_t, base + dyn[DT_RELA], rel_size);
	for (; rel_size; rel += 3, rel_size -= 3 * sizeof(size_t)) {
		if (!IS_RELATIVE(rel[1], 0))
			continue;
		size_t *rel_addr = cast_to_ptr(size_t, base + rel[0], sizeof(size_t));
		*rel_addr = base + rel[2];
	}

	__start_params_t params;

	params.sp = cheri_getaddress(sporig);
	params.argc = argc;
	params.auxv = auxv;
	params.base = base;

	crt_init_globals(phdr, phnum, &params);

	char *argv[argc + 1];

	char **argv_tmp = (char**)(uintptr_t)argvp;
	for (i = 0; i < argc; i++) {
		argv[i] = *argv_tmp;
		argv_tmp++;
	}
	argv[i] = 0;

	char *env[envc + 1];

	char **envp_tmp = (char**)(uintptr_t)envp;
	for (i = 0; i < envc; i++) {
		env[i] = *envp_tmp;
		envp_tmp++;
	}
	env[i] = 0;

	params.argv = argv;
	params.envp = env;


	stage2_func dls2;
	GETFUNCSYM(&dls2, __dls2, base+dyn[DT_PLTGOT]);
	dls2(params.base_cap, &params);
}

#endif
