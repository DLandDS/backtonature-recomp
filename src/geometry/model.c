/* Included by src/gameplay.c; shares its private declarations. */
void camera_get_view(MATRIX *view)
{
    *view = viewMatrix;
}

void camera_yaw_matrix(MATRIX *out)
{
    u16 v;
    out->m[0][0] = viewMatrix.m[0][0];
    out->m[0][1] = viewMatrix.m[0][1];
    out->m[0][2] = viewMatrix.m[0][2];
    out->m[2][0] = viewMatrix.m[0][2];
    out->m[2][1] = viewMatrix.m[0][1];
    v = viewMatrix.m[0][0];
    out->m[1][0] = 0;
    out->m[1][1] = 0x1000;
    out->m[1][2] = 0;
    out->m[2][2] = -v;
}
void model_save_gte_rot(void)
{
    gte_ReadRotMatrix(savedMatrix);
}
void model_compose_gte_matrix(MATRIX *m)
{
    gte_SetRotMatrix((MATRIX *)savedMatrix);
    gte_ldclmv(&m->m[0][0]);
    gte_mvmva(1, 0, 3, 3, 0);
    gte_stclmv(&((MATRIX *)savedMatrix)->m[0][0]);
    gte_ldclmv(&m->m[0][1]);
    gte_mvmva(1, 0, 3, 3, 0);
    gte_stclmv(&((MATRIX *)savedMatrix)->m[0][1]);
    gte_ldclmv(&m->m[0][2]);
    gte_mvmva(1, 0, 3, 3, 0);
    gte_stclmv(&((MATRIX *)savedMatrix)->m[0][2]);
    gte_SetTransMatrix((MATRIX *)savedMatrix);
    gte_ldlv0(m->t);
    gte_mvmva(1, 0, 0, 0, 0);
    gte_stlvl(((MATRIX *)savedMatrix)->t);
    gte_SetRotMatrix((MATRIX *)savedMatrix);
    gte_SetTransMatrix((MATRIX *)savedMatrix);
}
extern MATRIX colorMatrix;
extern MATRIX lightMatrix;

void model_set_light_dir(s32 index, SVECTOR *dir, u8 r, u8 g, u8 b)
{
    SVECTOR n;

    VectorNormalSS(dir, &n);
    lightMatrix.m[index][0] = n.vx;
    colorMatrix.m[0][index] = r << 4;
    colorMatrix.m[1][index] = g << 4;
    lightMatrix.m[index][1] = n.vy;
    lightMatrix.m[index][2] = n.vz;
    colorMatrix.m[2][index] = b << 4;
}
void model_set_light_angles(s32 index, s16 rx, s16 ry, s16 rz, u8 r, u8 g, u8 b)
{
    SVECTOR rot;
    MATRIX m;

    rot.vx = rx;
    rot.vy = ry;
    rot.vz = rz;
    m = GsIDMATRIX;
    m.m[0][0] = -m.m[0][0];
    m.m[1][1] = -m.m[1][1];
    m.m[2][2] = -m.m[2][2];
    RotMatrixX(rx, &m);
    RotMatrixY(ry, &m);
    lightMatrix.m[index][0] = m.m[2][0];
    lightMatrix.m[index][1] = m.m[2][1];
    lightMatrix.m[index][2] = m.m[2][2];
    colorMatrix.m[0][index] = r << 4;
    colorMatrix.m[1][index] = g << 4;
    colorMatrix.m[2][index] = b << 4;
}
void model_set_back_color(u8 r, u8 g, u8 b)
{
    backColor = r;
    storage8005E2DD = g;
    storage8005E2DE = b;
}

void model_set_fog_color(u8 r, u8 g, u8 b)
{
    farColor = r;
    storage8005E2E1 = g;
    storage8005E2E2 = b;
}

void model_set_fog_near(s32 value)
{
    fogNear = value;
}
void model_bind_tmd(s32 *model, s32 *file)
{
    s32 value;

    model[0] = (s32)file;
    value = file[2];
    if (value >= 2) {
        model[1] = (s32)((u8 *)file + (((value * 7) << 2) + 0xC));
    } else {
        model[1] = 0;
    }
}
void model_unbind(s32 *model)
{
    model[0] = 0;
    model[1] = 0;
}
/* Scratchpad stack swap (the PSX community SetSpadStack idiom): park the
 * caller sp at the top of the 1 KB scratchpad and run on it. */
#define SetSpadStack(addr)                                                                         \
    {                                                                                              \
        __asm__ volatile("move $8, %0" : : "r"(addr) : "$8", "memory");                            \
        __asm__ volatile("sw $29, 0($8)" : : : "$8", "memory");                                    \
        __asm__ volatile("addiu $8, $8, -4" : : : "$8", "memory");                                 \
        __asm__ volatile("move $29, $8" : : : "$8", "memory");                                     \
    }
#define ResetSpadStack()                                                                           \
    {                                                                                              \
        __asm__ volatile("addiu $29, $29, 4" : : : "$29", "memory");                               \
        __asm__ volatile("lw $29, 0($29)" : : : "$29", "memory");                                  \
    }

typedef struct ModelPart {
    u8 data[0x1C];
} ModelPart;
typedef struct ModelPartList {
    s32 unk00;
    s32 unk04;
    s32 count;
    ModelPart parts[1];
} ModelPartList;
extern ModelPartList *currentTmd;
extern void PopMatrix(void);
void model_apply_scale(void);
void model_draw_tmd_object(ModelPart *first, ModelPart *part);

void model_draw(ModelPartList **model)
{
    s32 i;

    currentTmd = *model;
    PushMatrix();
    if (scaleEnabled != 0) {
        SetSpadStack(0x1F8003FC);
        model_apply_scale();
        ResetSpadStack();
    }
    for (i = 0; i < currentTmd->count; i++) {
        SetSpadStack(0x1F8003FC);
        model_draw_tmd_object(currentTmd->parts, &currentTmd->parts[i]);
        ResetSpadStack();
    }
    PopMatrix();
}
typedef struct ModelPosList {
    s32 unk00;
    SVECTOR v[1];
} ModelPosList;
typedef struct ModelInstance {
    ModelPartList *list;
    ModelPosList *pos;
} ModelInstance;
extern s16 drawnParts;
s32 model_point_in_frustum(SVECTOR *pos);
void camera_frustum_setup(VECTOR *arg0, VECTOR *arg1);

