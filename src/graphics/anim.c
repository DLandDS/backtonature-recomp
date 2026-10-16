/* Included by src/archive.c; shares its private declarations. */
/* Debug $ra capture into a stack slot. It must be TWO asm statements, as in
 * the SetSpadStack idiom in gameplay.c: the operand is copied into $t0 by the
 * first and stored through by the second. A single asm containing both the
 * copy and the store makes cc1 allocate the address in $v1 instead of $v0. */
void anim_bind_file(ArchiveEntry *state, ArchiveHeader *file)
{
    void *raSlot;

    __asm__ volatile("move $8, %0" : : "r"(&raSlot) : "$8", "memory");
    __asm__ volatile("sw $31, 0($8)" : : : "$8", "memory");

    if ((u32)file != (((u32)file >> 2) << 2)) {
        FntPrint(storage800100A0);
        while (host_poll()) {
        }
    }
    if (file->unk00 != 0xE) {
        FntPrint(storage800100A0);
        while (host_poll()) {
        }
    }
    state->unk06 = 0;
    state->flags = 0;
    state->unk02 = 0;
    state->unk04 = 0;
    state->unk05 = 0;
    state->unk08 = 0x1000;
    state->unk0C = 0;
    state->unk0E = 0;
    state->unk10 = 0;
    state->unk12 = 0;
    state->unk14 = 0;
    state->unk18 = 0;
    state->unk1C = (u8 *)file + file->unk10;
    state->unk20 = (u8 *)file + file->unk14;
    state->unk24 = (u8 *)file + file->unk18;
    state->unk28 = (u8 *)file + file->unk1C;
    state->unk2C = (u8 *)file + file->unk20;
    anim_set_source(state, file, 0);
}
void bits16_set(ArchiveEntry *flags, u16 bits)
{
    flags->flags |= bits;
}
void bits16_clear(ArchiveEntry *flags, u16 bits)
{
    flags->flags &= (u16)~bits;
}
void anim_select_range(ArchiveEntry *state, u8 anim)
{
    state->unk04 = anim;
    state->unk05 = 0;
    state->unk06 = 0;
    state->unk02 &= 0xFCFF;
}
void anim_set_frame(ArchiveEntry *state, u8 frame)
{
    state->unk05 = frame;
    state->unk06 = 0;
}
void anim_set_offsets(ArchiveEntry *state, s16 pageX, s16 pageY, s16 clutX, s16 clutY)
{
    state->unk0C = pageX;
    state->unk0E = pageY;
    state->unk10 = clutX;
    state->unk12 = clutY;
}
void anim_set_source(ArchiveEntry *state, void *source, void *bank)
{
    state->unk14 = source;
    state->unk18 = bank;
}

void anim_stub_1(void) {}

void anim_set_bank(ArchiveEntry *state, void *bank)
{
    state->unk18 = bank;
}
/* Round 20 bounded lifetime experiment: block-scoped base and offset locals
 * retained the exact 15-instruction count but left eight differences at
 * +0x0C..+0x28. Both retail and candidate are leaf functions with no frame
 * or saved registers. The independent unk05 load still moved ahead of the
 * offset load, so expression scope did not control this load schedule. */
/* The `- (-(...))` is load-bearing, and it is the whole reason this function
 * matches. Written the obvious way, as `+ (arg0->unk04 * 4)`, cc1 hoists the
 * independent unk05 load ahead of the offset load and eight words differ from
 * +0x0C -- the bounded result this stub carried from round 20 to round 28.
 *
 * Round 20 concluded that expression *scope* did not control the load
 * schedule, and that was correct. What controls it is expression *shape*:
 * negating a negated subexpression changes the order cc1 evaluates the operands
 * in, and the loads follow. This is the first lever found that moves cc1
 * 2.95.2 load scheduling at all.
 *
 * Found by decomp-permuter, which reached score 0 in 42 iterations from the
 * plain-`+` form; see tools/permuter_setup.py. The permuter output also
 * wrapped the body in `if (1)` and lifted the literal 7 into a variable, and
 * neither is needed -- this is the minimized form, and it is the one verified
 * byte-identical. */
