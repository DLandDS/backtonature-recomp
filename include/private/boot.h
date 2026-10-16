/* PROVISIONAL translation-unit boundary.
 * Range: 0x80011370..0x800129F4.
 * Basis: Main boot and control flow after the handwritten crt0 gap.
 * Keep the boundary provisional until rodata/relocation evidence proves it. */
#include "common.h"
#include "include_asm.h"
#include "game_types.h"

/* PROBABLE: the two halves of a bump allocator built on the PSY-Q heap-top
 * accessors. GsGetWorkBase reads the current top, GsSetWorkBase writes it.
 * Both live above 0x80024A4C, outside the disassembled range, so they are
 * library code resolved at link time rather than functions of this build. */
extern void *GsGetWorkBase(void);
extern void GsSetWorkBase(void *top);

/* CONFIRMED: declared identically in text.c, which already calls both. */
extern s32 host_poll(void);
extern void SystemError(s32 code, s32 detail);

/* PROBABLE: a pointer to a global context block -- the load is indirect, so
 * currentFrame holds the address rather than being the block itself. Only two
 * fields are touched here, and their sum is the arena's upper limit; which of
 * the two is the base and which the length is not decidable from this
 * function alone, so both are left named for their offsets.
 *
 * Reached %gp_rel from this translation unit but %hi/%lo from text_queue_sprite,
 * sprite_draw_frame and text_sprite_draw -- the per-TU -G split, not two objects. */
/* TENTATIVE: one ordering-table band per (ot, len) pair: its entry count,
 * first and last cells, and two words main_init_frame_records clears. */
typedef struct {
    s32 count;
    u32 *first;
    s32 unk08;
    s32 unk0C;
    u32 *last;
} OtBand;

typedef struct {
    u8 unk000[0x16];
    u8 unk016;
    u8 unk017;
    u8 unk018;
    u8 unk019;
    u8 unk01A;
    u8 unk01B;
    u8 unk01C[0x84 - 0x1C];
    OtBand band[4]; /* 0x84 */
    u8 unk0D4[0xD8 - 0xD4];
    /* TENTATIVE: four (ot pointer, length) pairs cleared via ClearOTagR in
     * main_frame_clear, plus a fifth ot cleared unconditionally with a fixed
     * length of 0x800. */
    void *ot0; /* 0xD8 */
    void *ot1; /* 0xDC */
    void *ot2; /* 0xE0 */
    void *ot3; /* 0xE4 */
    u8 unk0E8[0xEC - 0xE8];
    s32 len0; /* 0xEC */
    s32 len1; /* 0xF0 */
    s32 len2; /* 0xF4 */
    s32 len3; /* 0xF8 */
    u8 unk0FC[0x100 - 0xFC];
    s32 count0; /* 0x100 */
    s32 count1;
    s32 count2;
    s32 count3;
    s32 field110;
    s32 field114;
    s32 field118;
    s32 field11C;
    void *ot4; /* 0x120 */
    s32 field124;
    /* TENTATIVE: field128 + field12C == one-past-the-end of the arena. */
    s32 field128;
    s32 field12C;
    void *field130;
} BootContext;

extern BootContext *currentFrame;

/* Retail addressing modes establish the storage widths and signedness of these
 * small globals: word loads/stores are s32, while the byte accessor pair is u8. */
extern s32 mainPreviousScene;
extern s32 mainScene;
extern s32 mainLoopRunning;
extern s32 mainUnused8005E280;
extern u8 storage8005E274;
extern s32 mainPutEnvEnabled;
extern s32 mainClearScreenEnabled;
extern s32 regionBase;
/* Small externs retail reaches with an unsplit lui/lw, never $gp. */
extern s32 storage8001009C; /* nongp */
extern s32 farmSectionSize; /* nongp */
extern s32 storage8007AFF4[];

extern void ClearOTagR(void *ot, s32 n);