s16 model_draw_visible(ModelInstance *inst, VECTOR *eye, VECTOR *target)
{
    ModelPosList *pos;
    ModelPartList *list;
    s32 i;

    drawnParts = 0;
    SetSpadStack(0x1F8003FC);
    pos = inst->pos;
    list = inst->list;
    if (pos == 0) {
        return 0;
    }
    camera_frustum_setup(eye, target);
    PushMatrix();
    if (scaleEnabled != 0) {
        model_apply_scale();
    }
    for (i = 0; i < list->count; i++) {
        if (model_point_in_frustum(&pos->v[i])) {
            model_draw_tmd_object(list->parts, &list->parts[i]);
        }
    }
    PopMatrix();
    ResetSpadStack();
    return drawnParts;
}
s32 model_point_in_frustum(SVECTOR *pos)
{
    SVECTOR dir;
    SVECTOR n;
    SVECTOR *np;
    s16 *plane;
    s32 dot;

    dir.vx = (viewDirection.vx - pos->vx) / 128;
    dir.vy = (viewDirection.vy - pos->vy) / 128;
    dir.vz = (viewDirection.vz - pos->vz) / 128;
    np = &n;
    VectorNormalSS(&dir, np);
    plane = clipPlanes;
    dot = plane[0] * n.vx;
    dot += plane[1] * np->vy;
    dot += plane[2] * np->vz;
    if (dot < 0) {
        return 0;
    }
    plane += 3;
    dot = plane[0] * n.vx;
    dot += plane[1] * np->vy;
    dot += plane[2] * np->vz;
    if (dot < 0) {
        return 0;
    }
    plane += 3;
    dot = plane[0] * n.vx;
    dot += plane[1] * np->vy;
    dot += plane[2] * np->vz;
    if (dot < 0) {
        return 0;
    }
    plane += 3;
    dot = plane[0] * n.vx;
    dot += plane[1] * np->vy;
    dot += plane[2] * np->vz;
    return dot >= 0;
}
/* Ported from hmbtn-decomp (CC0), model_draw_tmd_object. */
typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char u0, v0;
    u_short clut;
    u_char r1, g1, b1, p1;
    short x1, y1;
    u_char u1, v1;
    u_short tpage;
    u_char r2, g2, b2, p2;
    short x2, y2;
    u_char u2, v2;
    u_short pad2;
    u_char r3, g3, b3, p3;
    short x3, y3;
    u_char u3, v3;
    u_short pad3;
} POLY_GT4;
typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char u0, v0;
    u_short clut;
    u_char r1, g1, b1, p1;
    short x1, y1;
    u_char u1, v1;
    u_short tpage;
    u_char r2, g2, b2, p2;
    short x2, y2;
    u_char u2, v2;
    u_short pad2;
} POLY_GT3;
typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char r1, g1, b1, pad1;
    short x1, y1;
    u_char r2, g2, b2, pad2;
    short x2, y2;
} POLY_G3;
typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char u0, v0;
    u_short clut;
    short x1, y1;
    u_char u1, v1;
    u_short tpage;
    short x2, y2;
    u_char u2, v2;
    u_short pad1;
} POLY_FT3;
typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
    short x2, y2;
} POLY_F3;
extern OtContext *currentFrame;
void debug_hook_run(s32 arg0);
extern s16 drawnParts;
s32 model_clip_test3(s32 a, s32 b, s32 c);
s32 model_clip_test4(s32 a, s32 b, s32 c, s32 d);
void model_draw_tmd_object(ModelPart *first, ModelPart *part);
extern FrameState *modelFrame;

void model_draw_tmd_object(ModelPart *tableArg, ModelPart *objectArg)
{
    TmdObject *table = (TmdObject *)tableArg;
    TmdObject *object = (TmdObject *)objectArg;
    s32 count;
    SVECTOR *verts;
    u8 *packet;
    u8 *prim;
    u_long *ot;
    u32 header;
    s32 depth;

    count = object->nPrimitive;
    if (count <= 0) {
        return;
    }
    debug_hook_run(0x2000);
    verts = (SVECTOR *)((u8 *)table + object->vertTop);
    prim = currentFrameView->primCursor.bytes;
    packet = (u8 *)table + object->primitiveTop;
    ot = currentFrameView->ot;
    modelFrameView = currentFrameView;

loopF3:
    if (count <= 0) {
        goto done;
    }
    header = (*(u32 *)((packet) + (0)));
    if ((header & 0xFDFF0000) != (TMD_PRIM_F3_RAW << 16)) {
        goto loopG3;
    }
    {
        u8 *out = prim;
        u8 *source = packet;
        u_long *slot;
        __asm__ volatile("lwc2	$0, 0( %0 );"
                         "lwc2	$1, 4( %0 );"
                         "lwc2	$2, 0( %1 );"
                         "lwc2	$3, 4( %1 );"
                         "lwc2	$4, 0( %2 );"
                         "lwc2	$5, 4( %2 )"
                         :
                         : "r"(&verts[(*(u16 *)((packet) + (0x8)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0xA)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0xC)))]));
        packet += 0x10;
        count--;
        __asm__ volatile(".word "
                         "0x4A280030");
        (*(u32 *)((out) + (0x4))) = (*(u32 *)((source) + (0x4)));
        __asm__ volatile("cfc2	$12, $31;"
                         "nop;"
                         "sw	$12, 0( %0 )"
                         :
                         : "r"(&depth)
                         : "$12", "memory");
        if (depth < 0) {
            goto loopF3;
        }
        __asm__ volatile(".word "
                         "0x4B400006");
        ((out)[7]) = 0x20 | ((header >> 24) & 2);
        __asm__ volatile("swc2	$24, 0( %0 )" : : "r"(&depth) : "memory");
        if (depth <= 0) {
            goto loopF3;
        }
        __asm__ volatile("swc2	$12, 8( %0 );"
                         "swc2	$13, 12( %0 );"
                         "swc2	$14, 16( %0 )"
                         :
                         : "r"(out)
                         : "memory");
        if (!model_clip_test3((*(u32 *)((out) + (0x8))), (*(u32 *)((out) + (0xC))),
                              (*(u32 *)((out) + (0x10))))) {
            goto loopF3;
        }
        __asm__ volatile(".word "
                         "0x4B58002D");
        prim += 0x14;
        __asm__ volatile("swc2	$7, 0( %0 )" : : "r"(&depth) : "memory");
        if (!(header & 0x8000)) {
            depth += 0x23;
        }
        if (header & 0x4000) {
            depth += 1;
        }
        slot = &ot[depth / 4];
        (*(u32 *)(out) = ((u32)(sizeof(POLY_F3) / 4 - 1) << 24) | (*(slot) & 0xFFFFFF),
         *(slot) = (u32)(out) & 0xFFFFFF);
        drawnPartsView++;
    }
    goto loopF3;