u32 anim_frame_jumps(ArchiveEntry *state)
{
    u8 *base;

    base = (u8 *)state->unk2C + *(u16 *)((u8 *)state->unk28 - (-(state->unk04 * 4)) + 2) +
           state->unk05 * 10;
    return base[7] >> 7;
}
void anim_set_tick_flag(void *flag)
{
    animScreenFlag = flag;
}
/* Ported from hmbtn-decomp (CC0), anim_tick. */
void anim_tick(ArchiveEntry *state)
{
    AnimRange *anim;
    AnimFrame *frame;
    u16 flags;
    s16 count;
    s32 ticks;
    u8 next;

    if (state->flags & 4) {
        return;
    }
    anim = &((AnimRange *)state->unk28)[state->unk04];
    frame = &((AnimFrame *)((u8 *)state->unk2C + anim->offset))[state->unk05];
    flags = state->unk02 & ~(0x100 | 0x200);
    state->unk02 = flags;
    state->unk06++;
    count = state->unk06;
    ticks = (frame->duration + 1U) >> 1;
    if (count >= ticks) {
        state->unk06 = 0;
        next = ++state->unk05;
        if (next >= anim->frames) {
            flags |= 0x100;
            state->unk02 = flags;
            if (!(frame->control & 0x80)) {
                state->unk05 = frame->control & 0x7F;
                state->unk02 = (flags & ~0x100) | 0x200;
            }
        }
        if (frame->control != 0) {
            if (frame->control == 0xFF) {
                state->unk06 = ticks;
                state->unk05--;
                state->unk02 |= 0x100;
            } else if (frame->control & 0x80) {
                state->unk05 = frame->control & 0x7F;
                state->unk02 = (state->unk02 & ~0x100) | 0x200;
            }
        }
    }
    animScreenFlag = (void *)1;
}

u16 anim_range_frames(ArchiveEntry *state, s32 index)
{
    return *(u16 *)((u8 *)state->unk28 + ((index << 16) >> 14));
}
u16 anim_part_image(ArchiveEntry *state, u8 first, u8 second)
{
    u16 *base;

    base = (u16 *)((u8 *)state->unk2C + *(u16 *)((u8 *)state->unk28 - (-(first * 4)) + 2) +
                   second * 10);
    if (*base > 0xFFFD) {
        return 0;
    }
    return *(u16 *)((u8 *)state->unk24 + *(u16 *)((u8 *)state->unk20 - (-(*base * 4)) + 2));
}

u16 anim_part_palette(ArchiveEntry *state, u8 first, u8 second)
{
    u16 *base;

    base = (u16 *)((u8 *)state->unk2C + *(u16 *)((u8 *)state->unk28 - (-(first * 4)) + 2) +
                   second * 10);
    if (*base > 0xFFFD) {
        return 0;
    }
    return *(u16 *)((u8 *)state->unk24 + *(u16 *)((u8 *)state->unk20 - (-(*base * 4)) + 2) + 2);
}
extern int bcmp(const void *a, const void *b, u32 n);
extern s32 host_poll(void);

u16 anim_find_bank_image(void *archive, void *entry, u16 id)
{
    u8 *e;
    s32 i;
    s32 n;
    u32 len;

    e = (u8 *)entry + 4;
    n = *(u16 *)((u8 *)entry + 2);
    for (i = 0; i < n; i++) {
        len = *(u32 *)e & 0xFFFFFF;
        if (bcmp(*(u8 **)((u8 *)archive + 0x1C) + id * 8, e + 4, 8) == 0) {
            break;
        }
        e += ((len + 3) >> 2 << 2) + 0xC;
    }
    if (i == n) {
        while (host_poll() != 0) {
        }
    }
    return i;
}
/* B1 layer-2 group: the helper declarations above pass caller staging. Each
 * member keeps its entry pointer in s0 across the helper and 158E4 calls. */
