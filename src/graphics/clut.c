/* Included by src/gameplay.c; shares its private declarations. */
void clut_fade_setup(ClutSave *fade, u32 *buf, s32 span, s32 x0, s32 y0, s32 x1, s32 y1, s32 dstX,
                     s32 dstY)
{
    s32 base;
    RECT rect;

    fade->buf = buf;
    fade->unk04 = -2;
    fade->unk08 = span;
    fade->unk06 = 0;
    fade->unk0A = 0;
    base = clut_fade_page();
    DrawSync(0);
    rect.x = x0;
    rect.y = y0 + base;
    rect.w = 16;
    rect.h = 1;
    StoreImage(&rect, buf);
    DrawSync(0);
    rect.x = x1;
    rect.y = y1 + base;
    rect.w = 16;
    rect.h = 1;
    StoreImage(&rect, buf + 8);
    DrawSync(0);
    fade->unk0C = dstX;
    fade->unk0E = dstY + base;
    fade->unk10 = 16;
    fade->unk12 = 1;
}
void clut_blend_palettes(void *p, u8 *clut, u8 *pixels, s32 step, s32 mode)
{
    u16 *dst;
    u16 *b;
    u16 *a;
    s32 ca[3];
    s32 cb[3];
    s32 co[3];
    s32 *pb;
    s32 *po;
    s32 i;
    s32 c;

    pb = cb;
    po = co;
    dst = p;
    b = (u16 *)pixels;
    a = (u16 *)clut;

    for (i = 0; i < 16; i++) {
        ca[2] = *a & 0x1F;
        ca[1] = (*a >> 5) & 0x1F;
        ca[0] = (*a >> 10) & 0x1F;
        cb[2] = *b & 0x1F;
        cb[1] = (*b >> 5) & 0x1F;
        cb[0] = (*b >> 10) & 0x1F;
        math_lerp_vec3(step, mode, ca, pb, po);
        c = (u16)co[0] & 0x1F;
        c <<= 5;
        c |= (u16)co[1] & 0x1F;
        c <<= 5;
        c |= (u16)co[2] & 0x1F;
        if (c != 0) {
            c = (s16)(c | 0x8000);
        }
        *dst = c;
        dst++;
        b++;
        a++;
    }
}
typedef struct {
    u8 *data;
    s16 dir;
    s16 pad06;
    s16 mode;
    s16 frame;
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} LoadImageJob;
extern void SetDrawLoad(void *p, RECT *rect);
extern void clut_blend_palettes(void *p, u8 *clut, u8 *pixels, s32 arg3, s32 mode);
void clut_scroll_step(LoadImageJob *job, s32 step, s32 pingpong)
{
    RECT rect;
    u8 *p;

    if (job->mode <= 0) {
        return;
    }
    if (job->frame < 0) {
        job->frame = 0;
    }
    if (job->dir < 0) {
        job->dir = 0;
    }
    if (job->mode < step) {
        step = job->mode;
    }
    if (pingpong == 1) {
        if (job->dir != 0) {
            job->frame -= step;
            if (job->frame < 0) {
                job->frame = 0;
                job->dir = 0;
            }
        } else {
            job->frame += step;
            if (job->mode < job->frame) {
                job->frame = job->mode;
                job->dir = 1;
            }
        }
    } else {
        job->frame += step;
        if (job->frame >= job->mode) {
            job->frame -= job->mode;
        }
    }
    rect.x = job->x;
    rect.y = job->y;
    rect.w = job->w;
    rect.h = job->h;
    p = ot_insert_prim(0, 0x44);
    SetDrawLoad(p, &rect);
    clut_blend_palettes(p + 0x10, job->data, job->data + 0x20, job->frame, job->mode);
}

void clut_fade_step(LoadImageJob *job, s32 value)
{
    RECT rect;
    u8 *p;

    rect.x = job->x;
    rect.y = job->y;
    rect.w = job->w;
    rect.h = job->h;
    p = ot_insert_prim(0, 0x44);
    SetDrawLoad(p, &rect);
    clut_blend_palettes(p + 0x10, job->data, job->data + 0x20, value, job->mode);
}
s32 clut_next_serial(void)
{
    return clutSerial++;
}
s32 clut_fade_page(void)
{
    return 0;
}