loopG3:
    if (count <= 0) {
        goto done;
    }
    header = (*(u32 *)((packet) + (0)));
    if ((header & 0xFDFF0000) != (TMD_PRIM_G3_RAW << 16)) {
        goto loopFT3;
    }
    {
        u8 *out = prim;
        u8 *source = packet;
        u_long *slot;
        __asm__ volatile("lwc2	$0, 0( %0 );"
                         "lwc2	$1, 4( %0 );"
                         "lwc2	$2, 0( %1 );"
                         "lwc2	$3, 4( %1 );"
                         "lwc2	$4, 0( %2 );"
                         "lwc2	$5, 4( %2 )"
                         :
                         : "r"(&verts[(*(u16 *)((packet) + (0x10)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0x12)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0x14)))]));
        packet += 0x18;
        count--;
        __asm__ volatile(".word "
                         "0x4A280030");
        (*(u32 *)((out) + (0x4))) = (*(u32 *)((source) + (0x4)));
        (*(u32 *)((out) + (0xC))) = (*(u32 *)((source) + (0x8)));
        __asm__ volatile("cfc2	$12, $31;"
                         "nop;"
                         "sw	$12, 0( %0 )"
                         :
                         : "r"(&depth)
                         : "$12", "memory");
        if (depth < 0) {
            goto loopG3;
        }
        __asm__ volatile(".word "
                         "0x4B400006");
        (*(u32 *)((out) + (0x14))) = (*(u32 *)((source) + (0xC)));
        __asm__ volatile("swc2	$24, 0( %0 )" : : "r"(&depth) : "memory");
        if (depth <= 0) {
            goto loopG3;
        }
        __asm__ volatile("swc2	$12, 8( %0 );"
                         "swc2	$13, 16( %0 );"
                         "swc2	$14, 24( %0 )"
                         :
                         : "r"(out)
                         : "memory");
        if (!model_clip_test3((*(u32 *)((out) + (0x8))), (*(u32 *)((out) + (0x10))),
                              (*(u32 *)((out) + (0x18))))) {
            goto loopG3;
        }
        __asm__ volatile(".word "
                         "0x4B58002D");
        ((out)[7]) = 0x30 | ((header >> 24) & 2);
        prim += 0x1C;
        __asm__ volatile("swc2	$7, 0( %0 )" : : "r"(&depth) : "memory");
        if (!(header & 0x8000)) {
            depth += 0x23;
        }
        if (header & 0x4000) {
            depth += 1;
        }
        slot = &ot[depth / 4];
        (*(u32 *)(out) = ((u32)(sizeof(POLY_G3) / 4 - 1) << 24) | (*(slot) & 0xFFFFFF),
         *(slot) = (u32)(out) & 0xFFFFFF);
        drawnPartsView++;
    }
    goto loopG3;

loopFT3:
    if (count <= 0) {
        goto done;
    }
    header = (*(u32 *)((packet) + (0)));
    if ((header & 0xFDFF0000) != (TMD_PRIM_FT3_RAW << 16)) {
        goto loopGT3;
    }
    {
        u8 *out = prim;
        u8 *source = packet;
        u_long *slot;
        __asm__ volatile("lwc2	$0, 0( %0 );"
                         "lwc2	$1, 4( %0 );"
                         "lwc2	$2, 0( %1 );"
                         "lwc2	$3, 4( %1 );"
                         "lwc2	$4, 0( %2 );"
                         "lwc2	$5, 4( %2 )"
                         :
                         : "r"(&verts[(*(u16 *)((packet) + (0x14)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0x16)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0x18)))]));
        packet += 0x1C;
        count--;
        __asm__ volatile(".word "
                         "0x4A280030");
        (*(u32 *)((out) + (0xC))) = (*(u32 *)((source) + (0x4)));
        (*(u32 *)((out) + (0x14))) = (*(u32 *)((source) + (0x8)));
        __asm__ volatile("cfc2	$12, $31;"
                         "nop;"
                         "sw	$12, 0( %0 )"
                         :
                         : "r"(&depth)
                         : "$12", "memory");
        if (depth < 0) {
            goto loopFT3;
        }
        __asm__ volatile(".word "
                         "0x4B400006");
        (*(u32 *)((out) + (0x1C))) = (*(u32 *)((source) + (0xC)));
        __asm__ volatile("swc2	$24, 0( %0 )" : : "r"(&depth) : "memory");
        if (depth <= 0) {
            goto loopFT3;
        }
        __asm__ volatile("swc2	$12, 8( %0 );"
                         "swc2	$13, 16( %0 );"
                         "swc2	$14, 24( %0 )"
                         :
                         : "r"(out)
                         : "memory");
        if (!model_clip_test3((*(u32 *)((out) + (0x8))), (*(u32 *)((out) + (0x10))),
                              (*(u32 *)((out) + (0x18))))) {
            goto loopFT3;
        }
        __asm__ volatile(".word "
                         "0x4B58002D");
        (*(u32 *)((out) + (0x4))) = (*(u32 *)((source) + (0x10)));
        ((out)[7]) = 0x24 | ((header >> 24) & 2);
        __asm__ volatile("swc2	$7, 0( %0 )" : : "r"(&depth) : "memory");
        if (!(header & 0x8000)) {
            depth += 0x23;
        }
        if (header & 0x4000) {
            depth += 1;
        }
        slot = &ot[depth / 4];
        (*(u32 *)(out) = ((u32)(sizeof(POLY_FT3) / 4 - 1) << 24) | (*(slot) & 0xFFFFFF),
         *(slot) = (u32)(out) & 0xFFFFFF);
        drawnPartsView++;
        prim += 0x20;
    }
    goto loopFT3;

