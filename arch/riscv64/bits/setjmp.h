#ifdef __CHERI_PURE_CAPABILITY__
#define __NEED_uintptr_t
#include <bits/alltypes.h>
typedef struct {
	uintptr_t cap1[14];
	long fp[12];
	uintptr_t cap2[2];
} __jmp_buf;
#else
typedef long __jmp_buf[28];
#endif
