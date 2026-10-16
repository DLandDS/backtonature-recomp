#include "common.h"

/* text_clear_bank stores through $gp, so this unit was built with a non-zero -G.
 * At -G0 the same source emits lui/%hi + %lo and puts the store in the delay
 * slot -- visibly not what retail does. -G4 and -G8 are indistinguishable here
 * because the only small object involved is 2 bytes wide.
 *
 * A packed bit array lives 0x38 bytes into gameState. Retail loads through
 * `lw v0, 0x38(base)` -- the displacement stays on the load rather than being
 * folded into the index -- so 0x38 is a member offset within a structure, not
 * an element offset into a flat array. Indices are 16-bit, split into a word
 * selector (>> 5) and a bit within that word (& 31).
 *
 * Both halves are held in locals because retail computes them before the
 * branch in event_flag_set and shares them across both arms. */
typedef struct {
    u8 unk00[0x38];
    u32 flags[1];
} FlagBlock;

extern FlagBlock gameState;
extern u16 textBankId;
