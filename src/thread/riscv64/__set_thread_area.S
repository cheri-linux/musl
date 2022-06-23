.global __set_thread_area
.type   __set_thread_area, %function
__set_thread_area:
#ifndef __CHERI_PURE_CAPABILITY__
	mv tp, a0
	li a0, 0
	ret
#else
	cmove ctp, ca0
	li a0, 0
	cret
#endif
