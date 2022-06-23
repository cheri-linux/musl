/*-
 * Copyright (c) 2014 Robert N. M. Watson
 * Copyright (c) 2017-2018 Alex Richardson
 * All rights reserved.
 *
 * Modified for MUSL:
 * Copyright (c) 2019-2020 Huawei Technologies
 * 	Dmitry Kasatkin <dmitry.kasatkin@huawei.com>
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract (FA8750-10-C-0237)
 * ("CTSRD"), as part of the DARPA CRASH research programme.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef __CHERI_PURE_CAPABILITY__

#include <stdint.h>
#include <stdbool.h>
#include <elf.h>
#include <string.h>

#if defined(__mips)
#ifndef SHARED
#define GETDATASYM(addr, sym) __asm__ __volatile__ ( \
	".option pic0\n\t" \
	"dla %0, " #sym "\n\t" \
	:"=r"(addr));
#else
#define GETDATASYM(addr, sym) __asm__ __volatile__ ( \
	".hidden " #sym "\n" \
	".set push \n" \
	".set noreorder \n" \
	".p2align 3 \n" \
	"	bal 1f \n" \
	"	move $at, $ra \n" \
	"	.gpdword . \n" \
	"	.gpdword " #sym " \n" \
	"1:	ld $gp, ($ra) \n" \
	"	dsubu $gp, $ra, $gp \n" \
	"	ld $ra, 8($ra) \n" \
	"	daddu %0, $ra, $gp \n" \
	"	move $ra, $at \n" \
	".set pop \n" \
	: "=r"(addr) : : "memory", "at", "ra", "gp")
#endif
#elif defined(__riscv)
#define GETDATASYM(addr, sym) ({\
	void * __capability tmp; \
	__asm__ __volatile__ ( \
		".hidden " #sym "\n" \
		"cllc %1, " #sym "\n\t" \
		"cgetaddr %0, %1\n\t" \
		:"=r"(addr), "=&C"(tmp)); \
	})
#else
#error Unknown architecture
#endif

#undef MIN
#undef MAX
#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MAX(a, b) ((a) >= (b) ? (a) : (b))

#if __SIZEOF_LONG__ == 8
typedef Elf64_Ehdr	Elf_Ehdr;
typedef Elf64_Phdr	Elf_Phdr;
typedef Elf64_Addr	Elf_Addr;
#else
typedef Elf32_Ehdr	Elf_Ehdr;
typedef Elf32_Phdr	Elf_Phdr;
typedef Elf32_Addr	Elf_Addr;
#endif

#define BREAK	__asm__ __volatile__ ("break")
#define __DECONST(type, var)	((type)(uintptr_t)(const void *)(var))

#pragma push_macro("weak")
#undef weak

#define	__hidden	__attribute__((__visibility__("hidden")))

#define CHERI_INIT_GLOBALS_GDC_ONLY
#include <cheri_init_globals.h>

//#define CRT_INIT_GLOBALS_STATIC
#define CRT_INIT_GLOBALS_STATIC static __always_inline

__attribute__((weak)) extern int _DYNAMIC __no_subobject_bounds;

#define PT_GNU_RELRO	0x6474e552

static __always_inline void
crt_init_globals_3(void *data_cap, const void *code_cap,
                     const void *rodata_cap, __start_params_t *caps) {
	struct capreloc *start_relocs;
	struct capreloc *stop_relocs;
#ifdef DYNLINK
	start_relocs = caps->start_cap_relocs;
	stop_relocs = caps->stop_cap_relocs;
#else
#ifndef __CHERI_CAPABILITY_TABLE__

	/* If we are not using the CHERI capability table we can just synthesize
	 * the capabilities for these using the GOT and $ddc */
	start_relocs = &__start___cap_relocs;
	stop_relocs = &__stop___cap_relocs;
#else
	__UINT64_TYPE__ start_addr, end_addr;
	GETDATASYM(start_addr, __start___cap_relocs);
	GETDATASYM(end_addr, __stop___cap_relocs);
	long relocs_size = end_addr - start_addr;
	/*
	 * Always get __cap_relocs relative to the initial $pcc. This should span
	 * rodata and rw data, too so we can access __cap_relocs, no matter where it
	 * was placed.
	 */
	start_relocs = cheri_address_or_offset_set(cheri_getpcc(), start_addr);
	start_relocs = cheri_csetbounds(start_relocs, relocs_size);
	/*
	 * Note: with imprecise capabilities start_relocs could have a non-zero offset
	 * so we must not use setoffset!
	 * TODO: use csetboundsexact and teach the linker to align __cap_relocs.
	 */
	stop_relocs =
		(struct capreloc *)((__UINTPTR_TYPE__)start_relocs + relocs_size);
#endif
#endif

#if !defined(__CHERI_CAPABILITY_TABLE__)
	bool can_set_code_bounds = 0; /* legacy ABI -> need large bounds on $pcc */
#elif __CHERI_CAPABILITY_TABLE__ == 3
	bool can_set_code_bounds = 0; /* pc-relative ABI -> need large bounds on $pcc */
#else
	bool can_set_code_bounds = 1; /* fn-desc/plt ABI -> tight bounds okay */
