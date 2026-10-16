/* Included by src/ui.c; shares its private declarations. */
void name_sound_reset(void)
{
    nameSoundState = 0;
    nameSoundValue = 0;
    namePromptRow = 0;
}
void name_sound_begin(u32 sequence)
{
    name_sound_reset();
    nameSoundSeq = (u32 *)sequence;
    nameSoundState = 1;
}
s32 name_sound_step(void)
{
    if (nameSoundState != 0) {
        prim_add_tile((u32 *)(*(u8 **)((u8 *)currentFrame + 0xD8) + 0x6C), 0, 0, HWD0, VWD0, 0, 1);
        name_sound_draw();
        switch (nameSoundState) {
        case 4:
            nameSoundState = 0;
            return -1;
        case 5:
            nameSoundState = 0;
            return 1;
        }
        return 2;
    }
    return 0;
}
void name_entry_reset(void)
{
    namePhase = 0;
    nameCancelAllowed = 1;
    namePromptToggle = 0;
    nameCursorCell = 0;
    nameShift = 0;
    nameEntryUnused8005E34C = 0;
    nameLength = 0;
    nameTitleMode = 0;
    nameDirty = 0;
}
void name_entry_start(s16 *name)
{
    name_entry_reset();
    nameBuffer = name;
    name_buffer_clear();
    nameCancelAllowed = 1;
    nameTitleMode = 1;
    namePhase = 1;
}
void name_entry_start_no_cancel(s16 *name)
{
    name_entry_reset();
    nameBuffer = name;
    name_buffer_clear();
    nameCancelAllowed = 0;
    namePhase = 1;
}
void name_entry_start_cancellable(s16 *name)
{
    name_entry_reset();
    nameBuffer = name;
    name_buffer_clear();
    nameCancelAllowed = 1;
    namePhase = 1;
}
s32 name_entry_step(void)
{
    if (namePhase != 0) {
        prim_add_tile((u32 *)(*(u8 **)((u8 *)currentFrame + 0xD8) + 0x6C), 0, 0, HWD0, VWD0, 0, 1);
        name_grid_step();
        switch (namePhase) {
        case 3:
            namePhase = 0;
            return -1;
        case 4:
            namePhase = 0;
            return 1;
        }
        return 2;
    }
    return 0;
}
void name_sound_draw_frame(void);
void name_title_build_display(void);
void name_sound_update(void);
void name_sound_handle_input(void);
void ui_draw_icon_0b_1(s32 arg0, s16 arg1, s16 arg2);

void name_sound_draw(void)
{
    s32 display = *(s32 *)((u8 *)currentFrame + 0xD8) + 0x60;
    s16 day;
    s16 week;

    name_sound_draw_frame();
    if (nameSoundState == 3) {
        name_sound_handle_input();
        return;
    }
    menu_set_sound_enabled(0);
    text_set_input(0);
    menu_request_open(3, 0x3A9);
    if (nameSoundState == 1) {
        ui_draw_icon_0b_1(display, (s16)nameSoundValue * 64 + 0x24, 0x42);
    } else if (nameSoundState == 2) {
        day = nameSoundValue;
        week = day / 7;
        ui_draw_icon_0b_1(display, (day - week * 7) * 39 + 0x24, week * 14 + 0x5E);
    }
    name_title_build_display();
    name_sound_update();
}
extern u16 storage80053946[];
void ui_draw_icon_2b_sub(s32 arg0, s16 arg1, s16 arg2, s16 arg3);
void text_draw_number(s16 value, s16 digits, void *display, s16 x, s16 y);
void DrawTextWrapper(void *display, const u16 *stream, s16 x, s16 y);

