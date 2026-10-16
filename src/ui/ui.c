/* Included by src/ui.c; shares its private declarations. */
void DrawDialogUiLabelFromArg(void)
{
    s16 value = GetDialogUiLabelValue();
    s32 label = 7;

    if (value != 0) {
        label = CalculateUiStatLabelTier(value);
    }
    if (menuAnim[4] != label) {
        anim_select_range(menuAnim, label);
    }
    anim_draw_sprt_advance(*(u8 **)((u8 *)currentFrame + 0xD8) + 4, menuAnim, 0x11D, 0xD9);
}
s32 CalculateUiStatLabelTier(s16 which)
{
    u8 *base = gameState;
    s32 i = heartIndexTable[which];
    u8 *work = base + i * 468;
    u32 value = *(u16 *)(work + 0x99EC);
    if (value < 5000)
        return 0;
    if (value < 20000)
        return 1;
    if (value < 30000)
        return 2;
    if (value <= 39999)
        return 3;
    if (value <= 49999)
        return 4;
    if (value <= 59999)
        return 5;
    return 6;
}
/* CONFIRMED from archive.c and its direct call below: the decoder begins at
 * the container payload, four bytes after the signed slot/page field. */
void DecodeContainer0DFromStart(void *container);
void text_set_bank(s32 page);
void DecodeMessageSlotContainer(void *slot)
{
    s32 page = *(s32 *)slot;

    if (page != -1) {
        DecodeContainer0DFromStart((u8 *)slot + 4);
        text_set_bank(page);
    }
}
