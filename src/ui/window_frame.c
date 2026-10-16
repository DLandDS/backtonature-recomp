/* Included by src/system.c; shares its private declarations. */
/* Bevelled window frame: corner pixels, edge lines, then a gradient fill. */
void window_frame_draw(u32 *ot, s16 x, s16 y, s16 w, s16 h)
{
    s16 right;
    s32 xm1;
    s32 ym1;
    s32 xm2;
    s16 innerRight;
    s16 farRight;
    s16 top;
    s16 left;
    s16 bottom;
    s16 innerBottom;
    s16 farBottom;

    w--;
    h--;
    xm1 = x - 1;
    right = xm1 + w;
    prim_add_pixel(ot, right, y, 0xF8DBAF, 0);
    top = y + 1;
    prim_add_pixel(ot, right, top, 0xDEBB84, 0);
    farRight = x + w;
    prim_add_pixel(ot, farRight, top, 0xC89F60, 0);
    left = x + 1;
    ym1 = y - 1;
    bottom = ym1 + h;
    prim_add_pixel(ot, left, bottom, 0xE2C08B, 0);
    farBottom = y + h;
    prim_add_pixel(ot, left, farBottom, 0xC89F60, 0);
    prim_add_pixel(ot, x, bottom, 0xF8DBAF, 0);
    xm2 = x - 2;
    innerRight = xm2 + w;
    prim_add_line(ot, left, top, innerRight, top, 0xFDECD2, 0);
    prim_add_line(ot, left, top, left, bottom, 0xFDECD2, 0);
    prim_add_line(ot, right, top, right, bottom, 0xBA8B47, 0);
    prim_add_line(ot, left, bottom, innerRight, bottom, 0xBA8B47, 0);
    prim_add_line(ot, left, y, right, y, 0xFDECD2, 0);
    prim_add_line(ot, x, top, x, bottom, 0xFDECD2, 0);
    prim_add_line(ot, farRight, top, farRight, bottom, 0xBA8B47, 0);
    prim_add_line(ot, left, farBottom, right, farBottom, 0xBA8B47, 0);
    prim_add_gouraud_rect(ot, x + 2, y + 2, w - 3, h - 3, 0xF4DFBD, 0xF4DFBD, 0xF9CF8F, 0xF9CF8F,
                          0);
}