loopGT3:
    if (count <= 0) {
        goto done;
    }
    header = (*(u32 *)((packet) + (0)));
    if ((header & 0xFDFF0000) != (TMD_PRIM_GT3_RAW << 16)) {
        goto loopF4;
    }
    {
        u8 *out = prim;
        u8 *source = packet;
        u_long *slot;
        __asm__ volatile("lwc2	$0, 0( %0 );"
                         "lwc2	$1, 4( %0 );"
                         "lwc2	$2, 0( %1 );"
                         "lwc2	$3, 4( %1 );"
                         "lwc2	$4, 0( %2 );"
                         "lwc2	$5, 4( %2 )"
                         :
                         : "r"(&verts[(*(u16 *)((packet) + (0x1C)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0x1E)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0x20)))]));
        packet += 0x24;
        count--;
        __asm__ volatile(".word "
                         "0x4A280030");
        (*(u32 *)((out) + (0x4))) = (*(u32 *)((source) + (0x10)));
        (*(u32 *)((out) + (0x10))) = (*(u32 *)((source) + (0x14)));
        __asm__ volatile("cfc2	$12, $31;"
                         "nop;"
                         "sw	$12, 0( %0 )"
                         :
                         : "r"(&depth)
                         : "$12", "memory");
        if (depth < 0) {
            goto loopGT3;
        }
        __asm__ volatile(".word "
                         "0x4B400006");
        (*(u32 *)((out) + (0x1C))) = (*(u32 *)((source) + (0x18)));
        __asm__ volatile("swc2	$24, 0( %0 )" : : "r"(&depth) : "memory");
        if (depth <= 0) {
            goto loopGT3;
        }
        __asm__ volatile("swc2	$12, 8( %0 );"
                         "swc2	$13, 20( %0 );"
                         "swc2	$14, 32( %0 )"
                         :
                         : "r"(out)
                         : "memory");
        if (!model_clip_test3((*(u32 *)((out) + (0x8))), (*(u32 *)((out) + (0x14))),
                              (*(u32 *)((out) + (0x20))))) {
            goto loopGT3;
        }
        __asm__ volatile(".word "
                         "0x4B58002D");
        (*(u32 *)((out) + (0xC))) = (*(u32 *)((source) + (0x4)));
        (*(u32 *)((out) + (0x18))) = (*(u32 *)((source) + (0x8)));
        __asm__ volatile("swc2	$7, 0( %0 )" : : "r"(&depth) : "memory");
        if (!(header & 0x8000)) {
            depth += 0x23;
        }
        if (header & 0x4000) {
            depth += 1;
        }
        (*(u32 *)((out) + (0x24))) = (*(u32 *)((source) + (0xC)));
        ((out)[7]) = 0x34 | ((header >> 24) & 2);
        slot = &ot[depth / 4];
        (*(u32 *)(out) = ((u32)(sizeof(POLY_GT3) / 4 - 1) << 24) | (*(slot) & 0xFFFFFF),
         *(slot) = (u32)(out) & 0xFFFFFF);
        drawnPartsView++;
        prim += 0x28;
    }
    goto loopGT3;

loopF4:
    if (count <= 0) {
        goto done;
    }
    header = (*(u32 *)((packet) + (0)));
    if ((header & 0xFDFF0000) != (TMD_PRIM_F4_RAW << 16)) {
        goto loopG4;
    }
    {
        u8 *out = prim;
        u8 *source = packet;
        u_long *slot;
        __asm__ volatile("lwc2	$0, 0( %0 );"
                         "lwc2	$1, 4( %0 );"
                         "lwc2	$2, 0( %1 );"
                         "lwc2	$3, 4( %1 );"
                         "lwc2	$4, 0( %2 );"
                         "lwc2	$5, 4( %2 )"
                         :
                         : "r"(&verts[(*(u16 *)((packet) + (0x8)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0xA)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0xC)))]));
        packet += 0x10;
        count--;
        __asm__ volatile(".word "
                         "0x4A280030");
        (*(u32 *)((out) + (0x4))) = (*(u32 *)((source) + (0x4)));
        __asm__ volatile("cfc2	$12, $31;"
                         "nop;"
                         "sw	$12, 0( %0 )"
                         :
                         : "r"(&depth)
                         : "$12", "memory");
        if (depth < 0) {
            goto loopF4;
        }
        __asm__ volatile(".word "
                         "0x4B400006");
        __asm__ volatile("swc2	$24, 0( %0 )" : : "r"(&depth) : "memory");
        if (depth <= 0) {
            goto loopF4;
        }
        __asm__ volatile("swc2	$12, 8( %0 );"
                         "swc2	$13, 12( %0 );"
                         "swc2	$14, 16( %0 )"
                         :
                         : "r"(out)
                         : "memory");
        __asm__ volatile("lwc2	$0, 0( %0 );"
                         "lwc2	$1, 4( %0 )"
                         :
                         : "r"(&verts[(*(u16 *)((source) + (0xE)))]));
        __asm__ volatile("nop;"
                         "nop;"
                         ".word "
                         "0x4A180001");
        ((out)[7]) = 0x28 | ((header >> 24) & 2);
        __asm__ volatile("cfc2	$12, $31;"
                         "nop;"
                         "sw	$12, 0( %0 )"
                         :
                         : "r"(&depth)
                         : "$12", "memory");
        if (depth < 0) {
            goto loopF4;
        }
        __asm__ volatile("swc2	$14, 0( %0 )" : : "r"(&(*(u32 *)((out) + (0x14)))) : "memory");
        if (!model_clip_test4((*(u32 *)((out) + (0x8))), (*(u32 *)((out) + (0xC))),
                              (*(u32 *)((out) + (0x10))), (*(u32 *)((out) + (0x14))))) {
            goto loopF4;
        }
        __asm__ volatile(".word "
                         "0x4B68002E");
        prim += 0x18;
        __asm__ volatile("swc2	$7, 0( %0 )" : : "r"(&depth) : "memory");
        if (!(header & 0x8000)) {
            depth += 0x23;
        }
        if (header & 0x4000) {
            depth += 1;
        }
        slot = &ot[depth / 4];
        (*(u32 *)(out) = ((u32)(sizeof(POLY_F4) / 4 - 1) << 24) | (*(slot) & 0xFFFFFF),
         *(slot) = (u32)(out) & 0xFFFFFF);
        drawnPartsView++;
    }
    goto loopF4;

