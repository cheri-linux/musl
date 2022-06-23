.global sigsetjmp
.global __sigsetjmp
.type sigsetjmp, %function
.type __sigsetjmp, %function
sigsetjmp:
__sigsetjmp:
#ifndef __CHERI_PURE_CAPABILITY__
	bnez a1, 1f
	tail setjmp
1:

	sd ra, 208(a0)
	sd s0, 216(a0)
	mv s0, a0

	call setjmp

	mv a1, a0
	mv a0, s0
	ld s0, 216(a0)
	ld ra, 208(a0)

.hidden __sigsetjmp_tail
	tail __sigsetjmp_tail
#else
	bnez a1, 1f
	clgc ct0, setjmp
	cjr ct0
1:

	csc cra, ((14 * 16) + (12 * 8) + 0)(ca0)
	csc cs0, ((14 * 16) + (12 * 8) + 16)(ca0)
	cmove cs0, ca0

	clgc ct0, setjmp
	cjalr ct0

	cmove ca1, ca0
	cmove ca0, cs0
	clc cs0, ((14 * 16) + (12 * 8) + 16)(ca0)
	clc cra, ((14 * 16) + (12 * 8) + 0)(ca0)

.hidden __sigsetjmp_tail
	clgc ct0, __sigsetjmp_tail
	cjr ct0
#endif
