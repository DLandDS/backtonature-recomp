/* PROVISIONAL translation-unit boundary.
 * Range: 0x8001BAC4..0x80020E38.
 * Basis: Provisional gameplay group; rodata correlation did not yield narrower original translation
 * units. Keep the boundary provisional until rodata/relocation evidence proves it. */
#include "common.h"
#include "include_asm.h"
#include "game_types.h"
/* Shared draw context; retail accesses this pointer without $gp. */
typedef struct {
    u8 pad000[0x110];
    s32 otLen;
    s32 pad114;
    u32 *prim;
    s32 pad11C;
    u32 *ot;
} OtContext;
extern OtContext *currentFrame; /* nongp */
#include "psyq_gte.h"
#include "psyq_libgpu.h"

/* CONFIRMED width, TENTATIVE signedness: %gp_rel byte stores. */
extern u8 backColor;
extern u8 storage8005E2DD;
extern u8 storage8005E2DE;
extern u8 farColor;
extern u8 storage8005E2E1;
extern u8 storage8005E2E2;
extern u8 cameraDirty;
extern u8 cameraMode;

/* CONFIRMED width, TENTATIVE signedness: %gp_rel word loads/stores. */
extern s32 scaleEnabled;
extern s32 fogNear;
extern s32 cameraModeArg;
extern s32 lastQuadDepth;
extern s32 groundFlagBits;
extern s32 groundClip;
extern s32 cameraViewSettled;
extern s32 cameraViewX;
extern s32 cameraViewY;
extern s32 cameraViewZ;
extern s32 cameraBlockMode;
/* CONFIRMED width, TENTATIVE signedness: clut_next_serial %gp_rel counter. */
extern s32 clutSerial;
extern s32 groundTolerance;
extern s32 groundHitPrimPtr;
extern s32 seqArg;
extern s32 queueCursor;
extern s32 soundStereo;
extern s32 soundBeginCount;
extern s32 soundSsRunning;
extern s32 vabFreeMark;
extern s32 soundReadPtr;
extern s32 soundWritePtr;
/* CONFIRMED from callee frames: both helpers consume no stack arguments and
 * return without producing v0.  The FABC caller sign-extends its s16 input
 * before passing three register arguments to sound_key_on_free_voice. */
extern void xa_search_files(void);
extern s32 sound_key_on_free_voice(s16 arg0, s32 arg1, s32 arg2);
extern u32 sound_current_entry(void);
extern void sound_load_step(void);
extern void sound_fader_step(void);

/* CONFIRMED addressing/width, TENTATIVE signedness: %hi/%lo accesses stay incomplete. */
extern s32 storage80061F0C[];
extern s32 storage80061F1C[];
extern s32 masterFader[];
extern s32 savedMatrix[];
extern s32 voiceStamps[];
extern s32 queuedSounds[];
extern s32 queuedSoundHasArg[];
extern s32 storage8006BF88[];
extern s32 vabIds[];
extern s32 seqSlots[];
extern s32 seqData[];
extern s16 seqVabs[];
extern s32 seqStates[];
extern s32 seqHandles[];
extern void SsSeqStop(short);
extern void SsSeqReplay(short);
extern void SsSeqPause(short);
extern void SsSeqSetVol(short, short, short);
extern void SsSeqPlay(short, char, short);
extern void SsUtAllKeyOff(short);
extern s32 SsSeqOpen(s32, s16);
extern void SsStart(void);
extern void SsSetMVol(s16, s16);
extern void SsSeqClose(s16);
extern void SsEnd(void);
extern void SsVabClose(s16);
extern s32 sound_seq_stop(s32 arg0, s32 arg1);
extern s32 scalePairs[];
extern s32 frustumAngles[];
extern s16 reverbTable[];
extern s32 cameraEye[];
extern s32 cameraTarget[];
extern s32 cameraBlock[3];
extern s32 ratan2(s32 arg0, s32 arg1);
extern u16 modelKeys[];
extern s32 modelValues[];
extern MATRIX viewMatrix;
extern s16 clipPlanes[];
extern VECTOR viewDirection;
extern MATRIX GsIDMATRIX;
extern s32 VectorNormal(VECTOR *arg0, VECTOR *arg1);
extern MATRIX *RotMatrixX(s32 arg0, MATRIX *arg1);
extern MATRIX *RotMatrixY(s32 arg0, MATRIX *arg1);
extern void PushMatrix(void);
extern void PopMatrix(void);

/* TENTATIVE: %hi/%lo render-context head; model_draw_quad_depth/model_draw_quad_ot only
 * touch offsets 0x114 and 0x118, so the layout up to there is what's named. */
typedef struct {
    u8 pad000[0x110];
    s32 unk110;
    s32 unk114;
    void *unk118;
    s32 unk11C;
    void *unk120;
    u8 *unk124;
    u8 pad128[0x134 - 0x128];
} GfxCtx;
extern GfxCtx frameRecords;
/* Two distinct context pointers, not one: model_draw_quad_depth uses quadFrame
 * (gp+0x26C) and model_draw_quad_ot uses lastQuadFrame (gp+0x270). They are adjacent
 * and easy to conflate -- match_check will NOT catch swapping them, because
 * the gp displacement is still a pending relocation at object level. Only
 * `make verify` on the linked image shows it, as two bytes 0x26C vs 0x270. */
extern GfxCtx *quadFrame;
extern GfxCtx *lastQuadFrame;

/* TENTATIVE: five-color, four-vertex quad read by model_draw_quad_depth/model_draw_quad_ot. */
typedef struct {
    s32 col0;
    s32 col1;
    s32 col2;
    s32 col3;
    s32 baseColor;
    SVECTOR v0;
    SVECTOR v1;
    SVECTOR v2;
    SVECTOR v3;
} GfxQuadInput;

/* TENTATIVE: 0x28-byte packet written by model_draw_quad_depth/model_draw_quad_ot; the
 * size matches the literal passed to ot_insert_prim. */
typedef struct {
    s32 hdr;
    s32 col0;
    s32 xy0;
    s32 col1;
    s32 xy1;
    s32 col2;
    s32 xy2;
    s32 col3;
    s32 xy3;
    s32 col4;
} GfxPacket;

extern void *ot_insert_prim(s32 arg0, s32 arg1);

extern s32 SsSetReservedVoice(u8 arg0);
extern void SsSetStereo(void);
extern void sound_stereo_mode_hook(void);
extern s32 SsVoKeyOff(s32 arg0, s32 arg1);
extern s32 SsUtSetVVol(s32 arg0, s32 arg1, s32 arg2);
extern s32 SsUtKeyOffV(s32 arg0);
extern s32 SsUtSetReverbType(s32 arg0);
extern s32 SsUtSetReverbDepth(s32 arg0, s32 arg1);
extern s32 SsSetRVol(s32 arg0, s32 arg1);
extern s32 SsUtSetReverbFeedback(s32 arg0);
extern s32 SsUtSetReverbDelay(s32 arg0);
extern s32 SsUtReverbOn(void);
extern s32 SsUtReverbOff(void);
extern s32 SsSetMono(void);
extern s32 sound_voice_stop(s32 arg0);
extern s32 sound_key_off_voice(s32 arg0);
void vec3_copy(s32 *arg0, s32 *arg1);
void vec3_sub(s32 *arg0, s32 *arg1);

/* Typed views preserve the original declaration and compiler behavior. */
#define currentFrameView (*(FrameState * (*)) & currentFrame)
#define drawnPartsView   (*(s16(*)) & drawnParts)
#define modelFrameView   (*(FrameState * (*)) & modelFrame)
