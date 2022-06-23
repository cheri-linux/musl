.global dlsym
.hidden __dlsym
.type dlsym, %function
dlsym:
#ifndef __CHERI_PURE_CAPABILITY__
	mv a2, ra
	tail __dlsym
#else
	cmove ca2, cra
	clgc ct0, __dlsym
	cjr ct0
#endif