loopG4:
    if (count <= 0) {
        goto done;
    }
    header = (*(u32 *)((packet) + (0)));
    if ((header & 0xFDFF0000) != (TMD_PRIM_G4_RAW << 16)) {
        goto loopFT4;
    }
    {
        u8 *out = prim;
        u8 *source = packet;
        u_long *slot;
        __asm__ volatile("lwc2	$0, 0( %0 );"
                         "lwc2	$1, 4( %0 );"
                         "lwc2	$2, 0( %1 );"
                         "lwc2	$3, 4( %1 );"
                         "lwc2	$4, 0( %2 );"
                         "lwc2	$5, 4( %2 )"
                         :
                         : "r"(&verts[(*(u16 *)((packet) + (0x14)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0x16)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0x18)))]));
        packet += 0x1C;
        count--;
        __asm__ volatile(".word "
                         "0x4A280030");
        (*(u32 *)((out) + (0x4))) = (*(u32 *)((source) + (0x4)));
        (*(u32 *)((out) + (0xC))) = (*(u32 *)((source) + (0x8)));
        __asm__ volatile("cfc2	$12, $31;"
                         "nop;"
                         "sw	$12, 0( %0 )"
                         :
                         : "r"(&depth)
                         : "$12", "memory");
        if (depth < 0) {
            goto loopG4;
        }
        __asm__ volatile(".word "
                         "0x4B400006");
        (*(u32 *)((out) + (0x14))) = (*(u32 *)((source) + (0xC)));
        __asm__ volatile("swc2	$24, 0( %0 )" : : "r"(&depth) : "memory");
        if (depth <= 0) {
            goto loopG4;
        }
        __asm__ volatile("swc2	$12, 8( %0 );"
                         "swc2	$13, 16( %0 );"
                         "swc2	$14, 24( %0 )"
                         :
                         : "r"(out)
                         : "memory");
        __asm__ volatile("lwc2	$0, 0( %0 );"
                         "lwc2	$1, 4( %0 )"
                         :
                         : "r"(&verts[(*(u16 *)((source) + (0x1A)))]));
        __asm__ volatile("nop;"
                         "nop;"
                         ".word "
                         "0x4A180001");
        (*(u32 *)((out) + (0x1C))) = (*(u32 *)((source) + (0x10)));
        __asm__ volatile("cfc2	$12, $31;"
                         "nop;"
                         "sw	$12, 0( %0 )"
                         :
                         : "r"(&depth)
                         : "$12", "memory");
        if (depth < 0) {
            goto loopG4;
        }
        __asm__ volatile("swc2	$14, 0( %0 )" : : "r"(&(*(u32 *)((out) + (0x20)))) : "memory");
        if (!model_clip_test4((*(u32 *)((out) + (0x8))), (*(u32 *)((out) + (0x10))),
                              (*(u32 *)((out) + (0x18))), (*(u32 *)((out) + (0x20))))) {
            goto loopG4;
        }
        __asm__ volatile(".word "
                         "0x4B68002E");
        ((out)[7]) = 0x38 | ((header >> 24) & 2);
        prim += 0x24;
        __asm__ volatile("swc2	$7, 0( %0 )" : : "r"(&depth) : "memory");
        if (!(header & 0x8000)) {
            depth += 0x23;
        }
        if (header & 0x4000) {
            depth += 1;
        }
        slot = &ot[depth / 4];
        (*(u32 *)(out) = ((u32)(sizeof(POLY_G4) / 4 - 1) << 24) | (*(slot) & 0xFFFFFF),
         *(slot) = (u32)(out) & 0xFFFFFF);
        drawnPartsView++;
    }
    goto loopG4;

loopFT4:
    if (count <= 0) {
        goto done;
    }
    header = (*(u32 *)((packet) + (0)));
    if ((header & 0xFDFF0000) != (TMD_PRIM_FT4_RAW << 16)) {
        goto loopGT4;
    }
    {
        u8 *out = prim;
        u8 *source = packet;
        u_long *slot;
        __asm__ volatile("lwc2	$0, 0( %0 );"
                         "lwc2	$1, 4( %0 );"
                         "lwc2	$2, 0( %1 );"
                         "lwc2	$3, 4( %1 );"
                         "lwc2	$4, 0( %2 );"
                         "lwc2	$5, 4( %2 )"
                         :
                         : "r"(&verts[(*(u16 *)((packet) + (0x18)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0x1A)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0x1C)))]));
        packet += 0x20;
        count--;
        __asm__ volatile(".word "
                         "0x4A280030");
        (*(u32 *)((out) + (0x4))) = (*(u32 *)((source) + (0x14)));
        (*(u32 *)((out) + (0xC))) = (*(u32 *)((source) + (0x4)));
        __asm__ volatile("cfc2	$12, $31;"
                         "nop;"
                         "sw	$12, 0( %0 )"
                         :
                         : "r"(&depth)
                         : "$12", "memory");
        if (depth < 0) {
            goto loopFT4;
        }
        __asm__ volatile(".word "
                         "0x4B400006");
        (*(u32 *)((out) + (0x1C))) = (*(u32 *)((source) + (0xC)));
        __asm__ volatile("swc2	$24, 0( %0 )" : : "r"(&depth) : "memory");
        if (depth <= 0) {
            goto loopFT4;
        }
        __asm__ volatile("swc2	$12, 8( %0 );"
                         "swc2	$13, 16( %0 );"
                         "swc2	$14, 24( %0 )"
                         :
                         : "r"(out)
                         : "memory");
        __asm__ volatile("lwc2	$0, 0( %0 );"
                         "lwc2	$1, 4( %0 )"
                         :
                         : "r"(&verts[(*(u16 *)((source) + (0x1E)))]));
        __asm__ volatile("nop;"
                         "nop;"
                         ".word "
                         "0x4A180001");
        (*(u32 *)((out) + (0x24))) = (*(u32 *)((source) + (0x10)));
        __asm__ volatile("cfc2	$12, $31;"
                         "nop;"
                         "sw	$12, 0( %0 )"
                         :
                         : "r"(&depth)
                         : "$12", "memory");
        if (depth < 0) {
            goto loopFT4;
        }
        __asm__ volatile("swc2	$14, 0( %0 )" : : "r"(&(*(u32 *)((out) + (0x20)))) : "memory");
        if (!model_clip_test4((*(u32 *)((out) + (0x8))), (*(u32 *)((out) + (0x10))),
                              (*(u32 *)((out) + (0x18))), (*(u32 *)((out) + (0x20))))) {
            goto loopFT4;
        }
        __asm__ volatile(".word "
                         "0x4B68002E");
        (*(u32 *)((out) + (0x14))) = (*(u32 *)((source) + (0x8)));
        ((out)[7]) = 0x2C | ((header >> 24) & 2);
        __asm__ volatile("swc2	$7, 0( %0 )" : : "r"(&depth) : "memory");
        if (!(header & 0x8000)) {
            depth += 0x23;
        }
        if (header & 0x4000) {
            depth += 1;
        }
        slot = &ot[depth / 4];
        (*(u32 *)(out) = ((u32)(sizeof(POLY_FT4) / 4 - 1) << 24) | (*(slot) & 0xFFFFFF),
         *(slot) = (u32)(out) & 0xFFFFFF);
        drawnPartsView++;
        prim += 0x28;
    }
    goto loopFT4;

