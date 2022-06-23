#include <features.h>
#include "libc.h"

#define START "_start"

#include "dynlink.h"
#include "crt_arch.h"
#include "cheri.h"

#define ALIGN_L(x, a)			__ALIGN_MASK_L(x, a - 1)
#define __ALIGN_MASK_L(x, mask)		(((x) + (mask)) & ~(mask))

int main(int,char **,char **);

weak void _init();
weak void _fini();

#ifdef __CHERI_PURE_CAPABILITY__

int __libc_start_main(int (*)(), __start_params_t *params);

void _start_c(size_t *sp)
{
	int argc, envc, auxc;
	size_t *argvp, *envp, *auxv;
	int i, phnum = 0;
	char *c;
	void *base_cap;
	const Elf_Phdr *phdr = NULL;

	base_cap = cheri_getdefault();

	__start_params_t params;
	params.sp = cheri_getaddress(sp);

	argc = *sp++;
	params.argc = argc;

	long sp_a = cheri_getaddress(sp);
	sp_a = ALIGN_L(sp_a, (long)16);
	sp = cheri_setaddress(sp, sp_a);

	argvp = sp;
	envp = argvp + argc*2 + 2;

	for (sp = envp, envc = 0; *sp; envc++, sp+=2) ;

	auxv = sp + 1;
	params.auxv = auxv;

	for (i = 0; auxv[i]; auxv += 2) {
		switch (auxv[i]) {
		case AT_PHDR:
			phdr = cheri_setaddress(base_cap, auxv[i+1]);
			break;
		case AT_PHNUM:
			phnum = auxv[i+1];
			break;
		}
	}

	auxc = i; // *2

	params.base = 0;


	char **argv_tmp = (char**)(uintptr_t)argvp;

	char *argv[argc + 1];
	for (i = 0; i < argc; i++) {
		argv[i] = *argv_tmp;
		argv_tmp++;
	}
	argv[i] = 0;

	char **envp_tmp = (char**)(uintptr_t)envp;

	char *env[envc + 1];
	for (i = 0; i < envc; i++) {
		env[i] = *envp_tmp;
		envp_tmp++;
	}
	env[i] = 0;

	params.argv = argv;
	params.envp = env;

	pr_debug("_start_c: sp=%#llx, auxv=%#llx\n", params.sp, (long)params.auxv);

	__libc_start_main(main, &params);
}

#else

int __libc_start_main(int (*)(), int, char **,
	void (*)(), void(*)(), void(*)());

void _start_c(long *p)
{
	int argc = p[0];
	char **argv = (void *)(p+1);
	__libc_start_main(main, argc, argv, _init, _fini, 0);
}
#endif
