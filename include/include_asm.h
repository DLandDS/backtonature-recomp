#ifndef INCLUDE_ASM_H
#define INCLUDE_ASM_H

/* Authoritative template copied to include/ after every splat split.
 *
 * splat's stock template emits an extra top-level asm block that has no
 * declaration-order name, so tools/asm_order.py must reject it. Keeping
 * macro.inc inside each expansion makes every block self-contained.
 *
 * noreorder and noat are required because the retail disassembly is already
 * scheduled. The modes are restored for any C function that follows. */
#define INCLUDE_ASM(dir, func)                    \
    __asm__(".include \"macro.inc\"\n"            \
            ".section .text\n"                    \
            ".set noat\n"                        \
            ".set noreorder\n"                    \
            ".include \"" dir "/" #func ".s\"\n" \
            ".set reorder\n"                      \
            ".set at\n")

#endif /* INCLUDE_ASM_H */