#endif
	/*
	 * We can assume that all relocations in the __cap_relocs section have already
	 * been processed so we don't need to add a relocation base address to the
	 * location of the capreloc.
	 */
	cheri_init_globals_impl(start_relocs, stop_relocs, data_cap, code_cap,
			rodata_cap, can_set_code_bounds, caps->base);
}

/* This is __always_inline since it is called before globals have been set up */
static __always_inline void
crt_init_globals(const Elf_Phdr *phdr, long phnum, __start_params_t *caps)
{
	const Elf_Phdr *phlimit = phdr + phnum;
	Elf_Addr text_start = (Elf_Addr)-1l;
	Elf_Addr text_end = 0;
	Elf_Addr readonly_start = (Elf_Addr)-1l;
	Elf_Addr readonly_end = 0;
	Elf_Addr writable_start = (Elf_Addr)-1l;
	Elf_Addr writable_end = 0;
	bool have_rodata_segment = false;
	bool have_text_segment = false;
	bool have_data_segment = false;
	void * __capability data_cap;
	const void * __capability code_cap;
	const void * __capability rodata_cap;

	if (caps->base)
		caps->base_cap = cast_to_ptr(void, caps->base, -1);

	/* Attempt to bound the data capability to only the writable segment */
	for (const Elf_Phdr *ph = phdr; ph < phlimit; ph++) {
		if (ph->p_type != PT_LOAD && ph->p_type != PT_GNU_RELRO) {
#if 0 //ndef SHARED
			/* Static binaries should not have a PT_DYNAMIC phdr */
			if (ph->p_type == PT_DYNAMIC) {
				__builtin_trap();
				break;
			}
#endif
			continue;
		}
		/*
		 * We found a PT_LOAD or PT_GNU_RELRO phdr. PT_GNU_RELRO will
		 * be a subset of a matching PT_LOAD but we need to add the
		 * range from PT_GNU_RELRO to the constant capability since
		 * __cap_relocs could have some constants pointing to the relro
		 * section. The phdr for the matching PT_LOAD has PF_R|PF_W so
		 * it would not be added to the readonly if we didn't also
		 * parse PT_GNU_RELRO.
		 */
		Elf_Addr seg_start = ph->p_vaddr + caps->base;
		Elf_Addr seg_end = seg_start + ph->p_memsz;
		if ((ph->p_flags & PF_X)) {
			/* text segment */
			have_text_segment = true;
			text_start = MIN(text_start, seg_start);
			text_end = MAX(text_end, seg_end);
		} else if ((ph->p_flags & PF_W)) {
			/* data segment */
			have_data_segment = true;
			writable_start = MIN(writable_start, seg_start);
			writable_end = MAX(writable_end, seg_end);
		} else {
			have_rodata_segment = true;
			/* read-only segment (not always present) */
			readonly_start = MIN(readonly_start, seg_start);
			readonly_end = MAX(readonly_end, seg_end);
		}
	}

	if (!have_text_segment) {
		/* No text segment??? Must be an error somewhere else. */
		__builtin_trap();
	}
	if (!have_rodata_segment) {
		/*
		 * Note: If we don't have a separate rodata segment we also
		 * need to include the text segment in the rodata cap. This is
		 * required since all constants will be part of the read/exec
		 * segment instead of a separate read-only one.
		 */
		readonly_start = text_start;
		readonly_end = text_end;
	}
	if (!have_data_segment) {
		/*
		 * There cannot be any capabilities to initialize if there
		 * is no data segment. Set all to NULL to catch errors.
		 *
		 * Note: RELRO segment will be part of a R/W PT_LOAD.
		 */
		code_cap = NULL;
		data_cap = NULL;
		rodata_cap = NULL;
	} else {
		/* Check that ranges are well-formed */
		if (writable_end < writable_start ||
		    readonly_end < readonly_start ||
		    text_end < text_start)
			__builtin_trap();

		/* Abort if text and writeable overlap: */
		if (MAX(writable_start, text_start) <
		    MIN(writable_end, text_end)) {
			/* TODO: should we allow a single RWX segment? */
			__builtin_trap();
		}

#ifdef __CHERI_PURE_CAPABILITY__
		data_cap = __DECONST(void *, phdr);
#else
		data_cap = cheri_getdefault();
#endif
		data_cap = cheri_clearperm(data_cap, CHERI_PERM_EXECUTE);

		code_cap = cheri_getpcc();
		rodata_cap = cheri_clearperm(data_cap,
		    CHERI_PERM_STORE | CHERI_PERM_STORE_CAP |
		    CHERI_PERM_STORE_LOCAL_CAP);

		data_cap = cheri_setaddress(data_cap, writable_start);
		rodata_cap = cheri_setaddress(rodata_cap, readonly_start);

		/* TODO: should we use exact setbounds? */
		data_cap =
		    cheri_csetbounds(data_cap, writable_end - writable_start);
		rodata_cap =
		    cheri_csetbounds(rodata_cap, readonly_end - readonly_start);

		if (!cheri_gettag(data_cap))
			__builtin_trap();
		if (!cheri_gettag(rodata_cap))
			__builtin_trap();
		if (!cheri_gettag(code_cap))
			__builtin_trap();
	}
	caps->data_cap = data_cap;
	caps->code_cap = code_cap;
	caps->rodata_cap = rodata_cap;
	crt_init_globals_3(data_cap, code_cap, rodata_cap, caps);
}

#pragma pop_macro("weak")

#endif
