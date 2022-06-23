#if defined __riscv_float_abi_soft
#define RISCV_FP_SUFFIX "-sf"
#elif defined __riscv_float_abi_single
#define RISCV_FP_SUFFIX "-sp"
#elif defined __riscv_float_abi_double
#define RISCV_FP_SUFFIX ""
#endif

#define LDSO_ARCH "riscv64" RISCV_FP_SUFFIX

#define TPOFF_K 0

#define REL_SYMBOLIC    R_RISCV_64
#define REL_PLT         R_RISCV_JUMP_SLOT
#define REL_RELATIVE    R_RISCV_RELATIVE
#define REL_COPY        R_RISCV_COPY
#define REL_DTPMOD      R_RISCV_TLS_DTPMOD64
#define REL_DTPOFF      R_RISCV_TLS_DTPREL64
#define REL_TPOFF       R_RISCV_TLS_TPREL64

#ifdef __CHERI_PURE_CAPABILITY__

#define CRTJMP(pc,sp) __asm__ __volatile__( \
	"csetaddr csp, csp, %1\n\t" \
	"cspecialr ct0, pcc\n\t" \
	"csetaddr ct0, ct0, %0\n\t" \
	"cjr ct0\n\t" \
	: : "r"(pc), "r"(sp) : "memory")

#else

#define CRTJMP(pc,sp) __asm__ __volatile__( \
	"mv sp, %1 ; jr %0" : : "r"(pc), "r"(sp) : "memory" )

#endif
