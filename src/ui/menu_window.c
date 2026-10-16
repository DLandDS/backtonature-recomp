/* Included by src/ui.c; shares its private declarations. */
void window_frame_draw_at(s32 ot, s32 unused, s16 x, s16 y, s16 w, s16 h)
{
    window_frame_draw(ot, x, y, w, h);
}
u32 menu_choice(void)
{
    return menuChosen;
}
/* D_80010318 is referenced only from the .data segment, never from code,
 * so splat cannot migrate it into a function. It sits at 0x80010318,
 * between this unit's jtbl_80010224 and menuHandlers, and a unit gets only
 * one contiguous .rodata run -- so it is pulled in here explicitly to keep
 * that run unbroken. Its source position sets its position in .rodata. */
INCLUDE_ASM("asm/nonmatchings/ui", D_80010318);
/* The named table and rodata order marker preserve retail dispatch and data layout. */
s32 menu_begin_load(void);
s32 menu_load_result(void);
s32 menu_open_step(void);
s32 menu_run_frame(void);
s32 menu_close_step(void);
s32 menu_finish(void);

/* rodata-after: D_80010318 menuHandlers */
s32 (*const menuHandlers[])(u8 *) = {
    0,
    (s32 (*)(u8 *))menu_begin_load,
    (s32 (*)(u8 *))menu_load_result,
    (s32 (*)(u8 *))menu_open_step,
    (s32 (*)(u8 *))menu_run_frame,
    (s32 (*)(u8 *))menu_close_step,
    (s32 (*)(u8 *))menu_finish,
    0,
};

s32 menu_run_state(void)
{
    u8 state = gameState[0xCC];

    if (state == 0) {
        return 1;
    }
    return menuHandlers[state](gameState);
}
s32 menu_request_open(u32 id, u32 dataIndex)
{
    s32 result = 1;

    if (gameState[0xCC] == 0) {
        gameState[0xCC] = 1;
        result = 0;
        menuId = id;
        menuDataIndex = dataIndex;
        menuPhase = 0;
    }
    if (((u32)(gameState[0xCC] - 5) < 2) && ((menuId != id) || (menuDataIndex != dataIndex)) &&
        (menuQueuedId == (u32)-1)) {
        result = 0;
        menuQueuedId = id;
        menuQueuedArg = dataIndex;
    }
    return result;
}
u32 menu_get_phase(void)
{
    return menuPhase;
}
s32 menu_phase_is_result(void)
{
    s32 result = 0;
    if ((menuPhase == 2) || (menuPhase == 5)) {
        result = 1;
    }
    return result;
}
void menu_set_sound_enabled(u32 value)
{
    menuSoundEnabled = value;
}
u32 menu_get_sound_enabled(void)
{
    return menuSoundEnabled;
}
s32 menu_finish(void);

void menu_reset(void)
{
    menuSavedFlags = 0;
    menu_finish();
    storage8006C904[0] = 0;
    text_set_input(1);
    menu_set_sound_enabled(1);
    menuPhase = 2;
    menuQueuedId = -1;
}
void menu_begin_close(void)
{
    text_set_input(1);
    menu_set_sound_enabled(1);
    if (gameState[0xCC] == 4) {
        gameState[0xCC] = 5;
        menuBoxFrame = 0;
        menuPhase = 0xF;
    }
}
void menu_set_saved_flags(u32 value)
{
    menuSavedFlags = value;
}
/* Forward-declared: defined later in this unit, but called from
 * menu_open_step below that point. CONFIRMED against menu_open_step's own
 * outgoing stack stores: the caller stores 0x10B and 0x3E as words at
 * +0x10/+0x14 (read back with lh at the callee's SP0+0x10/+0x14, so args 5
 * and 6 are s16), &menuBoxFrame at +0x18 (read back with lw at SP0+0x18 and
 * then dereferenced/stored through, so arg 7 is s32 *), and 0xC and 0x1
 * (=menuWindowValues index? no -- s0) as words at +0x1c/+0x20 (read back with lw
 * at SP0+0x1c/+0x20 and used in `div`/`bne` arithmetic, so args 8 and 9 are
 * plain s32). Inside menu_box_animate's own body, $a2/$a3 (args 3 and 4) are
 * each `sll 16`/`sra 16`d before use, confirming s16; $a0 (arg 1) and $a1
 * (arg 2, dead -- never read before being overwritten as scratch) are used
 * unmasked, so plain s32. menu_box_animate sets $v0 on every return path, so
 * it is not void. */