/* Ported from hmbtn-decomp (CC0), main. */
int CdInit(void);
extern int DrawSync(int mode);
extern void GsSetDrawBuffOffset(void);
extern void GsSetOffset(s32 x, s32 y);
void InitGeom();
void McxStartCom(void);
void MemCardInit(long val);
void MemCardStart(void);
int ResetCallback(void);
int ResetGraph(int mode);
DISPENV *SetDefDispEnv(DISPENV *env, int x, int y, int w, int h);
DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h);
extern void SetGeomOffset(s32 x, s32 y);
extern void SetGeomScreen(s32 value);
int SetGraphDebug(int level);
void SetMem(long);
long SetVideoMode(long mode);
extern int VSync(int mode);
int VSyncCallback(void (*f)(void));
extern u8 *activeOt;
s32 afile_init(void);
s32 LoadAFileEntryToMemory(s32 startIndex, s32 endIndex, u8 *dst, u8 *buf);
void camera_init(void);
extern BootContext *currentFrame;
void debugTick(void);
void fade_start_in(s16 length, s16 ownOt, s16 blend);
extern FarmLoadArea farmArea;
void farmEnterDispatch(void);
void farm_entry_args_clear(void);
void farm_state_init(void);
extern void frame_buffers_layout(void);
extern s32 storage8005E3A4;
extern u8 storage8005E5D8[];
extern BootContext frameRecords[];
extern s32 gameState[];
extern InputRecord *inputPad1;
extern InputRecord *inputPad2;
extern InputRecord inputRecords[2];
void loadScreenTick(void);
void loading_screen_init(void);
void loading_screen_stop(void);
int main(void);
extern s32 mainClearScreenEnabled;
void main_copy_prim_buffer(s32 mode, s32 which);
void main_frame_begin(void);
void main_frame_reset(s32 index);
extern s32 mainFrameStarted;
void main_run_frame(void);
extern s32 mainScene;
void main_set_debug_mode(s32 unused);
void main_set_put_env(s32 value);
void mcardScreenTick(void);
void mg1RunState(void);
void mg2PhaseRun(void);
void mg3StepDispatch(void);
void mg4RunStep(void);
void mg5StepRun(void);
extern u8 *const overlayRegionTable[2]; /* nongp */
void pad_init(void);
void pad_reset_records(void);
void pad_set_repeat(s32 delay, s32 rate);
void pad_set_repeat2(u32 delay, u32 rate);
s32 pdaSampleRun(void);
extern s32 primCopyPending;
void saveScreenTick(void);
void saveTransferScreenTick(void);
extern void (*sceneFrameHandler)(void);
extern s32 sceneOverlayLoaded;
void screen_clear_rect(s16 x, s16 y, s16 w, s16 h, u8 r, u8 g, u8 b);
void sound_fade_in(s32 ticks);
void sound_fader_init(void);
void sound_init(void);
void sound_init_stereo_xa(void);
s32 sound_update(void);
void debugHandlers(void);
void swindWindowTick(void);
void text_set_palette(u32 palette);
void text_set_speed(s32 index);
void titleStepDispatch(void);
void vsyncFrameCallback(void);
extern void (*vsyncPendingCallback)(void);

/* Typed views preserve the original declaration and compiler behavior. */
#define activeOtView               (*(u_long * (*)) & activeOt)
#define currentFrameView           (*(FrameState * (*)) & currentFrame)
#define farmAreaView               (*(FarmLoadArea(*)) & farmArea)
#define frameRecordsView           (*(FrameState(*)[]) & frameRecords)
#define gameStateView              (*(FarmState(*)) & gameState)
#define inputPad1View              (*(InputRecord * (*)) & inputPad1)
#define inputPad2View              (*(InputRecord * (*)) & inputPad2)
#define inputRecordsView           (*(InputRecord(*)[2]) & inputRecords)
#define mainClearScreenEnabledView (*(s32(*)) & mainClearScreenEnabled)
#define mainFrameStartedView       (*(s32(*)) & mainFrameStarted)
#define mainSceneView              (*(s32(*)) & mainScene)
#define overlayRegionTableView     (*(u8 *const(*)[2]) & overlayRegionTable)
#define primCopyPendingView        (*(s32(*)) & primCopyPending)
#define sceneFrameHandlerView      (*(void (**)(void)) & sceneFrameHandler)
#define sceneOverlayLoadedView     (*(s32(*)) & sceneOverlayLoaded)
#define storage8005E3A4View        (*(s32(*)) & storage8005E3A4)
#define storage8005E5D8View        (*(u_long(*)[4]) & storage8005E5D8)
#define vsyncPendingCallbackView   (*(void (**)(void)) & vsyncPendingCallback)
