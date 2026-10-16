/* Included by src/system.c; shares its private declarations. */
u32 rand_range(u32 range)
{
    randSeed = randSeed * 0x41C64E6D + 0x3039;
    return ((randSeed >> 16) * (range & 0xFFFF)) >> 16;
}
/* CONFIRMED: PSY-Q libgpu/libetc. */
extern int ClearImage(RECT *rect, u8 r, u8 g, u8 b);
extern int DrawSync(int mode);

void screen_clear_rect(s16 x, s16 y, s16 w, s16 h, u8 r, u8 g, u8 b)
{
    RECT rect;

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    ClearImage(&rect, r, g, b);
    DrawSync(0);
}
typedef struct {
    s32 unk00;
    s32 unk04;
    s32 unk08;
    s32 unk0C;
    u32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
    s32 unk20;
    s32 unk24;
    u32 unk28;
    s32 unk2C;
    s32 unk30;
    s16 unk34;
    u8 unk36;
    u8 unk37;
} SystemState;

/* PROBABLE: the same 0x38-byte SystemState record pad_clear_record initializes
 * through its parameter, reached here as two parallel instances (a
 * secondary/mirror pair -- every write below hits both) via an unsplit
 * lui/lw pair in one register: small externs retail reaches without $gp,
 * hence the nongp marker (see NONGP_DECL in tools/match_check.py). */
extern SystemState *inputPad1; /* nongp */
extern SystemState *inputPad2; /* nongp */

typedef struct {
    u8 data[0x18];
} DispEnv;
typedef struct {
    u8 data[0x60];
} DrawEnv;
extern int SetDefDispEnv(DispEnv *env, s32 x, s32 y, s32 w, s32 h);
extern int SetDefDrawEnv(DrawEnv *env, s32 x, s32 y, s32 w, s32 h);
extern int PutDispEnv(DispEnv *env);
extern int PutDrawEnv(DrawEnv *env);
extern u8 storage8005E644[];
void pad_poll_ports(void);

void screen_pan_display(s16 w, s16 h)
{
    struct {
        s16 x;
        s16 y;
        u8 rest[0x14];
    } disp;

    SetDefDispEnv((DispEnv *)&disp, 0, 0, w, h);
    PutDispEnv((DispEnv *)&disp);
    inputPad1->unk08 = 0;
    do {
        if (inputPad1->unk04 & 0x8000) {
            disp.x -= 8;
        }
        if (inputPad1->unk04 & 0x2000) {
            disp.x += 8;
        }
        if (inputPad1->unk04 & 0x1000) {
            disp.y -= 8;
        }
        if (inputPad1->unk04 & 0x4000) {
            disp.y += 8;
        }
        VSync(0);
        pad_poll_ports();
        PutDispEnv((DispEnv *)&disp);
        if (inputPad1->unk08 & 0x20) {
            SetDefDispEnv((DispEnv *)storage8005E644, disp.x, disp.y, 0x280, 0x1E0);
            SetDefDispEnv((DispEnv *)(storage8005E644 + 0x134), disp.x, disp.y, 0x280, 0x1E0);
        }
        if (inputPad1->unk08 & 0x40) {
            SetDefDispEnv((DispEnv *)storage8005E644, 0, 0xF0, 0x140, 0xF0);
            SetDefDispEnv((DispEnv *)(storage8005E644 + 0x134), 0, 0, 0x140, 0xF0);
        }
    } while (!(inputPad1->unk08 & 1));
}
/* A frame-timer elapsed check over the VSync counter.
 *
 * CONFIRMED: VSync(-1) returns the free-running vertical-blank count; retail
 * passes -1 and uses the result as a monotonic timestamp.
 *
 * The reload of *last after the zero-initialisation arm is retail's, not an
 * artefact: cc1 2.95.2 will not forward the just-stored value across the
 * conditional store, so the second read has to be written as a second read.
 *
 * `slt` is signed, so the comparison is a signed one on the difference rather
 * than an unsigned one -- which is what makes this robust across the counter
 * wrapping. Retail computes `now - *last` and compares it against interval.
 *
 * PROBABLE: the third parameter is an auto-rearm flag. Retail compares it
 * against 1 rather than testing it for non-zero, and the `addiu v0, zero, 1`
 * that produces the comparison operand is the same register that carries the
 * return value out -- so the constant is emitted once and used twice. Writing
 * `return 1` after an `== 1` test reproduces that sharing. */
int vsync_elapsed(int interval, int *last, int reset)
{
    int now = VSync(-1);

    if (*last == 0) {
        *last = now;
    }

    if (now - *last < interval) {
        return 0;
    }

    if (reset == 1) {
        *last = now;
    }

    return 1;
}
/* PROBABLE: double-buffer index shared with the frame-setup family in this
 * unit -- it feeds both the POLY_F4 and DR_MODE arrays below, each indexed
 * by the same value (idx*3, then <<3 for the 24-byte POLY_F4 stride and <<2
 * for the 12-byte DR_MODE stride). Declared as an incomplete array rather
 * than a plain scalar to keep cc1 on the retail %hi/%lo access path instead
 * of %gp_rel under this unit's default -G8 -- the same device text.c uses
 * for inputPad1/currentFrame. */