void anim_draw_ft4_advance(void *ot, void *entry, s16 x, s16 y)
{
    anim_draw_ft4_cursor(ot, entry, ((ArchiveEntry *)entry)->unk04, ((ArchiveEntry *)entry)->unk05,
                         x, y, ((ArchiveEntry *)entry)->unk08, 0, 0);
    anim_tick(entry);
}
void anim_draw_sprt_advance(void *ot, void *entry, s16 x, s16 y)
{
    anim_draw_sprt_cursor(ot, entry, ((ArchiveEntry *)entry)->unk04, ((ArchiveEntry *)entry)->unk05,
                          x, y);
    anim_tick(entry);
}
s32 anim_draw_ft4_prim_advance(void *ot, void *prim, void *entry, s16 x, s16 y)
{
    s32 result;

    result = anim_draw_ft4(ot, prim, entry, ((ArchiveEntry *)entry)->unk04,
                           ((ArchiveEntry *)entry)->unk05, x, y,
                           (void *)((ArchiveEntry *)entry)->unk08, 0, 0, -1, -1);
    anim_tick(entry);
    return result;
}
void anim_draw_sprt_slots_advance(void *ot, void *sprites, void *modes, void *entry, s16 x, s16 y)
{
    anim_draw_sprt(ot, sprites, modes, entry, ((ArchiveEntry *)entry)->unk04,
                   ((ArchiveEntry *)entry)->unk05, x, y);
    anim_tick(entry);
}
void anim_draw_ft4_sized_advance(void *ot, void *entry, s16 x, s16 y, void *scale,
                                 void *keepPosition, u16 flags)
{
    anim_draw_ft4_cursor(ot, entry, ((ArchiveEntry *)entry)->unk04, ((ArchiveEntry *)entry)->unk05,
                         x, y, scale, keepPosition, flags);
    anim_tick(entry);
}
/* The shared context boot.c owns (BootContext there); this unit reads only
 * its +0x118 pointer. A small extern retail reaches without $gp. */
extern u8 *currentFrame; /* nongp */

s32 text_queue_sprite(void *ot, void *sprite, s16 x, s16 y, s32 unused4, s32 unused5, s32 unused6,
                      void *base, void *places)
{
    text_sprite_draw(ot, *(void **)(currentFrame + 0x118), sprite, x, y, 0x40, base, places, -1, -1,
                     -1);
    return 0;
}
void anim_draw_sprt(void *ot, void *sprites, void *modes, void *sprite, u8 anim, u8 frame, s16 x,
                    s16 y)
{
    anim_draw_sprt_parts(ot, sprites, modes, sprite, anim, frame, x, y);
}
/* B1 first form: anim_draw_sprt matched exactly (15 instructions). */
s32 anim_draw_ft4(void *ot, void *prim, void *sprite, u8 anim, u8 frame, s16 x, s16 y, void *scale,
                  void *keepPosition, u16 flags, s16 pageX, s16 pageY)
{
    s32 minusOne = -1;
    return sprite_draw_frame(ot, prim, sprite, anim, frame, x, y, scale, scale, keepPosition, flags,
                             pageX, pageY, minusOne, minusOne);
}
void anim_draw_sprt_cursor(void *ot, void *sprite, u8 anim, u8 frame, s16 x, s16 y)
{
    void *scratch = gs_work_alloc(0);

    anim_draw_sprt_parts(ot, &scratch, &scratch, sprite, anim, frame, x, y);
    gs_work_release(scratch);
}
void anim_draw_ft4_cursor(void *ot, void *sprite, u8 anim, u8 frame, s16 x, s16 y, void *scale,
                          void *keepPosition, u16 flags)
{
    gs_work_release((void *)sprite_draw_frame(ot, gs_work_alloc(0), sprite, anim, frame, x, y,
                                              scale, scale, keepPosition, flags, -1, -1, -1, -1));
}
extern void *storage8006C84C[];
void sprite_draw_traced(void *ot, void *prim, void *sprite, u8 anim, u8 frame, s16 x, s16 y,
                        void *scale, void *unused, u16 flags)
{
    __asm__ volatile("addu $8, %0, $0\n\tsw $31, 0($8)" : : "r"(storage8006C84C) : "$8");
    sprite_draw_frame(ot, prim, sprite, anim, frame, x, y, scale, scale, 0, flags, -1, -1, -1, -1);
}
void sprite_draw_at_cursor(void *ot, void *sprite, u8 anim, u8 frame, s16 x, s16 y, void *scale,
                           void *keepPosition, u16 flags, s16 pageX, s16 pageY, s16 clutX,
                           s16 clutY)
{
    gs_work_release((void *)sprite_draw_frame(ot, gs_work_alloc(0), sprite, anim, frame, x, y,
                                              scale, scale, keepPosition, flags, pageX, pageY,
                                              clutX, clutY));
}
void sprite_draw_at_cursor_simple(void *ot, void *sprite, u8 anim, u8 frame, s16 x, s16 y,
                                  void *scaleX, void *scaleY, u16 flags)
{
    gs_work_release((void *)sprite_draw_frame(ot, gs_work_alloc(0), sprite, anim, frame, x, y,
                                              scaleX, scaleY, 0, flags, -1, -1, -1, -1));
}
/* Sprite-part renderer. The CLUT offsets go through ONE shared temporary
 * (off): retail reloads e->unk10 into the register that held e->unk12,
 * which only happens when both values live in the same pseudo, so cc1
 * cannot hoist the second load above the first add. reserved[8] keeps
 * retail's 40-byte frame; the double negation in the frame lookup is the
 * load-order lever from anim_frame_jumps. */