loopGT4:
    if (count == 0) {
        goto done;
    }
    header = (*(u32 *)((packet) + (0)));
    if ((header & 0xFDFF0000) != (TMD_PRIM_GT4_RAW << 16)) {
        goto done;
    }
    {
        u8 *out = prim;
        u8 *source = packet;
        u_long *slot;
        __asm__ volatile("lwc2	$0, 0( %0 );"
                         "lwc2	$1, 4( %0 );"
                         "lwc2	$2, 0( %1 );"
                         "lwc2	$3, 4( %1 );"
                         "lwc2	$4, 0( %2 );"
                         "lwc2	$5, 4( %2 )"
                         :
                         : "r"(&verts[(*(u16 *)((packet) + (0x24)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0x26)))]),
                           "r"(&verts[(*(u16 *)((packet) + (0x28)))]));
        packet += 0x2C;
        count--;
        __asm__ volatile(".word "
                         "0x4A280030");
        (*(u32 *)((out) + (0x4))) = (*(u32 *)((source) + (0x14)));
        (*(u32 *)((out) + (0x10))) = (*(u32 *)((source) + (0x18)));
        __asm__ volatile("cfc2	$12, $31;"
                         "nop;"
                         "sw	$12, 0( %0 )"
                         :
                         : "r"(&depth)
                         : "$12", "memory");
        if (depth < 0) {
            goto loopGT4;
        }
        __asm__ volatile(".word "
                         "0x4B400006");
        (*(u32 *)((out) + (0x1C))) = (*(u32 *)((source) + (0x1C)));
        __asm__ volatile("swc2	$24, 0( %0 )" : : "r"(&depth) : "memory");
        if (depth <= 0) {
            goto loopGT4;
        }
        __asm__ volatile("swc2	$12, 8( %0 );"
                         "swc2	$13, 20( %0 );"
                         "swc2	$14, 32( %0 )"
                         :
                         : "r"(out)
                         : "memory");
        __asm__ volatile("lwc2	$0, 0( %0 );"
                         "lwc2	$1, 4( %0 )"
                         :
                         : "r"(&verts[(*(u16 *)((source) + (0x2A)))]));
        __asm__ volatile("nop;"
                         "nop;"
                         ".word "
                         "0x4A180001");
        (*(u32 *)((out) + (0x28))) = (*(u32 *)((source) + (0x20)));
        __asm__ volatile("cfc2	$12, $31;"
                         "nop;"
                         "sw	$12, 0( %0 )"
                         :
                         : "r"(&depth)
                         : "$12", "memory");
        if (depth < 0) {
            goto loopGT4;
        }
        __asm__ volatile("swc2	$14, 0( %0 )" : : "r"(&(*(u32 *)((out) + (0x2C)))) : "memory");
        if (!model_clip_test4((*(u32 *)((out) + (0x8))), (*(u32 *)((out) + (0x14))),
                              (*(u32 *)((out) + (0x20))), (*(u32 *)((out) + (0x2C))))) {
            goto loopGT4;
        }
        __asm__ volatile("nop;"
                         "nop;"
                         ".word "
                         "0x4B68002E");
        (*(u32 *)((out) + (0xC))) = (*(u32 *)((source) + (0x4)));
        (*(u32 *)((out) + (0x18))) = (*(u32 *)((source) + (0x8)));
        (*(u32 *)((out) + (0x24))) = (*(u32 *)((source) + (0xC)));
        (*(u32 *)((out) + (0x30))) = (*(u32 *)((source) + (0x10)));
        ((out)[7]) = 0x3C | ((header >> 24) & 2);
        __asm__ volatile("swc2	$7, 0( %0 )" : : "r"(&depth) : "memory");
        if (!(header & 0x8000)) {
            depth += 0x23;
        }
        if (header & 0x4000) {
            depth += 1;
        }
        slot = &ot[depth / 4];
        (*(u32 *)(out) = ((u32)(sizeof(POLY_GT4) / 4 - 1) << 24) | (*(slot) & 0xFFFFFF),
         *(slot) = (u32)(out) & 0xFFFFFF);
        drawnPartsView++;
        prim += 0x34;
    }
    goto loopGT4;
done:
    modelFrameView->primCursor.bytes = prim;
}

