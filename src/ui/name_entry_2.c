/* Included by src/ui.c; shares its private declarations. */
void name_grid_draw_caption(void)
{
    const u16 *text;

    window_frame_draw_at(*(s32 *)((u8 *)currentFrame + 0xD8) + 0x68, 1, 0x2D, 0x40, 0xE6, 0x56);
    if (nameShift == 0) {
        text = (const u16 *)(mainUiTextTableRuntimeOffset[0] + (s32)storage800537B2);
    } else {
        text = (const u16 *)(mainUiTextTableRuntimeOffset[0] + (s32)storage80053888);
    }
    text_set_spacing(1, 2);
    DrawTextWrapper(*(u8 **)((u8 *)currentFrame + 0xD8) + 0x64, text, 0x2F, 0x42);
    text_set_spacing(0, 0);
}
/* Ported from hmbtn-decomp (CC0), name_entry_key_input. */
u16 name_cursor_move(u16 day, u16 input, u32 kind);
u16 name_key_char(u16 day, u32 kind);
void name_entry_key_input(void)
{
    s16 sound;
    u16 cell;

    sound = -1;
    if (namePhase == 2) {
        return;
    }
    if (menu_phase_is_result() == 0) {
        return;
    }
    switch (((u32 *)inputPad1)[2]) {
    case 0x0800:
        if (nameLength == 0 || nameDirty == 0) {
            sound = 2;
        } else {
            menu_begin_close();
            sound = 0;
            namePromptToggle = 0;
            namePhase = 2;
        }
        break;
    case 0x0020:
        if (nameLength != 0) {
            sound = 1;
            nameLength--;
            ((u16 *)nameBuffer)[nameLength] = 0xFFFF;
        } else if (nameCancelAllowed != 1) {
            sound = 2;
        } else {
            menu_begin_close();
            sound = 1;
            namePhase = 3;
        }
        break;
    case 0x0040:
        if (nameCursorCell == 0xD) {
            nameShift = (nameShift + 1) & 1;
            sound = 5;
            break;
        }
        if (nameCursorCell != 0x53) {
            sound = 0;
        } else {
            if (nameLength == 0 || nameDirty == 0) {
                sound = 2;
            } else {
                menu_begin_close();
                sound = 0;
                namePromptToggle = 0;
                ((u16 *)nameBuffer)[nameLength + 1] = 0xFFFF;
                namePhase = 2;
            }
            break;
        }
        sound = 0;
        if (nameLength < 8) {
            if (nameCursorCell == 0x1B) {
                ((u16 *)nameBuffer)[nameLength] = 0xFFD;
            } else {
                ((u16 *)nameBuffer)[nameLength] = name_key_char(nameCursorCell, nameShift);
                nameDirty = 1;
            }
            nameLength++;
        }
        break;
    case 0x0004 | 0x0001:
    case 0x0008 | 0x0002:
        nameShift = (nameShift + 1) & 1;
        sound = 5;
        break;
    default:
        cell = nameCursorCell;
        nameCursorCell = name_cursor_move(cell, ((u32 *)inputPad1)[0], nameShift);
        if (cell != nameCursorCell) {
            sound = 5;
        }
        break;
    }
    if (sound >= 0) {
        sound_play(sound);
    }
}

void name_entry_confirm_input(void)
{
    s32 *input;

    name_prompt_draw(0x3A6, namePromptToggle);
    if (menu_phase_is_result() != 0) {
        input = inputPad1;
        if (input[2] == 0x40 && namePromptToggle == 0) {
            menu_begin_close();
            sound_play(0);
            namePhase = 4;
        } else if (input[2] == 0x40 || input[2] == 0x20) {
            menu_begin_close();
            sound_play(1);
            namePhase = 1;
        } else if (input[0] & 0x5000) {
            sound_play(5);
            namePromptToggle = (namePromptToggle + 1) & 1;
        }
    }
}
extern u16 textBank[];
void ui_draw_icon_0b_1(s32 arg0, s16 arg1, s16 arg2);

void name_prompt_draw(s32 id, u16 value)
{
    menu_set_sound_enabled(0);
    text_set_input(0);
    menu_request_open(3, id);
    if (menu_phase_is_result() != 0) {
        DrawTextWrapper(*(u8 **)((u8 *)currentFrame + 0xD8) + 0x58,
                        (const u16 *)(mainUiTextTableRuntimeOffset[0] + (s32)textBank), 0x93, 0x4E);
        window_frame_draw_at(*(s32 *)((u8 *)currentFrame + 0xD8) + 0x5C, 1, 0x91, 0x4C, 0x28, 0x1E);
        ui_draw_icon_0b_1(*(s32 *)((u8 *)currentFrame + 0xD8) + 0x54, 0x99, value * 12 + 0x54);
    }
}
extern u8 *nameGridRowLastCols[];
extern u8 *nameGridColLastRows[];
extern s32 (*nameCursorWrapFixups[])(s32 slot, s32 week, s32 forward);
s32 name_cursor_wrap_vertical(s32 slot, s32 week, s32 forward);