typedef struct SpriteFrame {
    u16 image;
    s16 x, y;
    u8 tail[4];
} SpriteFrame;
typedef struct SpritePart {
    u16 texture, clut;
    s16 x, y;
    u8 flags;
    u8 tail[3];
} SpritePart;
void anim_draw_sprt_parts(void *arg0, void *arg1, void *arg2, void *state, u8 group, u8 index,
                          s16 x, s16 y)
{
    volatile u8 reserved[8];
    u32 *ot = arg0;
    SPRT **sprites = arg1;
    DR_MODE **modes = arg2;
    ArchiveEntry *e = state;
    SpriteFrame *frame;
    SpritePart *part;
    s16 *texture;
    s16 *clut;
    s16 *lookup;
    s32 count;
    s16 texU;
    s32 dx;
    s32 dy;
    s32 u;
    s32 v;
    s32 w;
    s32 h;
    s32 depth;
    s32 texY;
    s32 palY, palX;
    s32 off;

    frame =
        (SpriteFrame *)((((u8 *)e->unk2C) + (*((u16 *)((((u8 *)e->unk28) - (-(group * 4))) + 2)))) +
                        (index * 10));
    if (frame->image > 0xFFFD) {
        return;
    }
    lookup = ((s16 *)e->unk20) + (frame->image * 2);
    dx = frame->x;
    dy = frame->y;
    part = (SpritePart *)((u8 *)e->unk24 + ((u16 *)lookup)[1]);
    part += lookup[0] - 1;
    for (count = lookup[0] - 1; count >= 0; count--) {
        texture = ((s16 *)e->unk1C) + (part->texture * 4);
        u = (((u16 *)texture)[0] + e->unk0C) & 0x3F;
        w = texture[2];
        clut = ((s16 *)e->unk1C) + (part->clut * 4);
        h = texture[3];
        v = (((u16 *)texture)[1] + e->unk0E) & 0xFF;
        depth = part->flags >> 7;
        if (depth) {
            u *= 2;
            w = (s16)(w * 2);
        } else {
            u = (s16)(u * 4);
            w = (s16)(w * 4);
        }
        setlen(*sprites, 4), setcode(*sprites, 0x64);
        *((u32 *)(&(*sprites)->r0)) = 0x64808080;
        if ((e->unk02 & 0x20) || ((part->flags >> 4) & 1))
            setSemiTrans(*sprites, 1);
        else
            setSemiTrans(*sprites, 0);
        (*sprites)->code &= 0xFE;
        (*sprites)->x0 = ((u16)part->x) + (x + dx);
        (*sprites)->y0 = ((u16)part->y) + (y + dy);
        (*sprites)->w = w;
        (*sprites)->h = h;
        (*sprites)->u0 = u;
        (*sprites)->v0 = v;
        palY = ((u16 *)clut)[1];
        off = e->unk12;
        palY += off;
        palX = clut[0];
        off = (s16)e->unk10;
        palX = palX + off;
        (*sprites)->clut = (palY << 6) | ((palX >> 4) & 0x3F);
        addPrim(ot, *sprites);
        *sprites = (SPRT *)(((u8 *)(*sprites)) + 20);
        setlen(*modes, 1);
        texU = (s16)e->unk0C;
        texY = texture[1] + (s16)e->unk0E;
        (*modes)->code[0] = (depth << 7) | ((texY & 0x100) >> 4) |
                            (((texture[0] + texU) & 0x3C0) >> 6) | ((texY & 0x200) << 2) |
                            0xE1000600;
        addPrim(ot, *modes);
        *modes = (DR_MODE *)(((u8 *)(*modes)) + 12);
        part--;
    }
}
/* Ported from hmbtn-decomp (CC0), sprite_draw_frame. */
extern u8 *currentFrame;
extern POLY_FT4 *sprite_draw_frame(u_long *arg0, POLY_FT4 *arg1, AnimState *arg2, u8 arg3, u8 arg4,
                                   s16 arg5, s16 arg6, s32 arg7, s32 arg8, s32 arg9, u16 arg10,
                                   s16 arg11, s16 arg12, s16 arg13, s16 arg14);
