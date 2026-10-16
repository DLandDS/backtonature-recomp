/* PROVISIONAL translation-unit boundary.
 * Range: 0x80012A34..0x800147BC.
 * Basis: System, graphics, input, and setup helpers before A_FILE path handling.
 * Keep the boundary provisional until rodata/relocation evidence proves it. */
#include "common.h"
#include "include_asm.h"
#include "psyq_libgpu.h"
#include "psyq_libcd.h"

/* CONFIRMED: PSY-Q libetc. Retail calls this with -1 to read the counter. */
extern int VSync(int mode);
extern void ChangeClearPAD(s32 mode);
extern u32 randSeed;
extern s16 fadeMode;
extern s32 fadeSkipNext;
extern s16 fadeBlend;
extern s32 fadeEnabled;
extern s16 fadeLevel;
extern s16 fadeLength;
extern s32 fadeOwnOt;
extern s32 fadeStepsLeft;
extern s32 fadeStepSize;
extern s32 fadeStepMode;
extern s32 fadeHold;
extern s32 padStarted;
extern s32 fadePutPending;
extern void main_set_put_env(s32 value);
extern void main_set_clear_screen(s32 value);