u16 name_cursor_move(u16 day, u16 input, u32 kind)
{
    s32 dir;
    s32 pos[2];
    s32 *axis;
    s32 limit;
    s32 (*fix)(s32 slot, s32 week, s32 forward);
    s32 forward;

    if (input == 0x1000) {
        dir = 0;
    } else if (input == 0x4000) {
        dir = 2;
    } else if (input == 0x8000) {
        dir = 1;
    } else if (input == 0x2000) {
        dir = 3;
    } else {
        return day;
    }
    if (kind >= 2) {
        return day;
    }
    pos[0] = (u16)(day % 14);
    pos[1] = (u16)(day / 14);
    forward = dir & 2;
    if (dir & 1) {
        axis = &pos[0];
        limit = nameGridRowLastCols[kind][pos[1]];
        fix = nameCursorWrapFixups[kind];
    } else {
        axis = &pos[1];
        limit = nameGridColLastRows[kind][pos[0]];
        fix = name_cursor_wrap_vertical;
    }
    *axis += forward ? 1 : -1;
    if (*axis < 0) {
        *axis = limit;
    }
    if (limit < *axis) {
        *axis = 0;
    }
    *axis = fix(pos[0], pos[1], forward);
    return pos[0] + pos[1] * 14;
}
s32 name_cursor_wrap_grid0(s32 col, s32 row, s32 forward)
{
    s32 result;
    if (row != 5)
        goto unchanged;
    if ((u32)(col - 1) < 4) {
        result = 0;
        if (forward == 0)
            goto done;
        return 5;
    }
    if ((u32)(col - 6) < 7) {
        result = 5;
        if (forward == 0)
            goto done;
        return 13;
    }
unchanged:
    return col;
done:
    return result;
}
s32 name_cursor_wrap_vertical(s32 col, s32 row, s32 forward)
{
    s32 result;
    if (col != 13)
        goto unchanged;
    if ((u32)(row - 2) >= 3)
        goto unchanged;
    if (forward != 0) {
        result = 5;
    } else {
        result = 1;
    }
    goto done;
unchanged:
    return row;
done:
    return result;
}
s32 name_cursor_wrap_grid1(s32 col, s32 row, s32 forward)
{
    if ((row < 2) && ((u32)(col - 0xA) < 3)) {
        if (forward == 0) {
            return 9;
        }
        return 0xD;
    }
    if (row == 5) {
        return 0xD;
    }
    return col;
}
/* The signed slot local and in-place gap addition retain the retail slot register. */
void name_marker_draw(u16 day)
{
    u16 week = day / 14;
    s32 slot = (u16)(day % 14);
    s32 gap = slot >= 5;

    if (slot >= 10) {
        gap++;
    }
    if (slot >= 13) {
        gap += 2;
    }
    slot += gap;
    ui_draw_icon_0b_1(*(s32 *)((u8 *)currentFrame + 0xD8) + 0x60, slot * 9 + 0x32,
                      week * 14 + 0x48);
}
extern u16 (*nameKeyCharHandlers[])(u16 slot, u16 week);

u16 name_key_char(u16 day, u32 kind)
{
    s32 slot;

    if (kind >= 2) {
        return 0xFFFF;
    }
    slot = day % 14;
    return nameKeyCharHandlers[kind](slot, day / 14);
}
s32 name_key_cell_letters(s32 col, s32 row)
{
    s32 varA0;
    s32 varV0;
    s32 varV1;

    varA0 = col;
    varV1 = 0;
    if (varA0 < 5) {
        varV0 = 5;
    } else {
        varV1 = 0x20;
        if (varA0 < 0xA) {
            varA0 -= 5;
            varV0 = 5;
        } else {
            varV1 = 0x40;
            varA0 -= 0xA;
            varV0 = 3;
        }
    }
    return (varV1 + ((row * varV0) + varA0)) & 0xFFFF;
}
u16 name_key_char_symbols(s32 col, s32 row)
{
    return nameSymbolChars[(row * 10) + col];
}
/* CONFIRMED: ui_draw_icon sign-extends arg3, loads arg4 with lh from
 * frame + 0x10, masks arg1/arg2 internally, and has no return value. */
void ui_draw_icon(s32 arg0, s32 arg1, s32 arg2, s16 arg3, s16 arg4);