s32 model_clip_test3(s32 a, s32 b, s32 c)
{
    s16 v;
    s16 s;
    int t;

    t = a + 1;
    v = t;
    if ((u16)(v + 19) < 419)
        goto small;
    t = b + 1;
    v = t;
    if ((u16)(v + 19) < 419)
        goto small;
    t = c + 1;
    v = t;
    if ((u16)(v + 19) >= 419)
        return 0;
small:
    s = a;
    if ((u16)(s + 19) < 339)
        return 1;
    s = b;
    if ((u16)(s + 19) < 339)
        return 1;
    s = c;
    return (u16)(s + 19) < 339;
}
s32 model_clip_test4(s32 a, s32 b, s32 c, s32 d)
{
    s16 v;
    register s32 shifted __asm__("$3") = (u32)a << 16;
    /* Keep the truncated first input in the retail temporary register. */
    __asm__("" : "+r"(shifted));
    v = shifted >> 16;
    if ((u16)(v + 19) < 339)
        return 1;
    v = b;
    if ((u16)(v + 19) < 339)
        return 1;
    v = c;
    if ((u16)(v + 19) < 339)
        return 1;
    v = d;
    return (u16)(v + 19) < 339;
}
void camera_frustum_setup(VECTOR *eye, VECTOR *target)
{
    MATRIX matrix[4];
    VECTOR normal;
    VECTOR delta;
    s16 *output = clipPlanes;
    s32 i;

    delta.vx = target->vx - eye->vx;
    delta.vy = target->vy - eye->vy;
    delta.vz = target->vz - eye->vz;
    VectorNormal(&delta, &normal);
    viewDirection.vx = eye->vx - (normal.vx << 1);
    viewDirection.vy = eye->vy - (normal.vy << 1);
    viewDirection.vz = eye->vz - (normal.vz << 1);
    matrix[0] = GsIDMATRIX;
    matrix[1] = GsIDMATRIX;
    matrix[2] = GsIDMATRIX;
    matrix[3] = GsIDMATRIX;
    RotMatrixY(frustumAngles[0], &matrix[0]);
    RotMatrixY(frustumAngles[1], &matrix[1]);
    RotMatrixX(frustumAngles[2], &matrix[2]);
    RotMatrixX(frustumAngles[3], &matrix[3]);
    PushMatrix();
    for (i = 0; i < 4; i++) {
        s16 *row = &matrix[i].m[i / 2][0];

        gte_SetRotMatrix(&matrix[i]);
        gte_ldclmv(&viewMatrix.m[0][0]);
        gte_mvmva(1, 0, 3, 3, 0);
        gte_stclmv(&matrix[i].m[0][0]);
        gte_ldclmv(&viewMatrix.m[0][1]);
        gte_mvmva(1, 0, 3, 3, 0);
        *output++ = *row++;
        gte_stclmv(&matrix[i].m[0][1]);
        gte_ldclmv(&viewMatrix.m[0][2]);
        gte_mvmva(1, 0, 3, 3, 0);
        *output++ = *row++;
        gte_stclmv(&matrix[i].m[0][2]);
        *output++ = *row++;
    }
    PopMatrix();
}
void model_set_scale_uniform(s32 numerator, s32 denominator)
{
    scalePairs[4] = numerator;
    scalePairs[2] = numerator;
    scalePairs[0] = numerator;
    scalePairs[5] = denominator;
    scalePairs[3] = denominator;
    scalePairs[1] = denominator;
    scaleEnabled = 1;
}
void model_set_scale_axes(s32 num0, s32 den0, s32 num1, s32 den1, s32 num2, s32 den2)
{
    scalePairs[0] = num0;
    scalePairs[1] = den0;
    scalePairs[2] = num1;
    scalePairs[3] = den1;
    scaleEnabled = 1;
    scalePairs[4] = num2;
    scalePairs[5] = den2;
}
void model_clear_scale(void)
{
    scaleEnabled = 0;
}
void model_apply_scale(void)
{
    MATRIX matrix;
    s32 i;

    gte_ReadRotMatrix(&matrix);
    for (i = 0; i < 3; i++) {
        if ((scalePairs[0] & -2) && scalePairs[1] >= 2) {
            matrix.m[i][0] = (matrix.m[i][0] * scalePairs[0]) / scalePairs[1];
        }
        if ((scalePairs[2] & -2) && scalePairs[3] >= 2) {
            matrix.m[i][1] = (matrix.m[i][1] * scalePairs[2]) / scalePairs[3];
        }
        if ((scalePairs[4] & -2) && scalePairs[5] >= 2) {
            matrix.m[i][2] = (matrix.m[i][2] * scalePairs[4]) / scalePairs[5];
        }
    }
    gte_SetRotMatrix(&matrix);
}
s32 model_key_value(s32 key)
{
    s32 i;

    key &= 0xFFFF;
    for (i = 0; i < 20; i++) {
        if (modelKeys[i] == key) {
            return modelValues[i];
        }
    }
    return 0;
}
/* Round 33e bounded: typed 28-byte section records and sparse packet switch reach 191 differences,
 * 205 versus 206 words; section-base recomputation and leaf-register allocation remain different.
 * Candidate round33e-D8B8.txt. */
/* Ported from hmbtn-decomp (CC0), model_relocate_textures. */
#define PRIM_MODE(p)     (*(u16 *)((p) + 2))
#define PRIM_CLUT(p)     (*(u16 *)((p) + 6))
#define PRIM_TPAGE(p)    (*(u16 *)((p) + 0xA))
#define VRAM_PAGE_WIDTH  64
#define VRAM_PAGE_HEIGHT 256
#define CLUT_WIDTH       16
/* Shift a primitive's texture by whole pages and its CLUT by 16-pixel columns and rows. */
#define MOVE_TEXTURE(prim, page_x, page_y, clutX, clutY)                                           \
    do {                                                                                           \
        s16 row;                                                                                   \
        PRIM_TPAGE(prim) = PRIM_TPAGE(prim) + (page_x) + ((page_y) << 4);                          \
        row = (PRIM_CLUT(prim) >> 6) + (clutY);                                                    \
        PRIM_CLUT(prim) = ((PRIM_CLUT(prim) & 0x3F) + (clutX)) | (row << 6);                       \
    } while (0)
void model_relocate_textures(TmdHeader **model, s32 texX, s32 texY, s32 clutX, s32 clutY)
{
    TmdHeader *tmd;
    TmdObject *objects;
    TmdObject *object;
    s32 i;
    s32 j;
    s32 count;
    u8 *prim;

    texX /= VRAM_PAGE_WIDTH;
    texY /= VRAM_PAGE_HEIGHT;
    clutX /= CLUT_WIDTH;
    tmd = *model;
    objects = tmd->objects;
    for (i = 0; i < tmd->nObj; i++) {
        object = &tmd->objects[i];
        prim = (u8 *)objects + object->primitiveTop;
        count = object->nPrimitive;
        for (j = 0; j < count; j++) {
            switch (PRIM_MODE(prim) & 0xFDFF) {
            case TMD_PRIM_FT3:
                MOVE_TEXTURE(prim, texX, texY, clutX, clutY);
                /* fallthrough */
            case TMD_PRIM_F3_FLAG4:
            case TMD_PRIM_G3_RAW:
            case TMD_PRIM_G4:
                prim += 0x18;
                break;
            case TMD_PRIM_F4:
            case TMD_PRIM_G3:
                prim += 0x14;
                break;
            case TMD_PRIM_FT3_RAW:
            case TMD_PRIM_GT3:
                MOVE_TEXTURE(prim, texX, texY, clutX, clutY);
                /* fallthrough */
            case TMD_PRIM_G3_FLAG4:
            case TMD_PRIM_G4_RAW:
                prim += 0x1C;
                break;
            case TMD_PRIM_GT3_RAW:
            case TMD_PRIM_GT4:
                MOVE_TEXTURE(prim, texX, texY, clutX, clutY);
                /* fallthrough */
            case TMD_PRIM_G4_FLAG4:
                prim += 0x24;
                break;
            case TMD_PRIM_FT4:
            case TMD_PRIM_FT4_RAW:
                MOVE_TEXTURE(prim, texX, texY, clutX, clutY);
                /* fallthrough */
            case TMD_PRIM_F4_FLAG4:
                prim += 0x20;
                break;
            case TMD_PRIM_GT4_RAW:
                MOVE_TEXTURE(prim, texX, texY, clutX, clutY);
                prim += 0x2C;
                break;
            case TMD_PRIM_LINE_F2_FLAG1:
                prim += 0xC;
                break;
            case TMD_PRIM_F3:
            case TMD_PRIM_F3_RAW:
            case TMD_PRIM_F4_RAW:
            case TMD_PRIM_LINE_G2_FLAG1:
                prim += 0x10;
                break;
            }
        }
    }
}

