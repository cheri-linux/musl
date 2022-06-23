#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

#ifndef SYSCALL_PURECAP
#define __CREG
#else
#define __CREG	"c"
#endif

#define __asm_syscall(...) \
	__asm__ __volatile__ ("ecall\n\t" \
	: "=r"(a0) : __VA_ARGS__ : "memory"); \
	return a0; \

static inline long __syscall0(long n)
{
	register long a7 __asm__("a7") = n;
	register syscall_arg_t a0 __asm__(__CREG"a0");
	__asm_syscall("r"(a7))
}

static inline long __syscall1(long n, syscall_arg_t a)
{
	register long a7 __asm__("a7") = n;
	register syscall_arg_t a0 __asm__(__CREG"a0") = a;
	__asm_syscall("r"(a7), "0"(a0))
}

static inline long __syscall2(long n, syscall_arg_t a, syscall_arg_t b)
{
	register long a7 __asm__("a7") = n;
	register syscall_arg_t a0 __asm__(__CREG"a0") = a;
	register syscall_arg_t a1 __asm__(__CREG"a1") = b;
	__asm_syscall("r"(a7), "0"(a0), "r"(a1))
}

static inline long __syscall3(long n, syscall_arg_t a, syscall_arg_t b, syscall_arg_t c)
{
	register long a7 __asm__("a7") = n;
	register syscall_arg_t a0 __asm__(__CREG"a0") = a;
	register syscall_arg_t a1 __asm__(__CREG"a1") = b;
	register syscall_arg_t a2 __asm__(__CREG"a2") = c;
	__asm_syscall("r"(a7), "0"(a0), "r"(a1), "r"(a2))
}

static inline long __syscall4(long n, syscall_arg_t a, syscall_arg_t b, syscall_arg_t c, syscall_arg_t d)
{
	register long a7 __asm__("a7") = n;
	register syscall_arg_t a0 __asm__(__CREG"a0") = a;
	register syscall_arg_t a1 __asm__(__CREG"a1") = b;
	register syscall_arg_t a2 __asm__(__CREG"a2") = c;
	register syscall_arg_t a3 __asm__(__CREG"a3") = d;
	__asm_syscall("r"(a7), "0"(a0), "r"(a1), "r"(a2), "r"(a3))
}

static inline long __syscall5(long n, syscall_arg_t a, syscall_arg_t b, syscall_arg_t c, syscall_arg_t d, syscall_arg_t e)
{
	register long a7 __asm__("a7") = n;
	register syscall_arg_t a0 __asm__(__CREG"a0") = a;
	register syscall_arg_t a1 __asm__(__CREG"a1") = b;
	register syscall_arg_t a2 __asm__(__CREG"a2") = c;
	register syscall_arg_t a3 __asm__(__CREG"a3") = d;
	register syscall_arg_t a4 __asm__(__CREG"a4") = e;
	__asm_syscall("r"(a7), "0"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4))
}

static inline long __syscall6(long n, syscall_arg_t a, syscall_arg_t b, syscall_arg_t c, syscall_arg_t d, syscall_arg_t e, syscall_arg_t f)
{
	register long a7 __asm__("a7") = n;
	register syscall_arg_t a0 __asm__(__CREG"a0") = a;
	register syscall_arg_t a1 __asm__(__CREG"a1") = b;
	register syscall_arg_t a2 __asm__(__CREG"a2") = c;
	register syscall_arg_t a3 __asm__(__CREG"a3") = d;
	register syscall_arg_t a4 __asm__(__CREG"a4") = e;
	register syscall_arg_t a5 __asm__(__CREG"a5") = f;
	__asm_syscall("r"(a7), "0"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5))
}

#define VDSO_USEFUL
/* We don't have a clock_gettime function.
#define VDSO_CGT_SYM "__vdso_clock_gettime"
#define VDSO_CGT_VER "LINUX_2.6" */

#define IPC_64 0
