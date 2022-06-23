.global __setjmp
.global _setjmp
.global setjmp
.type __setjmp, %function
.type _setjmp,  %function
.type setjmp,   %function
__setjmp:
_setjmp:
setjmp:
#ifndef __CHERI_PURE_CAPABILITY__
	sd s0,    0(a0)
	sd s1,    8(a0)
	sd s2,    16(a0)
	sd s3,    24(a0)
	sd s4,    32(a0)
	sd s5,    40(a0)
	sd s6,    48(a0)
	sd s7,    56(a0)
	sd s8,    64(a0)
	sd s9,    72(a0)
	sd s10,   80(a0)
	sd s11,   88(a0)
	sd sp,    96(a0)
	sd ra,    104(a0)

#ifndef __riscv_float_abi_soft
	fsd fs0,  112(a0)
	fsd fs1,  120(a0)
	fsd fs2,  128(a0)
	fsd fs3,  136(a0)
	fsd fs4,  144(a0)
	fsd fs5,  152(a0)
	fsd fs6,  160(a0)
	fsd fs7,  168(a0)
	fsd fs8,  176(a0)
	fsd fs9,  184(a0)
	fsd fs10, 192(a0)
	fsd fs11, 200(a0)
#endif

	li a0, 0
	ret
#else
	csc cs0,    (0*16)(ca0)
	csc cs1,    (1*16)(ca0)
	csc cs2,    (2*16)(ca0)
	csc cs3,    (3*16)(ca0)
	csc cs4,    (4*16)(ca0)
	csc cs5,    (5*16)(ca0)
	csc cs6,    (6*16)(ca0)
	csc cs7,    (7*16)(ca0)
	csc cs8,    (8*16)(ca0)
	csc cs9,    (9*16)(ca0)
	csc cs10,   (10*16)(ca0)
	csc cs11,   (11*16)(ca0)
	csc csp,    (12*16)(ca0)
	csc cra,    (13*16)(ca0)
	cincoffset ca0, ca0, (14 * 16)

#ifndef __riscv_float_abi_soft
	cfsd fs0,  (0*8)(ca0)
	cfsd fs1,  (1*8)(ca0)
	cfsd fs2,  (2*8)(ca0)
	cfsd fs3,  (3*8)(ca0)
	cfsd fs4,  (4*8)(ca0)
	cfsd fs5,  (5*8)(ca0)
	cfsd fs6,  (6*8)(ca0)
	cfsd fs7,  (7*8)(ca0)
	cfsd fs8,  (8*8)(ca0)
	cfsd fs9,  (9*8)(ca0)
	cfsd fs10, (10*8)(ca0)
	cfsd fs11, (11*8)(ca0)
	cincoffset ca0, ca0, (12 * 8)
#endif

	li a0, 0
	cret
#endif
