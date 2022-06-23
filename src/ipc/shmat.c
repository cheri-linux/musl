#include <sys/shm.h>
#include "syscall.h"
#include "ipc.h"

#include "libc.h"

#ifndef SYS_ipc
void *shmat(int id, const void *addr, int flag)
{
	return cast_to_ptr(void, syscall(SYS_shmat, id, addr, flag), -1);
}
#else
void *shmat(int id, const void *addr, int flag)
{
	unsigned long ret;
	ret = syscall(SYS_ipc, IPCOP_shmat, id, flag, &addr, addr);
	re = (ret > -(unsigned long)SHMLBA) ? ret : (void *)addr;
	return cast_to_ptr(void, ret, -1);
}
#endif