extern s32 storage8005E3A4; /* nongp */

/* PROBABLE: pointer to the shared draw-environment context; the ordering
 * table head cell lives at +0xDC and is read/written through it by the two
 * addPrim mask-pair sequences below. This is the same object boot.c reaches
 * as `BootContext *` (via %gp_rel there) and text.c reaches as
 * `DialogRenderContext *[]` (via %hi/%lo there, same as here) -- one
 * object, per-TU addressing paths, not three types. Declared array-of-one
 * for the same %hi/%lo reason as storage8005E3A4 above. */
typedef struct {
    u8 unk00[0xD8];
    u32 *unkD8;
    u32 *unkDC;
} DrawEnvContext;

extern DrawEnvContext *currentFrame; /* nongp */

extern POLY_F4 fillQuads[];
extern DR_MODE fillQuadModes[];

extern s32 fadeHalfStep;
extern u32 *activeOt; /* nongp */
extern u8 frameRecords[];
extern POLY_F4 fadeQuads[];
extern DR_MODE fadeQuadModes[];
s32 fade_is_settled(void);
void fade_set_enabled(s32 arg0);
extern u8 main_get_speed_mode(void);

void screen_fade_frame(void)
{
    s32 scale;
    s32 level;
    s32 v;
    s32 t;
    s32 abr;
    u32 *ot;
    POLY_F4 *p;
    DR_MODE *m;
    s32 i;

    scale = 0;
    if (fade_is_settled() == 1 && fadeEnabled != 1) {
        return;
    }
    fade_set_enabled(1);
    fadeHalfStep = fadeHalfStep == 0;
    if (fadeSkipNext == 1) {
        fadeSkipNext = 0;
        return;
    }
    if (fadeMode == 1) {
        if (fadeLevel > 0) {
            fadeLevel--;
        }
    } else if (fadeMode == 2) {
        if (fadeLength + 6 >= fadeLevel) {
            fadeLevel++;
        }
    } else if (fadeMode == 3) {
        main_set_clear_screen(0);
        if (fadeLength > fadeLevel) {
            if (main_get_speed_mode() == 0) {
                fadeLevel += fadeHalfStep;
            } else {
                fadeLevel++;
            }
        }
    } else {
        goto other;
    }
    if (fadeMode == 3) {
        if (fadeLength == fadeLevel) {
            fadeMode = 0;
            frameRecords[0x18] = 1;
            frameRecords[0x14C] = 1;
            frameRecords[0x17] = 0;
            frameRecords[0x14B] = 0;
            screen_clear_rect(0, 0, 0x140, 0x1E0, 0, 0, 0);
        } else if (fadeLength < fadeLevel) {
            fadeLevel = fadeLength;
        }
        scale = 0xFF000 / fadeLength;
        level = 0xFF;
        goto draw;
    }
other:
    if (fadeLevel >= fadeLength + 1) {
        level = 0xFF;
        if (fadeMode != 2) {
            goto stop;
        }
        if (fadeLength + 6 < fadeLevel) {
            goto draw;
        }
        goto stop;
    }
    if (fadeLength != 0) {
        scale = 0xFF000 / fadeLength;
        t = scale * fadeLevel / 4096;
        if (t < 0) {
            t = 0;
        }
        if (t >= 0x100) {
            t = 0xFF;
        }
        level = t & 0xFF;
        goto draw;
    }
    level = 0;
stop:
    fadeMode = 0;
draw:
    if (fadeOwnOt == 0) {
        ot = activeOt;
    } else {
        ot = currentFrame->unkD8 + 5;
    }
    if (level == 0) {
        return;
    }
    i = storage8005E3A4;
    p = &fadeQuads[i];
    m = &fadeQuadModes[i];
    if (fadeMode == 3) {
        if (fadePutPending == 1) {
            fadePutPending = 0;
            PutDispEnv((DispEnv *)(storage8005E644 + i * 0x134));
        }
        frameRecords[0x18] = 0;
        frameRecords[0x14C] = 0;
        frameRecords[0x17] = 1;
        frameRecords[0x14B] = 1;
        v = scale / 4096;
        abr = 2;
    } else {
        v = level;
        abr = fadeBlend;
    }
    setPolyF4(p);
    setRGB0(p, v, v, v);
    setXY4(p, 0, 0, 320, 0, 0, 240, 320, 240);
    setSemiTrans(p, 1);
    setDrawMode(m, 1, 1, (abr & 3) << 5, 0);
    addPrim(ot, p);
    addPrim(ot, m);
}

/* Full-screen semi-transparent flat quad plus its DR_MODE packet for the
 * current double buffer, chained onto env->unkDC. The macro order (setRGB0
 * before setXY4, setSemiTrans after) reproduces retail store order; with
 * storage8005E3A4/currentFrame nongp it matches exactly. */