s32 menu_box_animate(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5, s32 *arg6,
                     s32 arg7, s32 arg8);
/* Ported from hmbtn-decomp (CC0), menu_box_animate. */
s32 menu_box_animate(s32 ot, s32 unused, s16 x, s16 y, s16 w, s16 h, s32 *frame, s32 duration,
                     s32 mode)
{
    s32 heightStep;
    s32 widthStep;
    s32 t;
    s32 elapsed;
    s32 end;
    s32 scaled;
    s32 boxX;
    s32 boxY;
    s32 boxW;
    s32 boxH;

loop:
    boxY = y + h / 2;
    boxX = x + w / 2;
    heightStep = (h << 11) / duration;
    widthStep = (w << 11) / 6;
    if (mode == 1) {
        t = *frame;
    } else {
        elapsed = *frame - 6;
        t = duration - elapsed;
    }
    if (t < 6) {
        scaled = widthStep * t;
        t = scaled / 4096;
        if (widthStep)
            scaled = 0;
        boxX -= t;
        boxH = 4;
        boxW = t * 2;
    } else {
        scaled = heightStep * (t - 6);
        boxX = x;
        boxY -= scaled / 4096;
        boxH = scaled / 4096 * 2;
        boxW = w;
        if (boxH < 3) {
            (*frame)++;
            goto loop;
        }
        if (boxH < 4) {
            boxH = 4;
        }
    }
    end = duration + 6;
    if (*frame >= end) {
        if (mode == 1) {
            window_frame_draw_at(ot, 0, x, y, w, h);
        }
        *frame = end;
        return 1;
    }
    window_frame_draw_at(ot, 0, boxX, boxY, boxW, boxH);
    (*frame)++;
    return 0;
}

s16 menu_result_file(void);
void *LoadAFileEntryRange(s32 entry, s32 offset, s32 size, void *destination, s32 arg4, s32 arg5);
int event_flag_get(u16 index);
void event_flag_set(u16 index, u16 value);

s32 menu_begin_load(void)
{
    u8 *work;
    u32 flags;

    menuEventBits = event_flag_get(3);
    event_flag_set(3, 1);
    work = gameState;
    flags = *(u32 *)(work + 0xC);
    menuPhase = 14;
    menuSavedFlags = flags;
    *(u32 *)(work + 0xC) = flags | 0x8000000;
    LoadAFileEntryRange(menu_result_file(), menuDataIndex << 13, 0x2000, work + 0xD8, 0, 1);
    menuBoxFrame = 0;
    work[0xCC] = 2;
    return 0;
}
extern u8 storage8006C910[];
extern u8 *menuText;
void DecodeMessageSlotContainer(void *slot);
void text_reset(void);

