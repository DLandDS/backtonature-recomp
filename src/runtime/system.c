/* Included by src/boot.c; shares its private declarations. */
/* A breakpoint stub: it traps, and reports 1 to whoever resumes it. No C
 * construct emits BREAK on this compiler -- __builtin_break(1) becomes an
 * external call and 2.95 has no __builtin_trap -- so the instruction is
 * written directly. That is the whole function; everything around it is C.
 *
 * CONFIRMED: the operand must be the full 20-bit code, 0x400. maspsx rewrites
 * a one-operand `break N` into the two-field form as `break N>>10, N&0x3FF`
 * (maspsx/__init__.py, the `op == "break"` arm), so `break 1` assembles to
 * 0x0000004D -- the 1 lands in the low field. Retail's 0x0001000D has the 1 in
 * the high field, which is `break 0x400` before that rewrite. Do not "simplify"
 * this to `break 1`; GNU as alone would accept it and give the right word, but
 * the source never reaches GNU as unrewritten. */
s32 host_poll(void)
{
    __asm__ volatile("break 0x400");
    return 1;
}