void screen_fill_quad(u32 r, u32 g, u32 b)
{
    POLY_F4 *p = &fillQuads[storage8005E3A4];
    DR_MODE *m = &fillQuadModes[storage8005E3A4];
    DrawEnvContext *env = currentFrame;

    setPolyF4(p);
    setRGB0(p, r >> 12, g >> 12, b >> 12);
    setXY4(p, 0, 0, 320, 0, 0, 240, 320, 240);
    setSemiTrans(p, 1);
    setDrawMode(m, 1, 1, 0x40, 0);
    addPrim(env->unkDC, p);
    addPrim(env->unkDC, m);
}
void fade_reset(void)
{
    fadeMode = 1;
    fadeLevel = 0;
    fadeLength = 0x3C;
    fadeSkipNext = 0;
    fadeBlend = 2;
    fadeOwnOt = 0;
}

void fade_start_in(s16 value, s16 timer, s32 mode)
{
    fadeMode = 1;
    fadeLevel = value - 1;
    fadeLength = value;
    fadeSkipNext = 0;
    fadeBlend = mode;
    fadeOwnOt = timer;
}

void fade_start_out(s16 value, s16 timer, s32 mode)
{
    fadeMode = 2;
    fadeLevel = 0;
    fadeLength = value + 1;
    fadeSkipNext = 0;
    fadeBlend = mode;
    fadeOwnOt = timer;
}
void fade_start_blend(s16 value)
{
    fadeMode = 3;
    fadeLevel = 0;
    if (value >= 31) {
        fadeLength = value = 30;
    }
    fadeLength = value + 1;
    fadePutPending = 1;
    fadeSkipNext = 1;
    main_set_put_env(0);
    main_set_clear_screen(0);
    fadeBlend = 2;
    fadeOwnOt = 0;
}
s32 fade_is_settled(void)
{
    s16 state = fadeMode;
    if (state == 1) {
        return fadeLevel < 1;
    }
    if (state == 3) {
        return fadeLevel == fadeLength + 3;
    }
    if (state == 2) {
        return fadeLevel >= fadeLength + 1;
    }
    return 1;
}
s32 fade_get_mode(void)
{
    switch (fadeMode) {
    case 1:
        return 1;
    case 2:
        return 2;
    default:
        return 0;
    }
}
void fade_set_enabled(s32 value)
{
    fadeEnabled = value;
}

s32 fade_is_enabled(void)
{
    return fadeEnabled;
}
void fade_start_steps_inverse(s32 value)
{
    fadeStepMode = 2;
    fadeStepSize = value;
    fadeStepsLeft = 255 / value;
    fadeHold = 1;
}

void fade_start_steps(s32 value)
{
    fadeStepMode = 1;
    fadeStepSize = value;
    fadeStepsLeft = 255 / value;
}
s32 fade_steps_done(void)
{
    return fadeStepsLeft == 0;
}
extern POLY_F4 fadeStepQuads[];
extern DR_MODE fadeStepTpages[];
extern u8 frameRecords[];
extern u32 *activeOt; /* nongp */
void display_reset_envs(void);

void screen_fade_step(void)
{
    s32 buf;
    u8 shade;
    POLY_F4 *p;
    DR_MODE *m;

    if (fadeStepsLeft > 0) {
        buf = (u8 *)currentFrame != frameRecords;
        display_reset_envs();
        main_set_put_env(0);
        main_set_clear_screen(0);
        if (fadeHold == 1) {
            fadeHold = 0;
            return;
        }
        if (fadeStepMode == 1) {
            shade = ~(fadeStepSize * fadeStepsLeft);
        } else {
            shade = fadeStepSize;
        }
        p = &fadeStepQuads[buf];
        m = &fadeStepTpages[buf];
        setPolyF4(p);
        setRGB0(p, shade, shade, shade);
        setXY4(p, 0, 0, 640, 0, 0, 480, 640, 480);
        setSemiTrans(p, 1);
        setDrawMode(m, 1, 1, 0x140, 0);
        addPrim(activeOt, p);
        addPrim(activeOt, m);
        if (--fadeStepsLeft >= 0) {
            return;
        }
    }
    fadeStepsLeft = 0;
}
void display_reset_envs(void)
{
    DispEnv disp;
    DrawEnv draw;
    SetDefDispEnv(&disp, 0, 0, 0x280, 0x1E0);
    SetDefDrawEnv(&draw, 0, 0, 0x280, 0x1E0);
    PutDispEnv(&disp);
    PutDrawEnv(&draw);
}
void prim_add_pixel(u32 *ot, s16 x0, s16 y0, s32 color, s16 abe);
void prim_add_line(u32 *ot, s16 x0, s16 y0, s16 x1, s16 y1, s32 color, s16 abe);
void prim_add_gouraud_rect(u32 *ot, s16 x0, s16 y0, s16 w, s16 h, s32 color0, s32 color1,
                           s32 color2, s32 color3, s16 abe);