void name_sound_draw_frame(void)
{
    u8 *base = *(u8 **)((u8 *)currentFrame + 0xD8);
    u8 *display = base + 0x64;

    window_frame_draw_at((s32)(base + 0x68), 1, 0xAB, 0x20, 0x85, 0x12);
    DrawTextWrapper(display, (const u16 *)(mainUiTextTableRuntimeOffset[0] + (s32)storage80053946),
                    0xAD, 0x23);
    if (nameSoundState >= 2) {
        ui_draw_icon_2b_sub((s32)display, 0xFD, 0x23, (s16)nameSoundSeq[1]);
    }
    if (nameSoundState >= 3) {
        text_draw_number(nameSoundSeq[2] + 1, 2, display, 0x115, 0x23);
    }
}
void ui_draw_icon_2b_sub(s32 arg0, s16 arg1, s16 arg2, s16 arg3);
void text_draw_number(s16 value, s16 digits, void *display, s16 x, s16 y);
u16 *text_write_season(s32 index, u16 *dst);
extern u16 nameDisplayText[];
void DrawTextWrapper(void *display, const u16 *stream, s16 x, s16 y);

void name_title_build_display(void)
{
    s32 base = *(s32 *)((u8 *)currentFrame + 0xD8);
    s32 display = base + 0x64;
    s32 i;

    window_frame_draw_at(base + 0x68, 1, 0x14, 0x38, 0x117, 0x67);
    for (i = 0; i < 4; i++) {
        ui_draw_icon_2b_sub(display, i * 0x40 + 0x1E, 0x3C, i);
        *(s16 *)text_write_season(i, nameDisplayText) = -1;
        DrawTextWrapper((void *)display, nameDisplayText, i * 0x40 + 0x2A, 0x3C);
    }
    for (i = 0; i < 30; i++) {
        text_draw_number(i + 1, 2, (void *)display, (i % 7) * 39 + 0x1E, (i / 7) * 14 + 0x58);
    }
}
void name_sound_commit_value(void);
void name_sound_commit_next(void);
void name_sound_set_done(void);
void name_sound_load_value(void);
s16 name_sound_move_position(s16 arg0);
s16 name_sound_move_index(s16 arg0);

void name_sound_update(void)
{
    void (*confirm)(void);
    void (*cancel)(void);
    s16 (*move)(s16);
    s16 prev;

    if (menu_phase_is_result() != 0) {
        if (nameSoundState == 1) {
            confirm = name_sound_commit_value;
            cancel = name_sound_set_done;
            move = name_sound_move_position;
        } else if (nameSoundState == 2) {
            confirm = name_sound_commit_next;
            cancel = name_sound_load_value;
            move = name_sound_move_index;
        } else {
            return;
        }
        if (inputPad1[2] == 0x40) {
            confirm();
            sound_play(0);
        } else if (inputPad1[2] == 0x20) {
            cancel();
            sound_play(1);
        } else {
            prev = nameSoundValue;
            nameSoundValue = move(prev);
            if ((s16)nameSoundValue != prev) {
                sound_play(5);
            }
        }
    }
}
void name_sound_commit_value(void)
{
    nameSoundSeq[1] = nameSoundValue;
    nameSoundValue = 0;
    nameSoundState = 2;
}
void name_sound_commit_next(void)
{
    *(u16 *)((u8 *)nameSoundSeq + 4) = nameSoundValue;
    nameSoundValue = 0;
    namePromptRow = 0;
    nameSoundState = 3;
    menu_begin_close();
}
void name_sound_set_done(void)
{
    nameSoundState = 4;
    menu_begin_close();
}
void name_sound_load_value(void)
{
    u16 value = nameSoundSeq[1];
    nameSoundState = 1;
    nameSoundValue = value;
}
s16 name_sound_move_position(s16 position)
{
    s32 input = *inputPad1;

    if (input == 0x2000) {
        position++;
        if (position >= 4) {
            position = 0;
        }
    } else if (input == 0x8000) {
        position--;
        if (position < 0) {
            position = 3;
        }
    }
    return position;
}
s16 name_sound_move_index(s16 day)
{
    switch ((u32)*inputPad1 & 0xF000) {
    case 0x1000:
        day -= 7;
        if (day < 0) {
            day += 28;
            if (day < 23) {
                day += 7;
            }
        }
        break;
    case 0x4000:
        day += 7;
        if (day >= 30) {
            day -= 28;
            if (day >= 7) {
                day -= 7;
            }
        }
        break;
    case 0x8000:
        day--;
        if (day < 0) {
            day += 30;
        }
        break;
    case 0x2000:
        day++;
        if (day >= 30) {
            day -= 30;
        }
        break;
    }
    return day;
}
void name_prompt_draw(s32 id, u16 value);
s32 sound_play(s16 sound);