typedef struct UvRect {
    u8 u0, v0;
    u16 clut;
    u8 u1, v1;
    u16 page;
    u8 u2, v2;
    u16 pad;
    u8 u3, v3;
    u16 pad2;
} UvRect;
void model_quad_set_texture(UvRect *p, s32 x, s32 y, s32 w, s32 h, s32 cx, s32 cy)
{
    p->u0 = (x % 64) * 4;
    p->v0 = y % 256;
    p->u1 = p->u0 + (w - 1);
    p->v1 = p->v0;
    p->u2 = p->u0;
    p->v2 = p->v0 + (h - 1);
    p->u3 = p->u0 + (w - 1);
    p->v3 = p->v0 + (h - 1);
    p->page = GetTPage(0, 0, x, y);
    p->clut = GetClut(cx, cy);
}
s32 model_quad_clip_check(s32 xy0, s32 xy1, s32 xy2, s32 xy3)
{
    return 0;
}

GfxPacket *model_draw_quad_depth(GfxQuadInput *quad, s32 depthBias)
{
    GfxPacket *pkt;
    s32 flag;
    SVECTOR *p0 = &quad->v0;
    SVECTOR *p1 = &quad->v1;
    SVECTOR *p2 = &quad->v2;

    gte_ldv3(p0, p1, p2);
    pkt = frameRecords.unk118;
    quadFrame = &frameRecords;
    gte_rtpt_b();
    pkt->col1 = quad->col0;
    pkt->col2 = quad->col1;
    gte_stflg(&flag);
    if (flag < 0) {
        return 0;
    }
    gte_nclip_b();
    pkt->col3 = quad->col2;
    gte_stopz(&flag);
    if (flag <= 0) {
        return 0;
    }
    gte_stsxy3_ft4(pkt);
    gte_ldv0(&quad->v3);
    gte_rtps();
    pkt->col4 = quad->col3;
    gte_stflg(&flag);
    if (flag < 0) {
        return 0;
    }
    gte_stsxy(&pkt->xy3);
    gte_avsz4();
    if (model_quad_clip_check(pkt->xy0, pkt->xy1, pkt->xy2, pkt->xy3) == 0) {
        return 0;
    }
    gte_stotz(&flag);
    pkt->col0 = quad->baseColor;
    *((u8 *)&pkt->hdr + 3) = 9;
    *((u8 *)&pkt->col0 + 3) = 0x2C;
    lastQuadDepth = (flag + depthBias) >> (0xE - quadFrame->unk114);
    ot_insert_prim(lastQuadDepth, 0x28);
    return pkt;
}

GfxPacket *model_draw_quad_ot(GfxQuadInput *quad, s32 otIndex)
{
    GfxPacket *pkt;
    s32 flag;
    SVECTOR *p0 = &quad->v0;
    SVECTOR *p1 = &quad->v1;
    SVECTOR *p2 = &quad->v2;

    gte_ldv3(p0, p1, p2);
    pkt = frameRecords.unk118;
    lastQuadFrame = &frameRecords;
    gte_rtpt_b();
    pkt->col1 = quad->col0;
    pkt->col2 = quad->col1;
    gte_stflg(&flag);
    if (flag < 0) {
        return 0;
    }
    gte_nclip_b();
    pkt->col3 = quad->col2;
    gte_stopz(&flag);
    if (flag <= 0) {
        return 0;
    }
    gte_stsxy3_ft4(pkt);
    gte_ldv0(&quad->v3);
    gte_rtps();
    pkt->col4 = quad->col3;
    gte_stflg(&flag);
    if (flag < 0) {
        return 0;
    }
    gte_stsxy(&pkt->xy3);
    gte_avsz4();
    if (model_quad_clip_check(pkt->xy0, pkt->xy1, pkt->xy2, pkt->xy3) == 0) {
        return 0;
    }
    gte_stotz(&flag);
    pkt->col0 = quad->baseColor;
    *((u8 *)&pkt->hdr + 3) = 9;
    *((u8 *)&pkt->col0 + 3) = 0x2C;
    ot_insert_prim(otIndex, 0x28);
    return pkt;
}
s32 model_last_quad_depth(void)
{
    return lastQuadDepth;
}
s32 gte_project_vertex(s32 *screen, SVECTOR *v)
{
    DVECTOR sxy;
    s32 result;

    gte_ldv0(v);
    gte_rtps();
    gte_stflg(&result);
    if (result < 0) {
        return 0;
    }
    gte_stsxy(&sxy);
    if (screen != 0) {
        screen[0] = sxy.vx;
        screen[1] = sxy.vy;
    }
    gte_stszotz(&result);
    return result;
}
s32 gte_project_halfwords(s32 *out, VECTOR *in)
{
    SVECTOR value;
    value.vx = in->vx;
    value.vy = in->vy;
    value.vz = in->vz;
    value.pad = 0;
    return gte_project_vertex(out, &value) & 0xFF;
}
s32 gte_project_vertex_otz(SVECTOR *v)
{
    DVECTOR sxy[2];
    s32 otz;

    gte_ldv0(v);
    gte_rtps();
    gte_stsxy(&sxy);
    gte_stotz(&otz);
    return otz;
}
/* PROBABLE: the shared draw context (BootContext in boot.c); this unit
 * reads only its ordering-table fields. A small extern retail reaches
 * without $gp. */
void debug_hook_run(s32 arg0);
