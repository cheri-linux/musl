#define _GNU_SOURCE
#include <sys/socket.h>
#include <limits.h>
#include <errno.h>
#include "syscall.h"

int sendmmsg(int fd, struct mmsghdr *msgvec, unsigned int vlen, unsigned int flags)
{
	return syscall_cp(SYS_sendmmsg, fd, msgvec, vlen, flags);
}
