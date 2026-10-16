/* Included by src/ui.c; shares its private declarations. */
void ui_draw_icon_2a(s32 ot, s16 x, s16 y)
{
    ui_draw_icon(ot, 0x2A, 0, x, y);
}
void ui_draw_icon_27_0(s32 ot, s16 x, s16 y)
{
    ui_draw_icon(ot, 0x27, 0, x, y);
}

void ui_draw_icon_27_2(s32 ot, s16 x, s16 y)
{
    ui_draw_icon(ot, 0x27, 2, x, y);
}

void ui_draw_icon_27_1(s32 ot, s16 x, s16 y)
{
    ui_draw_icon(ot, 0x27, 1, x, y);
}

void ui_draw_icon_27_3(s32 ot, s16 x, s16 y)
{
    ui_draw_icon(ot, 0x27, 3, x, y);
}
void ui_blink_tick(int advance)
{
    u8 q;
    s16 s;
    int t;

    s = t = (s16)advance;
    if (s != -1) {
        t = uiIconCycle + 1;
        advance = (u8)t;
        t = (u32)advance / 120;
        s = t;
        q = s;
        advance -= q * 120;
        uiIconCycle = advance;
    } else {
        uiIconCycle = 0;
    }
}
void ui_draw_icon_29_phase(s32 ot, s16 x, s16 y)
{
    ui_draw_icon(ot, 0x29, (u8)(uiIconCycle / 30), x, y);
}
void ui_draw_icon_28_phase(s32 ot, s16 x, s16 y)
{
    ui_draw_icon(ot, 0x28, (u8)(uiIconCycle / 30), x, y);
}

void ui_draw_icon_0d_phase(s32 ot, s16 x, s16 y)
{
    ui_draw_icon(ot, 0xD, (u8)(uiIconCycle / 30), x, y);
}

void ui_draw_icon_0c_phase(s32 ot, s16 x, s16 y)
{
    ui_draw_icon(ot, 0xC, (u8)(uiIconCycle / 30), x, y);
}
void ui_draw_icon_0b_1(s32 ot, s16 x, s16 y)
{
    ui_draw_icon(ot, 0xB, 1, x, y);
}
void ui_draw_icon_18_sub(s32 ot, s16 x, s16 y, s16 sub)
{
    ui_draw_icon(ot, 0x18, sub, x, y);
}
void ui_draw_icon_1e(s32 ot, s16 x, s16 y)
{
    ui_draw_icon(ot, 0x1E, 0, x, y);
}
void ui_blink_tick_60(int advance)
{
    u8 q;
    s16 s;
    int t;

    s = t = (s16)advance;
    if (s != -1) {
        t = uiIconTick + 1;
        advance = (u8)t;
        t = (u32)advance / 60;
        s = t;
        q = s;
        advance -= q * 60;
        uiIconTick = advance;
    } else {
        uiIconTick = 0;
    }
}
void ui_draw_icon_07_phase(s32 ot, s16 x, s16 y)
{
    ui_draw_icon(ot, 7, (u8)(uiIconTick / 30), x, y);
}
void ui_draw_icon_2b_sub(s32 ot, s16 x, s16 y, s16 sub)
{
    ui_draw_icon(ot, 0x2B, sub, x, y);
}
void ui_draw_icon(s32 ot, s32 icon, s32 frame, s16 x, s16 y)
{
    if (uiIconAnim[5] != (u32)storage800977F8) {
        anim_bind_file(uiIconAnim, storage800977F8);
    }
    anim_draw_sprt_cursor(ot, (s32)uiIconAnim, (u8)icon, (u8)frame, x, y);
}