s32 menu_load_result(void)
{
    void *slot;
    s32 mode;
    s32 result;

    slot =
        LoadAFileEntryRange(menu_result_file(), menuDataIndex << 13, 0x2000, storage8006C910, 0, 0);
    mode = 1;
    result = menu_box_animate(*(s32 *)((u8 *)currentFrame + 0xD8) + 8, 0, 0x1A, 0xA4, 0x10B, 0x3E,
                              &menuBoxFrame, 0xC, mode);
    if (slot != 0) {
        DecodeMessageSlotContainer(slot);
        menuText = storage8006C910 + 0x1800;
        text_reset();
        storage8006C910[-0xC] = (result == mode) ? 4 : 3;
        anim_bind_file(menuAnim, storage800977F8);
        farm_work_sector_release();
    }
    return 0;
}
s16 menu_result_file(void)
{
    u32 index = menuId;
    if (index >= 5) {
        index = 0;
    }
    return menuWindowValues[index];
}
s32 menu_open_step(void)
{
    s32 mode = 1;

    if (menu_box_animate(*(s32 *)((u8 *)currentFrame + 0xD8) + 8, 0, 0x1A, 0xA4, 0x10B, 0x3E,
                         &menuBoxFrame, 0xC, mode) == mode) {
        storage8006C904[0] = 4;
    }
    return 0;
}
extern s32 text_get_palette(void);
extern void text_set_palette(u32 arg0);
extern u32 text_window_draw(void *ot, u8 *stream, s16 x, s16 y, s16 unused, s16 bottom);
void DrawDialogUiLabelFromArg(void);
/* Message-box result handler. The switch includes an explicit empty case 0
 * so cc1 builds retail jtbl_80010378 from index 0, and cases 10..13 are
 * written out in full; cc1 cross-jumps their common tail itself. */
s32 menu_run_frame(void)
{
    s32 saved;
    s32 label;
    u32 sel;

    text_set_spacing(0, 2);
    label = 0;
    saved = text_get_palette();
    text_set_palette(3);
    menuPhase = text_window_draw((void *)(*(s32 *)((u8 *)currentFrame + 0xD8) + 8), menuText, 0x1E,
                                 0xA8, 0x107, 0x3A);
    text_set_palette(saved);
    text_set_spacing(0, 0);
    menuChosen = 0;
    switch (menuPhase) {
    case 0:
        break;
    case 2:
        gameState[0xCC] = 5;
        menuPhase = 0xF;
        break;
    case 1:
    case 3:
    case 4:
    case 5:
        label = 1;
        break;
    case 10:
        gameState[0xCC] = 5;
        menuChosen = 1;
        menuPhase = 0xF;
        break;
    case 11:
        gameState[0xCC] = 5;
        menuChosen = 2;
        menuPhase = 0xF;
        break;
    case 12:
        gameState[0xCC] = 5;
        menuChosen = 3;
        menuPhase = 0xF;
        break;
    case 13:
        gameState[0xCC] = 5;
        menuChosen = 4;
        menuPhase = 0xF;
        break;
    }
    if (gameState[0xCC] != 6) {
        window_frame_draw_at(*(s32 *)((u8 *)currentFrame + 0xD8) + 0xC, 0, 0x1A, 0xA4, 0x10B, 0x3E);
        if (label != 0 && menuSoundEnabled == 1) {
            DrawDialogUiLabelFromArg();
        }
    }
    if (gameState[0xCC] == 5) {
        menuBoxFrame = 0;
    }
    return 0;
}
s32 menu_close_step(void)
{
    s32 mode = 6;

    if (menu_box_animate(*(s32 *)((u8 *)currentFrame + 0xD8) + 8, 0, 0x1A, 0xA4, 0x10B, 0x3E,
                         &menuBoxFrame, mode, 0) == 1) {
        storage8006C904[0] = mode;
        if (menuQueuedId == -1) {
            menuPhase = 2;
        }
    }
    return 0;
}
/* A word-offset local keeps the full work-area base and retail branch addresses. */
s32 menu_finish(void)
{
    u32 *flags;
    s32 wordOffset = 0xC;

    event_flag_set(3, menuEventBits);
    if (menuSavedFlags & 0x8000000) {
        flags = (u32 *)(gameState + wordOffset);
        *flags |= 0x8000000;
    } else {
        flags = (u32 *)(gameState + wordOffset);
        *flags &= ~0x8000000;
    }
    gameState[0xCC] = 0;
    if (menuQueuedId != (u32)-1) {
        menu_request_open(menuQueuedId, menuQueuedArg);
        menuQueuedId = -1;
    } else {
        menuPhase = 2;
    }
    return 1;
}
s16 GetDialogUiLabelValue(void);
s32 CalculateUiStatLabelTier(s16 value);
