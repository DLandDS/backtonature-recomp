/* Included by src/ui.c; shares its private declarations. */
void bg_tile_mode_sprite(s32 arg)
{
    bg_tile_screen(arg, (UiCallback)bg_tile_draw_sprite);
}
void bg_tile_mode_cursor(s32 arg)
{
    bg_tile_screen(arg, (UiCallback)bg_tile_draw_cursor);
}
extern u32 bgScrollFrames;
extern s16 bgTileFrameOrder[];
void bg_tile_screen(s32 ot, UiCallback cb)
{
    u32 n, phase, y;
    s32 x, k = 0;
    UiCallback fn;
    anim_bind_file(uiIconAnim, storage800977F8);
    n = ++bgScrollFrames;
    fn = cb;
    phase = (n >> 2) - n / 96 * 24;
    for (y = 0; y < 0x108; y += 24) {
        for (x = -12; x < 0x158; x += 24) {
            k++;
            k &= 3;
            fn(ot, (s32)uiIconAnim, bgTileFrameOrder[k], x - phase, y - phase);
        }
    }
}
void bg_tile_draw_sprite(s32 ot, s32 sprite, s32 frame, s32 x, s32 y)
{
    anim_draw_sprt_cursor(ot, sprite, 0x1D, (u8)frame, (s16)x, (s16)y);
}
void bg_tile_draw_cursor(s32 ot, s32 sprite, s32 frame, s32 x, s32 y)
{
    s32 *ctx = (s32 *)((u8 *)currentFrame + 0x118);

    anim_draw_sprt(ot, ctx, ctx, sprite, 0x1D, (u8)frame, (s16)x, (s16)y);
}
