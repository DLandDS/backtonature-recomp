/* PROVISIONAL translation-unit boundary.
 * Range: 0x800147BC..0x80017A38.
 * Basis: A_FILE loading, animation setup, and 0D container decode helpers.
 * Keep the boundary provisional until rodata/relocation evidence proves it. */
#include "common.h"
#include "include_asm.h"
#include "game_types.h"
#include "psyq_libcd.h"
#include "psyq_libgpu.h"

typedef struct ArchiveEntry {
    u16 flags;
    u16 unk02;
    u8 unk04;
    u8 unk05;
    u16 unk06;
    u32 unk08;
    u16 unk0C;
    u16 unk0E;
    u16 unk10;
    u16 unk12;
    void *unk14;
    void *unk18;
    void *unk1C;
    void *unk20;
    void *unk24;
    void *unk28;
    void *unk2C;
} ArchiveEntry;

typedef struct ArchiveHeader {
    u32 unk00;
    u32 unk04;
    u32 unk08;
    u32 unk0C;
    u32 unk10;
    u32 unk14;
    u32 unk18;
    u32 unk1C;
    u32 unk20;
} ArchiveHeader;

extern s32 afileRangeActive;
extern s32 afileStreamState;
extern s32 afileRangeStart;
extern s32 afileRangeEnd;
extern s32 afileRangeBuf;
extern s32 afileBusy;
extern void *animScreenFlag;
extern char storage800100A0[];
extern s32 FntPrint(char *fmt, ...);
extern s32 host_poll(void);
extern void text_window_clear(void);
extern s32 afile_start_range(s32 arg0, s32 arg1, s32 arg2);
extern void *gs_work_alloc(s32 arg0);
extern void anim_set_source(ArchiveEntry *arg0, void *arg1, void *arg2);
extern void anim_draw_sprt_cursor(void *arg0, void *arg1, u8 arg2, u8 arg3, s16 arg4, s16 arg5);
void anim_draw_sprt(void *arg0, void *arg1, void *arg2, void *arg3, u8 arg4, u8 arg5, s16 arg6,
                    s16 arg7);

/* B0 prototype evidence, round 26:
 * CONFIRMED: anim_tick has exactly one pointer argument.  Its prologue
 * copies a0 to a3 at 0x800158E4 and has no frame or stack loads.
 * CONFIRMED: anim_draw_sprt_parts has eight arguments: a0/a1/a2/a3, then lbu
 * arguments at 0x80016294 (0x38) and 0x800162CC (0x3C), and lh arguments at
 * 0x800162B8 (0x40) and 0x800162BC (0x44).  The callee preserves the first
 * argument in s1 and the third in s0, confirming ordinary word-sized ABI
 * slots for the register arguments.
 * CONFIRMED: anim_draw_ft4_cursor has nine arguments: a0/a1, byte-like a2/a3
 * (0x80015FC4/0x80015FD4 mask them with 0xFF), then lh at 0x80015F98
 * (0x78) and 0x80015FA0 (0x7C), lw at 0x80015F88 (0x80) and 0x80015F90
 * (0x84), and lhu at 0x80015FB0 (0x88).  Its frame is 0x68.
 * CONFIRMED: sprite_draw_frame has fifteen arguments: a0/a1/a2 and byte-like a3
 * (0x800165EC masks a3 with 0xFF), followed by lbu at 0x8001661C (0x40),
 * lh at 0x80016628 (0x44), 0x8001662C (0x48), lw at 0x80016620 (0x4C)
 * and 0x80016624 (0x50), lhu at 0x80016630 (0x58), lh at 0x80016634
 * (0x5C), 0x80016660 (0x60), 0x80016638 (0x64), and 0x80016668
 * (0x68).  Its frame is 0x30. */
extern void anim_tick(ArchiveEntry *arg0);
s32 anim_draw_ft4(void *arg0, void *arg1, void *arg2, u8 arg3, u8 arg4, s16 arg5, s16 arg6,
                  void *arg7, void *arg8, u16 arg9, s16 arg10, s16 arg11);
extern void anim_draw_sprt_parts(void *arg0, void *arg1, void *arg2, void *arg3, u8 arg4, u8 arg5,
                                 s16 arg6, s16 arg7);
extern void anim_draw_ft4_cursor(void *arg0, void *arg1, u8 arg2, u8 arg3, s16 arg4, s16 arg5,
                                 void *arg6, void *arg7, u16 arg8);
extern POLY_FT4 *sprite_draw_frame(u_long *arg0, POLY_FT4 *arg1, AnimState *arg2, u8 arg3, u8 arg4,
                                   s16 arg5, s16 arg6, s32 arg7, s32 arg8, s32 arg9, u16 arg10,
                                   s16 arg11, s16 arg12, s16 arg13, s16 arg14);
extern void gs_work_release(void *top);
/* CONFIRMED from 15DD4's four register arguments and seven outgoing words at
 * 0x10..0x28; 16C08 consumes the corresponding stack slots at 0x9C..0xA0
 * and later.  Scalar widths are intentionally kept as s32 until field
 * semantics are established. */
extern POLY_FT4 *text_sprite_draw(u_long *arg0, POLY_FT4 *arg1, AnimState *arg2, s16 arg3, s16 arg4,
                                  u16 arg5, u8 *arg6, VramPlace *arg7, s16 arg8, s16 arg9,
                                  s16 arg10);

/* Evidence: DecodeContainer0D reads a pointer in a0, three signed 16-bit
 * register arguments in a1-a3, then a signed 16-bit stack argument and a 32-bit mode. */
extern void DecodeContainer0D(void *container, s16 arg1, s16 arg2, s16 arg3, s16 arg4, s32 arg5);

typedef struct ArcFileEntry {
    u8 pad[0x24];
    u32 offset;
} ArcFileEntry;
extern ArcFileEntry afileMembers[];
extern char storage80010078[];
extern char storage80010088[];
extern u32 gameState[];
extern s32 afileSector;
extern s32 afileBaseSector;
s32 cd_read_file(char *name, u32 *buffer);

/* Typed views preserve the original declaration and compiler behavior. */
#define currentFrameView  (*(FrameState * (*)) & currentFrame)
#define textSpriteKeyView (*(s16(*)) & textSpriteKey)
