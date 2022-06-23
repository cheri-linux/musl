.global __longjmp
.global _longjmp
.global longjmp
.type __longjmp, %function
.type _longjmp,  %function
.type longjmp,   %function
__longjmp:
_longjmp:
longjmp:
#ifndef __CHERI_PURE_CAPABILITY__
	ld s0,    0(a0)
	ld s1,    8(a0)
	ld s2,    16(a0)
	ld s3,    24(a0)
	ld s4,    32(a0)
	ld s5,    40(a0)
	ld s6,    48(a0)
	ld s7,    56(a0)
	ld s8,    64(a0)
	ld s9,    72(a0)
	ld s10,   80(a0)
	ld s11,   88(a0)
	ld sp,    96(a0)
	ld ra,    104(a0)

#ifndef __riscv_float_abi_soft
	fld fs0,  112(a0)
	fld fs1,  120(a0)
	fld fs2,  128(a0)
	fld fs3,  136(a0)
	fld fs4,  144(a0)
	fld fs5,  152(a0)
	fld fs6,  160(a0)
	fld fs7,  168(a0)
	fld fs8,  176(a0)
	fld fs9,  184(a0)
	fld fs10, 192(a0)
	fld fs11, 200(a0)
#endif

	seqz a0, a1
	add a0, a0, a1
	ret
#else
	clc cs0,    (0*16)(ca0)
	clc cs1,    (1*16)(ca0)
	clc cs2,    (2*16)(ca0)
	clc cs3,    (3*16)(ca0)
	clc cs4,    (4*16)(ca0)
	clc cs5,    (5*16)(ca0)
	clc cs6,    (6*16)(ca0)
	clc cs7,    (7*16)(ca0)
	clc cs8,    (8*16)(ca0)
	clc cs9,    (9*16)(ca0)
	clc cs10,   (10*16)(ca0)
	clc cs11,   (11*16)(ca0)
	clc csp,    (12*16)(ca0)
	clc cra,    (13*16)(ca0)
	cincoffset ca0, ca0, (14 * 16)

#ifndef __riscv_float_abi_soft
	cfld fs0,  (0*8)(ca0)
	cfld fs1,  (1*8)(ca0)
	cfld fs2,  (2*8)(ca0)
	cfld fs3,  (3*8)(ca0)
	cfld fs4,  (4*8)(ca0)
	cfld fs5,  (5*8)(ca0)
	cfld fs6,  (6*8)(ca0)
	cfld fs7,  (7*8)(ca0)
	cfld fs8,  (8*8)(ca0)
	cfld fs9,  (9*8)(ca0)
	cfld fs10, (10*8)(ca0)
	cfld fs11, (11*8)(ca0)
	cincoffset ca0, ca0, (12 * 8)
#endif

	seqz a0, a1
	add a0, a0, a1
	cret
#endif