static inline void setOrientedUV(POLY_FT4 *prim, s32 u, s32 v, s32 w, s32 h, s16 orient)
{
    if (orient == 0) {
        prim->u0 = u;
        prim->v0 = v;
        prim->u1 = u + (w - 1);
        prim->v1 = v;
        prim->u2 = u;
        prim->v2 = v + (h - 1);
        prim->u3 = u + (w - 1);
        prim->v3 = v + (h - 1);
    } else if (orient == 1) {
        prim->u0 = u + (w - 1);
        prim->v0 = v;
        prim->u1 = u;
        prim->v1 = v;
        prim->u2 = u + (w - 1);
        prim->v2 = v + (h - 1);
        prim->u3 = u;
        prim->v3 = v + (h - 1);
    } else if (orient == 2) {
        prim->u0 = u;
        prim->v0 = v + (h - 1);
        prim->u1 = u + (w - 1);
        prim->v1 = v + (h - 1);
        prim->u2 = u;
        prim->v2 = v;
        prim->u3 = u + (w - 1);
        prim->v3 = v;
    } else {
        prim->u0 = u + (w - 1);
        prim->v0 = v + (h - 1);
        prim->u1 = u;
        prim->v1 = v + (h - 1);
        prim->u2 = u + (w - 1);
        prim->v2 = v;
        prim->u3 = u;
        prim->v3 = v;
    }
}

