#define START "_start"
#define _dlstart_c _start_c
#include "../ldso/dlstart.c"

int main();
weak void _init();
weak void _fini();

#ifdef __CHERI_PURE_CAPABILITY__

int __libc_start_main(int (*)(), int, char **, char **,
	void (*)(), void(*)(), void(*)());

hidden void __dls2(unsigned char *base, size_t *sp)
{
	int argc = sp[0];
	char **argv = (void *)(sp+1);
	char **envp = argv+argc+1;
	__libc_start_main(main, argc, argv, envp, _init, _fini, 0);
}

#else

int __libc_start_main(int (*)(), int, char **,
	void (*)(), void(*)(), void(*)());

hidden void __dls2(unsigned char *base, size_t *sp)
{
	__libc_start_main(main, *sp, (void *)(sp+1), _init, _fini, 0);
}

#endif
