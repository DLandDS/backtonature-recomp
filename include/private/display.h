/* PROVISIONAL translation-unit boundary.
 * Range: 0x800129F4..0x80012A34.
 * Basis: Two confirmed SetDispMask wrappers with matching -G0 code generation.
 * Keep the boundary provisional until rodata/relocation evidence proves it. */
#include "common.h"
#include "include_asm.h"

/* CONFIRMED: PSY-Q display-mask API, identified by retained SDK symbol. */
extern void SetDispMask(int mask);