POLY_FT4 *sprite_draw_frame(u_long *ot, POLY_FT4 *prim, AnimState *sprite, u8 anim, u8 frameIndex,
                            s16 x, s16 y, s32 scaleX, s32 scaleY, s32 keepPosition, u16 flags,
                            s16 pageX, s16 pageY, s16 clutX, s16 clutY)
{
    AnimFrame *frame;
    AnimPart *part;
    AnimImage *tex;
    AnimImage *palette;
    s16 n;
    s32 count;
    s32 vx, vy;
    s16 px, py;
    s32 u, v, h;
    s32 tv;
    s32 left, right, top, bottom;
    s16 x0, x1, y0, y1;
    u8 attr;
    s32 depth;
    s32 orient;

    frame = &((AnimFrame *)((sprite)->frames + (&sprite->ranges[anim])->offset))[frameIndex];
    if (frame->parts > 0xFFFD) {
        return prim;
    }
    count = sprite->partLists[frame->parts].count;
    part = ((AnimPart *)((sprite)->parts + (&sprite->partLists[frame->parts])->offset));
    part += count - 1;
    for (n = count - 1; n >= 0; n--) {
        tex = &sprite->images[part->image];
        palette = &sprite->images[part->palette];
        (((P_TAG *)(prim))->len = (u_char)(9)), (((P_TAG *)(prim))->code = (u_char)(0x2c));
        (((sprite->flags & ANIM_FLAG_SEMI_TRANSPARENT) || ((part->attr >> 4) & 1))
             ? (((P_TAG *)(prim))->code = (u_char)((u_char)(((P_TAG *)(prim))->code) | 0x02))
             : (((P_TAG *)(prim))->code = (u_char)((u_char)(((P_TAG *)(prim))->code) & ~0x02)));
        prim->r0 = 0x80;
        prim->g0 = 0x80;
        prim->b0 = 0x80;
        ((0) ? (((P_TAG *)(prim))->code = (u_char)((u_char)(((P_TAG *)(prim))->code) | 0x01))
             : (((P_TAG *)(prim))->code = (u_char)((u_char)(((P_TAG *)(prim))->code) & ~0x01)));
        if (clutX == -1) {
            vx = palette->x + sprite->clutOffX;
            vy = palette->y + sprite->clutOffY;
        } else {
            vx = clutX;
            vy = clutY;
        }
        prim->clut = (((vy) << 6) | (((vx) >> 4) & 0x3f));
        attr = part->attr;
        depth = attr >> 7;
        orient = attr & 3;
        if (pageX == -1) {
            vx = tex->x + sprite->pageOffX;
            px = vx & ~0x3F;
            u = vx & 0x3F;
            vx = tex->y + sprite->pageOffY;
            py = vx & ~0xFF;
            v = vx & 0xFF;
        } else {
            u = pageX;
            v = pageY;
            px = u;
            py = v;
        }
        vx = u & 0x3F;
        prim->tpage = ((((depth) & 0x3) << 7) | (((0) & 0x3) << 5) | (((py) & 0x100) >> 4) |
                       (((px) & 0x3ff) >> 6) | (((py) & 0x200) << 2));
        px = tex->w;
        h = tex->h;
        tv = v & 0xFF;
        if (depth == 0) {
            vx = vx << 2;
            px = px << 2;
        } else {
            vx = (s16)(vx << 1);
            px = px << 1;
        }
        setOrientedUV(prim, vx, tv, px, h, orient);
        if (keepPosition == 0) {
            left = frame->dx + part->dx;
            right = left + px;
            top = frame->dy + part->dy;
            bottom = top + h;
            if (scaleX + scaleY != 0x1000 * 2) {
                x0 = x + left * scaleX / 0x1000;
                x1 = x + right * scaleX / 0x1000;
                y0 = y + top * scaleY / 0x1000;
                y1 = y + bottom * scaleY / 0x1000;
            } else {
                x0 = x + left;
                x1 = x + right;
                y0 = y + top;
                y1 = y + bottom;
            }
            if (flags & (ANIM_DRAW_FLIP_X | ANIM_DRAW_FLIP_Y)) {
                prim->x0 = x1;
                prim->y0 = y1;
                prim->x1 = x0;
                prim->y1 = y1;
                prim->x2 = x1;
                prim->y2 = y0;
                prim->x3 = x0;
                prim->y3 = y0;
            } else if (flags & ANIM_DRAW_FLIP_X) {
                prim->x0 = x1;
                prim->y0 = y0;
                prim->x1 = x0;
                prim->y1 = y0;
                prim->x2 = x1;
                prim->y2 = y1;
                prim->x3 = x0;
                prim->y3 = y1;
            } else if (flags & ANIM_DRAW_FLIP_Y) {
                prim->x0 = x0;
                prim->y0 = y1;
                prim->x1 = x1;
                prim->y1 = y1;
                prim->x2 = x0;
                prim->y2 = y0;
                prim->x3 = x1;
                prim->y3 = y0;
            } else {
                prim->x0 = x0;
                prim->y0 = y0;
                prim->x1 = x1;
                prim->y1 = y0;
                prim->x2 = x0;
                prim->y2 = y1;
                prim->x3 = x1;
                prim->y3 = y1;
            }
        }
        if (flags & ANIM_DRAW_CURSOR) {
            (*(u32 *)(prim) = ((u32)(sizeof(POLY_FT4) / 4 - 1) << 24) | (*(ot) & 0xFFFFFF),
             *(ot) = (u32)(prim) & 0xFFFFFF);
            currentFrameView->primCursor.ft4++;
        } else {
            (((P_TAG *)(prim))->addr = (u_long)((u_long)(((P_TAG *)(ot))->addr))),
                (((P_TAG *)(ot))->addr = (u_long)(prim));
        }
        prim++;
        part--;
    }
    return prim;
}

