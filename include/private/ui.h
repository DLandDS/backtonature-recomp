/* PROVISIONAL translation-unit boundary.
 * Range: 0x80020E38..0x80024AE4.
 * Basis: UI and dialog-label helpers through the game-to-PSY-Q boundary; correlated with
 * jtbl_80010224, menuHandlers, and jtbl_80010378. Keep the boundary provisional until
 * rodata/relocation evidence proves it. */
#include "common.h"
#include "include_asm.h"
#include "game_types.h"
#include "psyq_libcd.h"

/* TENTATIVE: UI work area; byte +0xCD gates access to the buffer at +0xD8. */
extern u8 gameState[];
extern u16 storage8005393A[];
extern s32 mainUiTextTableRuntimeOffset[];
extern s32 queuedSounds[];
extern u32 vabStream[];
extern s32 soundOffsets[];
extern s16 menuWindowValues[];
/* Incomplete-array declaration, matching the storage8006C905 trick elsewhere in
 * this file: forces %hi/%lo addressing instead of %gp_rel for this small
 * scalar-shaped byte flag. */
extern u8 storage8006C904[];
extern s16 heartIndexTable[];
extern u16 nameSymbolChars[];

/* TENTATIVE: byte flag cleared when a dialog page is retired.
 *
 * Declared as an array of unknown size on purpose. _gp is 0x8005E274 and the
 * gp-relative displacement is a signed 16-bit, so only 0x80056274..0x80066273
 * is reachable through $gp. This symbol is above that window, which is why
 * retail addresses it with %hi/%lo. Given a size, cc1 emits
 * `.extern storage8006C905, 1` and GNU as makes it small-data and gp-relative
 * anyway -- the relocation would overflow at link time. Without a size it
 * stays large, and the %hi/%lo pair comes out. */
extern u8 storage8006C905[];
/* CONFIDENCE: opcode-width declarations for the gp-relative UI state. */
extern u32 menuSoundEnabled;
extern s16 cdAudioRequest;
extern u16 nameSoundState;
extern u16 nameSoundValue;
extern u16 namePromptRow;
/* CONFIRMED pointer target begins with 16-bit fields; +2 is used below. */
extern u16 *nameSoundSeq;
extern u16 namePhase;
extern s32 nameCancelAllowed;
extern s16 nameEntryUnused8005E34C;
extern u16 nameLength;
extern s32 nameDirty;
extern u16 namePromptToggle;
extern s16 nameCursorCell;
extern s16 nameShift;
extern s32 nameTitleMode;
extern s16 *nameBuffer;
/* Array-of-one pointer, matching the same lever text.c/system.c already use
 * for this exact global: currentFrame's value is a context pointer, and the
 * array-of-unknown-length declaration is what forces %hi/%lo addressing
 * instead of %gp_rel (currentFrame = 0x8005E3C0 sits inside this unit's own
 * gp window, so a plain scalar declaration collapses the two-instruction
 * lui/lw pair into a single gp_rel load, which is not what retail does
 * here). */
extern s32 *currentFrame; /* nongp */
extern s32 currentEntry;
extern u32 menuId;
extern s32 menuBoxFrame;
extern u32 menuPhase;
extern u32 menuChosen;
extern s32 menuEventBits;
extern u32 menuSavedFlags;
extern u32 menuDataIndex;
extern u32 menuQueuedId;
extern u32 menuQueuedArg;
extern u8 uiIconCycle;
extern u8 uiIconTick;
extern u32 uiIconAnim[];
extern u8 menuAnim[];
extern s16 HWD0; /* nongp */
extern s16 VWD0; /* nongp */
extern u8 storage800977F8[];
extern s32 *inputPad1; /* nongp */
extern s32 voiceStamps[];

/* Forward-declared: defined later in this unit (name_buffer_clear), but called
 * here before that point. Retail's callers need the real void prototype
 * visible at the call site -- an implicit int declaration leaves the return
 * value register (v0) looking live to the allocator, which then places a
 * multiply-stored literal in v1 instead of v0 and mismatches. */
void name_buffer_clear(void);
