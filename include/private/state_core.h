/* PROVISIONAL translation-unit boundary.
 * Range: 0x8001AFE0..0x8001BAC4.
 * Basis: State-block helpers around padVibrationHandlers and the three existing matched state
 * functions. Keep the boundary provisional until rodata/relocation evidence proves it. */
#include "common.h"
#include "include_asm.h"
#include "psyq_gte.h"

extern s32 fogNear;
extern u8 loadingPackets[];
extern u8 backColor;
extern u8 storage8005E2DD;
extern u8 storage8005E2DE;
extern u8 farColor;
extern u8 storage8005E2E1;
extern u8 storage8005E2E2;
extern MATRIX colorMatrix;
extern MATRIX lightMatrix;
extern MATRIX baseMatrix;

extern void SetFogNear(s32 a, s32 h);
extern void SetFarColor(s32 rfc, s32 gfc, s32 bfc);

/* TENTATIVE: positional state block; no field semantics are claimed yet. */
typedef struct {
    s32 unk00;
    s32 unk04;
    s32 unk08;
    s32 unk0C;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
} Block20;

extern Block20 modelCamera;
extern Block20 viewMatrix;
extern Block20 *storage80061E04[];
extern s32 loadingScreenActive;
typedef struct {
    u8 unk00[0x36];
    u8 unk36;
    u8 unk37;
} StateData;
extern StateData inputRecords;
extern s16 vibrationRepeats[2];
extern s16 vibrationActive[2];
extern s32 vibrationCursors[2];
extern s16 vibrationPriorities[2];
typedef void (*StateCallback)(u32 index);
/* PROBABLE: callback dispatch table indexed by the two active state slots.
 * Keep the bound incomplete: retail materializes its address with hi/lo. */
extern StateCallback padVibrationHandlers[];
extern void *memset(void *dst, s32 value, u32 size);

/* B3 prototype evidence: 15704 copies a0/a1 into s0/s1 at 0x8001570C and
 * 0x80015714, then dereferences s1 as a 0D header and writes the entry fields
 * through s0.  The existing archive types are not shared across units, so
 * retain the ABI as opaque pointers here. */
extern void anim_bind_file(void *entry, void *header);
extern void DecodeContainer0DFromStart(void *container);
extern void ClearOTagR(void *ot, s32 n);
extern void display_off(void);
void camera_set_eye_roll(s32 a, s32 b, s32 c, s32 d);
void camera_set_target(s32 a, s32 b, s32 c);

/* Evidence: B028 sign-extends four register halfwords and loads two more
 * halfwords from the caller stack before forwarding all six unchanged to the
 * bounded callee. */
extern void loading_screen_start(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5);
