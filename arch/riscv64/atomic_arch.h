#define a_barrier a_barrier
static inline void a_barrier()
{
	__asm__ __volatile__ ("fence rw,rw" : : : "memory");
}

#ifndef __CHERI_PURE_CAPABILITY__

#define a_cas a_cas
static inline int a_cas(volatile int *pc, int t, int s)
{
	register long p = (long)pc;
	int old, tmp;
	__asm__ __volatile__ (
		"\n1:	lr.w.aqrl %0, (%2)\n"
		"	bne %0, %3, 1f\n"
		"	sc.w.aqrl %1, %4, (%2)\n"
		"	bnez %1, 1b\n"
		"1:"
		: "=&r"(old), "=&r"(tmp)
		: "r"(p), "r"((long)t), "r"((long)s)
		: "memory");
	return old;
}

#define a_cas_p a_cas_p
static inline void *a_cas_p(volatile void *p, void *t, void *s)
{
	void *old;
	int tmp;
	__asm__ __volatile__ (
		"\n1:	lr.d.aqrl %0, (%2)\n"
		"	bne %0, %3, 1f\n"
		"	sc.d.aqrl %1, %4, (%2)\n"
		"	bnez %1, 1b\n"
		"1:"
		: "=&r"(old), "=&r"(tmp)
		: "r"(p), "r"(t), "r"(s)
		: "memory");
	return old;
}
#else // __CHERI_PURE_CAPABILITY__
#define a_cas a_cas
static inline int a_cas(volatile int *p, int t, int s)
{
	_Bool success = __atomic_compare_exchange_n(p, &t, s, 0,
			__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	(void)success;
	return t;
}
#define a_cas_p a_cas_p
static inline void *a_cas_p(volatile void *p, void *t, void *s)
{
	_Bool success = __atomic_compare_exchange_n((void**)p, &t, s, 0,
			__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	(void)success;
	return t;
}
#endif
