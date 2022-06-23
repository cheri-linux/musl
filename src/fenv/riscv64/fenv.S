#ifdef __riscv_flen

#ifndef __CHERI_PURE_CAPABILITY__
#define RET	ret
#else
#define RET	cret
#endif

.global feclearexcept
.type feclearexcept, %function
feclearexcept:
	csrc fflags, a0
	li a0, 0
	RET

.global feraiseexcept
.type feraiseexcept, %function
feraiseexcept:
	csrs fflags, a0
	li a0, 0
	RET

.global fetestexcept
.type fetestexcept, %function
fetestexcept:
	frflags t0
	and a0, t0, a0
	RET

.global fegetround
.type fegetround, %function
fegetround:
	frrm a0
	RET

.global __fesetround
.type __fesetround, %function
__fesetround:
	fsrm t0, a0
	li a0, 0
	RET

.global fegetenv
.type fegetenv, %function
fegetenv:
	frcsr t0
#ifndef __CHERI_PURE_CAPABILITY__
	sw t0, 0(a0)
#else
	csw t0, 0(ca0)
#endif
	li a0, 0
	RET

.global fesetenv
.type fesetenv, %function
fesetenv:
	li t2, -1
	li t1, 0
	beq a0, t2, 1f
#ifndef __CHERI_PURE_CAPABILITY__
	lw t1, 0(a0)
#else
	clw t1, 0(ca0)
#endif
1:	fscsr t1
	li a0, 0
	RET

#endif
