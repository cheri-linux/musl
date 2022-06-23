#ifndef LIBC_H
#define LIBC_H

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#ifdef __CHERI_PURE_CAPABILITY__
#include "cheric.h"
#define cast_to_ptr(type, ptr, len)	({ (type *)cheri_long(ptr, -1); })
#else
#define cast_to_ptr(type, ptr, len)	({ (type *)(ptr); })
#endif

typedef struct {
	size_t sp;
	int argc;
	char **argv;
	char **envp;
	size_t *auxv;
	size_t base;
	void *base_cap;
	void *data_cap;
	const void *code_cap;
	const void *rodata_cap;
	struct capreloc *start_cap_relocs;
	struct capreloc *stop_cap_relocs;
} __start_params_t;

struct __locale_map;

struct __locale_struct {
	const struct __locale_map *cat[6];
};

struct tls_module {
	struct tls_module *next;
	void *image;
	size_t len, size, align, offset;
};

struct __libc {
	char can_do_threads;
	char threaded;
	char secure;
	volatile signed char need_locks;
	int threads_minus_1;
	size_t *auxv;
	struct tls_module *tls_head;
	size_t tls_size, tls_align, tls_cnt;
	size_t page_size;
	struct __locale_struct global_locale;
};

#ifndef PAGE_SIZE
#define PAGE_SIZE libc.page_size
#endif

extern hidden struct __libc __libc;
#define libc __libc

#ifdef __CHERI_PURE_CAPABILITY__
hidden void __init_libc(__start_params_t *params);
#else
hidden void __init_libc(char **, char *);
#endif
hidden void __init_tls(size_t *);
hidden void __init_ssp(void *);
hidden void __libc_start_init(void);
hidden void __funcs_on_exit(void);
hidden void __funcs_on_quick_exit(void);
hidden void __libc_exit_fini(void);
hidden void __fork_handler(int);

extern hidden size_t __hwcap;
extern hidden size_t __sysinfo;
extern char *__progname, *__progname_full;

extern hidden const char __libc_version[];

hidden void __synccall(void (*)(void *), void *);
hidden int __setxid(int, int, int, int);

#endif
