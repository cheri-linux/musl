#define _BSD_SOURCE
#include <string.h>

#ifndef __CHERI_PURE_CAPABILITY__
#define PTRC	"r"
#else
#define PTRC	"C"
#endif

void explicit_bzero(void *d, size_t n)
{
	d = memset(d, 0, n);
	__asm__ __volatile__ ("" : : PTRC(d) : "memory");
}
