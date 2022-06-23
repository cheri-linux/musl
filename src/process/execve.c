#include <unistd.h>
#include "syscall.h"

#if defined(__CHERI_PURE_CAPABILITY__) && !defined(SYSCALL_PURECAP)
static int count(char *const *arg)
{
	int i;

	for (i = 0; *arg; i++, arg++) ;

	return i;
}

int execve(const char *path, char *const argv[], char *const envp[])
{
	int i;
	long kargv[count(argv) + 1];
	long kenvp[count(envp) + 1];

	for (i = 0; argv[i]; i++)
		kargv[i] = (long)argv[i];
	kargv[i] = 0;
	for (i = 0; envp[i]; i++)
		kenvp[i] = (long)envp[i];
	kenvp[i] = 0;

	/* do we need to use environ if envp is null? */
	return syscall(SYS_execve, path, kargv, kenvp);
}
#else
int execve(const char *path, char *const argv[], char *const envp[])
{
	/* do we need to use environ if envp is null? */
	return syscall(SYS_execve, path, argv, envp);
}

#endif