void name_sound_handle_input(void)
{
    s32 *input;

    name_prompt_draw(0x3A7, namePromptRow);
    if (menu_phase_is_result() != 0) {
        input = inputPad1;
        if (input[2] == 0x40 && namePromptRow == 0) {
            sound_play(0);
            nameSoundState = 5;
            menu_begin_close();
        } else if (input[2] == 0x40 || input[2] == 0x20) {
            sound_play(1);
            nameSoundState = 2;
            nameSoundValue = nameSoundSeq[2];
            menu_begin_close();
        } else if (input[0] & 0x5000) {
            sound_play(5);
            namePromptRow = (namePromptRow + 1) & 1;
        }
    }
}
void name_marker_draw(u16 day);
void name_entry_key_input(void);

void name_grid_step(void)
{
    menu_set_sound_enabled(0);
    text_set_input(0);
    menu_request_open(3, 0x3A8);
    name_title_prepare(nameTitleMode);
    if (namePhase == 2) {
        name_entry_confirm_input();
    } else {
        name_marker_draw(nameCursorCell);
        name_grid_draw_caption();
        name_entry_key_input();
    }
}
/* Seeded reverse-fill candidate remained two differences: retail materializes
 * -1 before the loop count, while the natural C declaration order emitted the
 * count first. Preserve ASM rather than force declaration scheduling. */
void name_buffer_clear(void)
{
    u16 value = 0xFFFF;
    s16 *p = nameBuffer + 8;
    s32 i = 8;

    do {
        *p = value;
        i--;
        p--;
    } while (i >= 0);
}
extern u16 nameDisplayText[];
void name_title_build_name(u16 *buf);
void CopyMainUiTextTableString(u16 *buf);

void name_title_prepare(s32 fromName)
{
    u32 i;
    void (*copy)(u16 *);

    for (i = 0; i < 16; i++) {
        nameDisplayText[i] = 0xFFD;
    }
    nameDisplayText[15] = 0xFFFF;
    if (fromName != 0) {
        copy = name_title_build_name;
    } else {
        copy = CopyMainUiTextTableString;
    }
    copy(nameDisplayText);
    window_frame_draw_at(*(s32 *)((u8 *)currentFrame + 0xD8) + 0x68, 1, 0xAB, 0x20, 0x85, 0x12);
    DrawTextWrapper(*(u8 **)((u8 *)currentFrame + 0xD8) + 0x64, nameDisplayText, 0xAD, 0x23);
}
void name_title_build_name(u16 *dst)
{
    u16 *start;
    u16 *src;
    u16 *s;
    u8 *work;
    s32 i;

    start = dst;
    i = 0;
    src = (u16 *)nameBuffer;
    do {
        if (src[i] == 0xFFFF) {
            work = gameState;
            start[i] = (*(u32 *)(work + 8) % 15 != 0) ? 0xFFD : 0x75;
            break;
        }
        *dst++ = src[i];
        i++;
    } while (i < 8);
    s = (u16 *)((u8 *)storage8005393A + mainUiTextTableRuntimeOffset[0]);
    start += 9;
    for (i = 0; s[i] != 0xFFFF; i++) {
        start[i] = s[i];
    }
}
extern u16 storage8005392E[];
