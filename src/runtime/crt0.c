/* Included by src/crt0.c; shares its private declarations. */
void __main(void) {}

/* Round 38 CRT0 check: fixed v0/v1/at C variables reproduce all nine loop
 * instructions, but GCC 2.95 appends jr ra and its nop instead of falling
 * through into stup1. noreturn and naked do not suppress that epilogue;
 * __builtin_unreachable is compiled as an ordinary external call. */
INCLUDE_ASM("asm/nonmatchings/crt0", start);
/* Sony SDK bootstrap fragment, not a C ABI function: installs sp/gp/fp,
 * saves the incoming ra in BSS, calls InitHeap, restores ra, and falls
 * through into stup0. A C attempt using fixed registers still emits a
 * stack frame before sp initialization and differs at all 30 instructions. */
INCLUDE_ASM("asm/nonmatchings/crt0", stup1);
