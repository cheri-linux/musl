#ifdef __CHERI_PURE_CAPABILITY__

__asm__(
".section .sdata,\"aw\"\n"
".text\n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
".weak __global_pointer$\n"
".hidden __global_pointer$\n"
".option push\n"
".option norelax\n\t"
"cllc cgp, __global_pointer$\n"
".option pop\n\t"
"cmove ca0, csp\n"
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n\t"
"cllc ca1, _DYNAMIC\n\t"
"andi t0, sp, -32\n\t"
"csetaddr csp, csp, t0\n\t"
"cllc ct0, " START "_c\n\t"
"cjr ct0"
);

#else

__asm__(
".section .sdata,\"aw\"\n"
".text\n"
".global " START "\n"
".type " START ",%function\n"
START ":\n"
".weak __global_pointer$\n"
".hidden __global_pointer$\n"
".option push\n"
".option norelax\n\t"
"lla gp, __global_pointer$\n"
".option pop\n\t"
"mv a0, sp\n"
".weak _DYNAMIC\n"
".hidden _DYNAMIC\n\t"
"lla a1, _DYNAMIC\n\t"
"andi sp, sp, -16\n\t"
"tail " START "_c"
);

#endif
