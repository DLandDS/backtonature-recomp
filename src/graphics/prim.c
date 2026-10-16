/* Included by src/archive.c; shares its private declarations. */
void prim_add_pixel(u32 *ot, s16 x0, s16 y0, s32 color, s16 abe)
{
    TILE_1 *p;

    p = gs_work_alloc(0xC);
    prim_make_pixel(p, ot, x0, y0, color, abe);
}
void prim_add_line(u32 *ot, s16 x0, s16 y0, s16 x1, s16 y1, s32 color, s16 abe)
{
    LINE_F2 *p;

    p = gs_work_alloc(0x10);
    prim_make_line(p, ot, x0, y0, x1, y1, color, abe);
}
void prim_add_gouraud_rect(u32 *ot, s16 x0, s16 y0, s16 w, s16 h, s32 color0, s32 color1,
                           s32 color2, s32 color3, s16 abe)
{
    POLY_G4 *p;

    p = gs_work_alloc(0x24);
    prim_make_gouraud_rect(p, ot, x0, y0, w, h, color0, color1, color2, color3, abe);
}
void prim_add_rect(u32 *ot, s16 x0, s16 y0, s16 w, s16 h, s32 color, s16 abe)
{
    POLY_F4 *p;

    p = gs_work_alloc(0x18);
    prim_make_rect(p, ot, x0, y0, w, h, color, abe);
}
void prim_add_tile(u32 *ot, s16 x, s16 y, s16 w, s16 h, s32 color, s16 abe)
{
    TILE *p;

    p = gs_work_alloc(0x10);
    prim_make_tile(p, ot, x, y, w, h, color, abe);
}
void *prim_make_rect_mode(void *p, u32 *ot, s16 x, s16 y, s16 w, s16 h, s32 color, s16 arg7,
                          u8 arg8);

void prim_add_rect_mode(u32 *ot, s16 x, s16 y, s16 w, s16 h, s32 color, s16 semiTrans, u8 flags)
{
    gs_work_release(prim_make_rect_mode(gs_work_alloc(0), ot, x, y, w, h, color, semiTrans, flags));
}
TILE_1 *prim_make_pixel(TILE_1 *p, u32 *ot, s16 x0, s16 y0, s32 color, s16 abe)
{
    setTile1(p);
    setSemiTrans(p, abe);
    setXY0(p, x0, y0);
    setRGB0(p, color >> 16, color >> 8, color);
    addPrim(ot, p);
    return p + 1;
}
/* Seeded Track-A candidate was bounded after two forms. Retail has 44
 * instructions; best candidate had 43 and reported 44 aligned differences
 * from +0x00 because one missing word shifted the body. The prior 42-word
 * form reported 30 differences. Both failed in register allocation and
 * scheduling around initial packet stores, conditional 0x40/0x42 code, and
 * packed union/byte-field addressing. Preserve ASM. */
LINE_F2 *prim_make_line(LINE_F2 *p, u32 *ot, s16 x0, s16 y0, s16 x1, s16 y1, s32 color, s16 abe)
{
    setLineF2(p);
    setSemiTrans(p, abe);
    setXY2(p, x0, y0, x1, y1);
    setRGB0(p, color >> 16, color >> 8, color);
    addPrim(ot, p);
    return p + 1;
}
POLY_G4 *prim_make_gouraud_rect(POLY_G4 *p, u32 *ot, s16 x0, s16 y0, s16 w, s16 h, s32 color0,
                                s32 color1, s32 color2, s32 color3, s16 abe)
{
    setPolyG4(p);
    setSemiTrans(p, abe);
    setXY4(p, x0, y0, x0 + w, y0, x0, y0 + h, x0 + w, y0 + h);
    setRGB0(p, color0 >> 16, color0 >> 8, color0);
    setRGB1(p, color1 >> 16, color1 >> 8, color1);
    setRGB2(p, color2 >> 16, color2 >> 8, color2);
    setRGB3(p, color3 >> 16, color3 >> 8, color3);
    addPrim(ot, p);
    return p + 1;
}
/* Seeded Track-A candidate was bounded after two forms. Retail has 50
 * instructions; strongest candidate had 49. An interrupted final form had 48
 * and reported 46 aligned differences from +0x08. cc1 scheduled the packet
 * pointer/initial 0x28 store differently and selected logical rather than
 * retail arithmetic shifts for color-byte extraction. Signed field/value
 * typing and a separate code local were both tried. Preserve ASM. */
POLY_F4 *prim_make_rect(POLY_F4 *p, u32 *ot, s16 x0, s16 y0, s16 w, s16 h, s32 color, s16 abe)
{
    setPolyF4(p);
    setRGB0(p, color >> 16, color >> 8, color);
    setSemiTrans(p, abe);
    setXY4(p, x0, y0, x0 + w, y0, x0, y0 + h, x0 + w, y0 + h);
    addPrim(ot, p);
    return p + 1;
}
/* Seeded Track-A candidate was bounded after two forms. Retail has 44
 * instructions. Direct 0x62/0x60 byte-7 assignment generated 43 instructions
 * and 16 differences beginning at +0x08, keeping initial 0x60 in $t2 rather
 * than retail $v0. An s32 code local also generated 43 but reported 44 aligned
 * differences from +0x00 by moving 0x60 before packet setup. Preserve ASM. */
TILE *prim_make_tile(TILE *p, u32 *ot, s16 x, s16 y, s16 w, s16 h, s32 color, s16 abe)
{
    setTile(p);
    setRGB0(p, color >> 16, color >> 8, color);
    setSemiTrans(p, abe);
    setXY0(p, x, y);
    setWH(p, w, h);
    addPrim(ot, p);
    return p + 1;
}
void *prim_make_rect_mode(void *p, u32 *ot, s16 x, s16 y, s16 w, s16 h, s32 color, s16 abe, u8 abr)
{
    DR_TPAGE *mode;
    DR_TPAGE *next;

    next = (DR_TPAGE *)prim_make_rect(p, ot, x, y, w, h, color, abe);
    mode = next;
    next = mode + 1;
    setlen(mode, 1);
    mode->code[0] = 0xE1000200 | ((abr & 3) << 5);
    addPrim(ot, mode);
    return next;
}