s32 image_bank_upload(u8 *entry, void *dest, u16 size, s16 x, s16 y);
extern s32 func_80015B24_raw(void *, void *, u16) __asm__("func_80015B24");
s32 sprite_measure_text(ArchiveEntry *e, s32 id, void *dest, s32 x, s32 y)
{
    s32 size;
    size = image_bank_upload(e->unk18, dest, (u16)func_80015B24_raw(e, e->unk18, (u16)id), (s16)x,
                             (s16)y);
    return (size / 4) * 4 + 0x20;
}
/* Ported from hmbtn-decomp (CC0), text_sprite_draw. */
extern u8 *currentFrame;
s32 sprite_measure_text(ArchiveEntry *e, s32 id, void *dest, s32 x, s32 y);
extern POLY_FT4 *text_sprite_draw(u_long *arg0, POLY_FT4 *arg1, AnimState *arg2, s16 arg3, s16 arg4,
                                  u16 arg5, u8 *arg6, VramPlace *arg7, s16 arg8, s16 arg9,
                                  s16 arg10);
extern s16 textSpriteKey;

POLY_FT4 *text_sprite_draw(u_long *ot, POLY_FT4 *prim, AnimState *sprite, s16 x, s16 y, u16 flags,
                           u8 *base, VramPlace *places, s16 matchPlace, s16 matchKey,
                           s16 replaceKey)
{
    AnimFrame *frame;
    s32 count;
    AnimPart *part;
    AnimImage *tex;
    VramPlace *place;
    s16 seen[10];
    s32 seenCount;
    s32 n;
    s32 i;
    s32 k;
    s32 found;
    s32 width;
    s16 u, v, w, h;
    s16 left, right, top, bottom;
    s32 code;
    s16 pageX;
    s16 pageY;
    u8 attr;
    s32 depth;
    s32 orient;
    s32 originX;
    s32 originY;

    frame = &((AnimFrame *)((sprite)->frames +
                            (&sprite->ranges[sprite->animation])->offset))[sprite->frame];
    if (frame->parts > 0xFFFD) {
        return prim;
    }
    seenCount = 0;
    for (k = 0; k < 10; k++) {
        seen[k] = -1;
    }
    count = sprite->partLists[frame->parts].count;
    part = ((AnimPart *)((sprite)->parts + (&sprite->partLists[frame->parts])->offset));
    part += count - 1;
    for (n = count - 1; n >= 0; n--, prim++, part--) {
        width = 0;
        found = 0;
        tex = &sprite->images[part->image];
        place = &places[part->place - 0x80];
        code = part->place - 128;
        for (i = 0; i < 10; i++) {
            if (seen[i] == code) {
                found = 1;
                break;
            }
        }
        if (!found) {
            seen[seenCount++] = part->place - 128;
            width = sprite_measure_text(sprite, part->image, base, place->pageX, place->pageY);
            textSpriteKeyView = part->palette;
            if (matchPlace != -1) {
                if (part->place == 0x80) {
                    textSpriteKeyView = matchPlace;
                }
                if (part->palette == matchKey) {
                    textSpriteKeyView = replaceKey;
                }
            }
            width += sprite_measure_text(sprite, textSpriteKeyView, base + width, place->clutX,
                                         place->clutY);
        }
        (((P_TAG *)(prim))->len = (u_char)(9)), (((P_TAG *)(prim))->code = (u_char)(0x2c));
        (((part->attr >> 4) & 1)
             ? (((P_TAG *)(prim))->code = (u_char)((u_char)(((P_TAG *)(prim))->code) | 0x02))
             : (((P_TAG *)(prim))->code = (u_char)((u_char)(((P_TAG *)(prim))->code) & ~0x02)));
        prim->r0 = 0x80;
        prim->g0 = 0x80;
        prim->b0 = 0x80;
        ((0) ? (((P_TAG *)(prim))->code = (u_char)((u_char)(((P_TAG *)(prim))->code) | 0x01))
             : (((P_TAG *)(prim))->code = (u_char)((u_char)(((P_TAG *)(prim))->code) & ~0x01)));
        prim->clut = (((place->clutY) << 6) | (((place->clutX) >> 4) & 0x3f));
        pageX = place->pageX;
        attr = part->attr;
        pageY = place->pageY;
        depth = attr >> 7;
        prim->tpage = ((((depth) & 0x3) << 7) | (((0) & 0x3) << 5) | (((pageY) & 0x100) >> 4) |
                       (((pageX) & 0x3ff) >> 6) | (((pageY) & 0x200) << 2));
        u = pageX & 0x3F;
        w = tex->w;
        orient = attr & 3;
        if (depth == 0) {
            u = u << 2;
            w = w << 2;
        } else {
            u = u << 1;
            w = w << 1;
        }
        v = pageY & 0xFF;
        h = tex->h;
        if (orient == 0) {
            prim->u0 = u;
            prim->v0 = v;
            prim->u1 = u + (w - 1);
            prim->v1 = v;
            prim->u2 = u;
            prim->v2 = v + (h - 1);
            prim->u3 = u + (w - 1);
            prim->v3 = v + (h - 1);
        } else if (orient == 1) {
            prim->u0 = u + (w - 1);
            prim->v0 = v;
            prim->u1 = u;
            prim->v1 = v;
            prim->u2 = u + (w - 1);
            prim->v2 = v + (h - 1);
            prim->u3 = u;
            prim->v3 = v + (h - 1);
        } else if (orient == 2) {
            prim->u0 = u;
            prim->v0 = v + (h - 1);
            prim->u1 = u + (w - 1);
            prim->v1 = v + (h - 1);
            prim->u2 = u;
            prim->v2 = v;
            prim->u3 = u + (w - 1);
            prim->v3 = v;
        } else {
            prim->u0 = u + (w - 1);
            prim->v0 = v + (h - 1);
            prim->u1 = u;
            prim->v1 = v + (h - 1);
            prim->u2 = u + (w - 1);
            prim->v2 = v;
            prim->u3 = u;
            prim->v3 = v;
        }
        originX = x + (u16)frame->dx;
        left = part->dx + originX;
        right = left + w;
        originY = y + (u16)frame->dy;
        top = part->dy + originY;
        bottom = top + h;
        if (flags & (ANIM_DRAW_FLIP_X | ANIM_DRAW_FLIP_Y)) {
            prim->x0 = right;
            prim->y0 = bottom;
            prim->x1 = left;
            prim->y1 = bottom;
            prim->x2 = right;
            prim->y2 = top;
            prim->x3 = left;
            prim->y3 = top;
        } else if (flags & ANIM_DRAW_FLIP_X) {
            prim->x0 = right;
            prim->y0 = top;
            prim->x1 = left;
            prim->y1 = top;
            prim->x2 = right;
            prim->y2 = bottom;
            prim->x3 = left;
            prim->y3 = bottom;
        } else if (flags & ANIM_DRAW_FLIP_Y) {
            prim->x0 = left;
            prim->y0 = bottom;
            prim->x1 = right;
            prim->y1 = bottom;
            prim->x2 = left;
            prim->y2 = top;
            prim->x3 = right;
            prim->y3 = top;
        } else {
            prim->x0 = left;
            prim->y0 = top;
            prim->x1 = right;
            prim->y1 = top;
            prim->x2 = left;
            prim->y2 = bottom;
            prim->x3 = right;
            prim->y3 = bottom;
        }
        (*(u32 *)(prim) = ((u32)(sizeof(POLY_FT4) / 4 - 1) << 24) | (*(ot) & 0xFFFFFF),
         *(ot) = (u32)(prim) & 0xFFFFFF);
        base += width;
        currentFrameView->primCursor.ft4++;
    }
    return prim;
}
