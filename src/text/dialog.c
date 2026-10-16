/* Included by src/text.c; shares its private declarations. */
void text_set_spacing(s16 x, s16 y)
{
    textSpacingX = x;
    textSpacingY = y;
}
void text_set_palette(u32 palette)
{
    if (palette >= 7) {
        palette = 6;
    }
    textPalette = palette;
}
s32 text_get_palette(void)
{
    return textPalette;
}
void text_set_speed(u32 index)
{
    if (index >= 3) {
        index = 1;
    }
    textGlyphInterval = textSpeedIntervals[index];
}
s32 text_get_speed(void)
{
    if (textGlyphInterval == 0) {
        return 0;
    }
    if (textGlyphInterval == 5) {
        return 2;
    }
    return 1;
}
void text_begin_typing(void)
{
    if (textPhase == 0) {
        textPhase = 1;
    }
}
void text_advance_glyph(void)
{
    if (textPhase == 1 || textPhase == 3) {
        textPhase = 0;
        textIndex++;
    }
}
void text_set_input(s32 value)
{
    textInput = value;
}
s32 text_get_input(void)
{
    return textInput;
}
s16 GetDialogUiLabelValue(void)
{
    return textWaitFrames;
}
/* CONFIRMED: a0/a1 pass through as the display and stream; a2/a3 are
 * sign-extended before the parser call. The six outgoing stack words map to
 * ParseDialogTextStream's y, scale, mode, aux, recursive, and lineReset. */
void DrawTextWrapper(void *display, const u16 *stream, s16 x, s16 y)
{
    void *scratch;

    scratch = gs_work_alloc(0);
    gs_work_release(ParseDialogTextStream(display, scratch, stream, x, y, 0x1000, 0, 0, 0, 0));
}
/* CONFIRMED prototype: a0/a1 are retained as raw pointers, a2/a3 and both
 * stack arguments are loaded with lh and sign-extended. The wrapper proves
 * the fifth argument is passed even though the callee reads only the sixth. */
u16 RenderDialogTextPage(u32 *ot, u16 *stream, s16 x, s16 y, s16 unused, s16 bottom);
void text_window_draw(u32 *ot, u16 *stream, s16 x, s16 y, s16 unused, s16 bottom)
{
    RenderDialogTextPage(ot, stream, x, y, unused, bottom);
}
extern s16 text_end_y(void);
void text_end_y_discard(void)
{
    text_end_y();
}
u16 text_get_last_glyph(void)
{
    return textLastGlyph;
}
u16 text_get_bank(void)
{
    return textBankId;
}
/* CONFIRMED: retail has two direct sh stores, so the ABI is a full-word
 * argument whose low half is copied to both page selectors. */
void text_set_bank(s32 page);
void text_set_bank(s32 page)
{
    textBankId = page;
    textBankLoaded = page;
}
/* Ported from hmbtn-decomp (CC0), text_draw_centered. */
void text_draw_centered(u32 *ot, u16 *text, s16 x, s16 y, s16 width)
{
    u8 *packet;
    s16 limit;

    packet = gs_work_alloc(0);
    limit = textSpacingY + 16;
    packet = text_draw_lines_centered(ot, packet, text, x, y, width, 0x1000, limit, textSpacingX);
    gs_work_release(packet);
}

/* Ported from hmbtn-decomp (CC0), text_draw_right. */
void text_draw_right(u32 *ot, u16 *text, s16 x, s16 y, s16 width)
{
    u8 *packet;
    s16 limit;

    packet = gs_work_alloc(0);
    limit = textSpacingY + 16;
    packet = text_draw_lines_right(ot, packet, text, x, y, width, 0x1000, limit, textSpacingX);
    gs_work_release(packet);
}

/* CONFIRMED: same allocation/parser/free family as DrawTextWrapper. The
 * fifth wrapper argument is forwarded unchanged to the parser's aux slot. */
void text_draw_semi(void *display, const u16 *stream, s16 x, s16 y, s32 aux)
{
    void *scratch;

    scratch = gs_work_alloc(0);
    gs_work_release(ParseDialogTextStream(display, scratch, stream, x, y, 0x1000, 0, aux, 0, 0));
}
u16 *text_list_append(u16 *dst, u16 *src)
{
    if (*dst != 0xFFFF) {
        do {
            dst++;
        } while (*dst != 0xFFFF);
    }

    *dst = *src;
    if (*src != 0xFFFF) {
        do {
            src++;
            dst++;
            *dst = *src;
        } while (*src != 0xFFFF);
    }

    return dst;
}
void text_list_copy(u16 *dst, u16 *src)
{
    *dst = *src;
    if (*src != 0xFFFF) {
        do {
            src++;
            dst++;
            *dst = *src;
        } while (*src != 0xFFFF);
    }
}
/* Seeded comparison candidate generated 18 instructions versus retail 19
 * and reported 19 aligned differences from +0x00. Retail starts with a jump
 * whose delay slot materializes unsigned 0xFFFF; the natural loop starts with
 * the first loads and forms different branch targets. Preserve ASM after the
 * first bounded form; no linked verification was accepted for this miss. */
s32 text_list_compare(u16 *a, u16 *b)
{
    u16 ca, cb;
    u16 term = 0xFFFF;

    goto compare;
    for (;;) {
        b++;
    compare:
        ca = *a;
        cb = *b;
        if (ca < cb) {
            return -1;
        }
        if (cb < ca) {
            return 1;
        }
        a++;
        if (cb == term) {
            return 0;
        }
    }
}
s32 text_match_either(u16 *a, u16 *b, u16 *c)
{
    s32 r;
    u16 term = 0xFFFF;

    goto compare;
    for (;;) {
        if (*a++ == term) {
            break;
        }
        b++;
        c++;
    compare:
        if (*a == *b || *a == *c) {
            r = 0;
            continue;
        }
        r = 1;
        if (*a < *b) {
            r = -1;
        }
        break;
    }
    return r;
}
/* Ported from hmbtn-decomp (CC0), text_printf. */
#include "psyq_libgpu.h"
extern DialogRenderContext *currentFrame;
extern FarmLoadArea farmArea;
extern u8 gameState[];
s32 loading_screen_is_active(void);
void prim_add_tile(u_long *ot, s16 x, s16 y, s16 w, s16 h, s32 packed, s16 flag);
int sprintf(char *buffer, const char *fmt, ...);
u16 text_char_code(u8 c);
u8 *ParseDialogTextStream(u_long *display, u8 *scratch, u16 *stream, s16 x, s16 y, s32 scale,
                          s32 mode, s32 aux, s32 recursive, s32 lineReset);
extern char textFmtDec[];
extern char textFmtHex[];
extern char textFmtInt[];
s32 text_get_palette(void);
extern u16 textGlyphs[64];
extern s16 textOffsetX;
extern s16 textOffsetY;
extern s16 textOriginX;
extern s16 textOriginY;
extern s16 textPenX;
extern s16 textPenY;
void text_printf(u8 *fmt, ...);
void text_set_palette(u32 arg0);
extern s16 textSpacingX;
extern s16 textSpacingY;
typedef void *va_list;

void text_printf(u8 *fmt, ...)
{
    va_list args;
    u8 number[16];
    s16 lineLength[5];
    u8 *p;
    u8 *str;
    s16 lines;
    s32 i;
    s16 savedA;
    s16 savedB;
    u_long *ot;

    if (gameStateView.globalFlags & GLOBAL_FLAG_TRANSITION) {
        return;
    }
    if (farmAreaView.mode.view != FARM_VIEW_NORMAL) {
        return;
    }
    if (farmAreaView.mode.eventMode != FARM_MODE_IDLE) {
        return;
    }
    if (loading_screen_is_active() == 1) {
        return;
    }

    for (i = 0; i < 5; i++) {
        lineLength[i] = 0;
    }
    lines = 0;
    (args = ((char *)&(fmt) + (((sizeof(fmt) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))));
    p = fmt;
    i = 0;
    while (1) {
        if (*p == '\0') {
            textGlyphsView[i] = 0xFFFF;
            break;
        }
        if (*p != '%') {
            textGlyphsView[i] = text_char_code(*p);
            i++;
            if (*p == '\n') {
                lines++;
            } else {
                lineLength[lines]++;
            }
        } else {
            p++;
            str = number;
            switch (*p) {
            case 'd':
                sprintf(
                    (char *)str, textFmtDecView,
                    (args = ((char *)(args)) +=
                     (((sizeof(int) + sizeof(int) - 1) / sizeof(int)) * sizeof(int)),
                     *((int *)((char *)(args) -
                               (((sizeof(int) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))))));
                goto copy;
            case 'x':
                sprintf(
                    (char *)str, textFmtHexView,
                    (args = ((char *)(args)) +=
                     (((sizeof(int) + sizeof(int) - 1) / sizeof(int)) * sizeof(int)),
                     *((int *)((char *)(args) -
                               (((sizeof(int) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))))));
                goto copy;
            case 'i':
                sprintf(
                    (char *)str, textFmtIntView,
                    (args = ((char *)(args)) +=
                     (((sizeof(int) + sizeof(int) - 1) / sizeof(int)) * sizeof(int)),
                     *((int *)((char *)(args) -
                               (((sizeof(int) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))))));
                goto copy;
            case 's':
                str =
                    (args = ((char *)(args)) +=
                     (((sizeof(u8 *) + sizeof(int) - 1) / sizeof(int)) * sizeof(int)),
                     *((u8 **)((char *)(args) -
                               (((sizeof(u8 *) + sizeof(int) - 1) / sizeof(int)) * sizeof(int)))));
            copy:
                while (*str != '\0') {
                    textGlyphsView[i] = text_char_code(*str);
                    i++;
                    lineLength[lines]++;
                    str++;
                }
                break;
            }
        }
        p++;
        if (i >= 64) {
            break;
        }
    }
    savedA = textSpacingXView;
    savedB = textSpacingYView;
    textGlyphsView[63] = 0xFFFF;
    textSpacingXView = 0;
    textSpacingYView = 0;
    i = text_get_palette();
    text_set_palette(0);
    if (textOffsetXView < 321 && textOffsetYView < 121) {
        ot = currentFrameView->ot;
        currentFrameView->primCursor.bytes =
            ParseDialogTextStream(ot, currentFrameView->primCursor.bytes, textGlyphsView,
                                  textOriginXView + textOffsetXView,
                                  textOriginYView + textOffsetYView, 0x1000, 0, 0, 0, 1);
        text_set_palette(i);
        for (i = 0; i <= lines; i++) {
            if (lineLength[i] != 0) {
                prim_add_tile(ot, (i == 0) ? textOriginXView + textOffsetXView : textOriginXView,
                              textOriginYView + textOffsetYView + i * 12, lineLength[i] * 8, 12,
                              0xFFFFFF, 0);
            }
        }
        textSpacingXView = savedA;
        textSpacingYView = savedB;
        textOffsetXView = textPenXView - textOriginXView;
        textOffsetYView = textPenYView - textOriginYView;
    }
}

void text_set_origin(u16 x, u16 y)
{
    textOriginX = x;
    textOriginY = y;
    textOffsetX = 0;
    textOffsetY = 0;
}
void text_reset_cursor(void)
{
    textOffsetX = 0;
    textOffsetY = 0;
}
/* Ported from hmbtn-decomp (CC0), text_window_frame. */
#include "psyq_libgpu.h"
typedef struct {
    u_long tag;
    u_long code[2];
} DR_AREA;
void SetDrawArea(DR_AREA *p, RECT *r);
extern DialogRenderContext *currentFrame;
void farm_work_sector_release(void);
void *gs_work_alloc(s32 size);
extern DialogNavigationInput *inputPad1;
void text_advance_glyph(void);
u16 *text_advance_line(u16 *stream, s32 occurrence);
extern s32 textBankReading;
extern void *textBankResult;
void text_begin_typing(void);
extern s32 textBlipToggle;
extern s16 textChoice;
u32 text_choice_face_bank(u16 arg0);
s32 text_choice_menu(s16 x, s16 y, s16 *cursor, s16 count);
u16 text_count_entries(u16 *stream, u16 marker, s16 maxWidth);
u16 text_count_matches(u16 *stream, u16 marker, s16 limit, u16 *consumedOut);
extern u16 *textCurrent;
void text_draw_face(u32 *ot, s16 x, s16 y, s16 pagePresent);
void text_draw_typed(void *display, const u16 *stream, s16 x, s16 y);
extern s16 textFaceBank;
extern s32 textFaceReload;
extern s32 textFaceShown;
u16 *text_find_nth(u16 *stream, u16 marker, s32 occurrence, s32 maxWidth);
s32 text_get_speed(void);
u16 text_get_state(void);
extern u16 textIndex;
extern s32 textInput;
u16 *text_insert_source(u16 code);
extern u16 textLastCode;
extern u16 textLastGlyph;
void LoadPortraitPageFromMessege0(s16 page, s32 singleAttempt);
void text_next_page(void);
extern s32 textNoScroll;
void text_reset_state(void);
extern s32 textScrollHold;
extern s32 textScrollStarted;
extern s16 textScrollState;
extern s32 textScrollTimer;
extern s16 textScrollY;
extern s32 textScrolling;
void text_set_speed(u32 arg0);
extern s32 storage8005E4B0;
void text_start_choice(void);
extern u16 textState;
extern s16 textWaitFrames;
u16 RenderDialogTextPage(u32 *ot, u16 *stream, s16 x, s16 y, s16 unused, s16 bottom);
s32 vsync_elapsed(s32 interval, s32 *last, s32 restart);

TextWindowResult RenderDialogTextPage(u32 *ot, u16 *text, s16 x, s16 y, s16 unused, s16 height)
{
    u16 *p;
    s32 step;
    s32 result;
    TextWindowResult ret;
    s32 held;
    s32 pressed;
    enum TextWindowState state;
    u16 *mark;
    u16 *line;
    s32 i;
    s32 total;
    s16 lineY;
    s32 savedMode;
    DR_AREA *restore;
    DR_AREA *clip;
    RECT area;

    if (storage8005E4B0View != text) {
        storage8005E4B0View = text;
        textCurrentView = text;
        textIndexView = 0;
        p = text;
        if (*p != 0xFFFF) {
            do {
                if (*p & 0x4000) {
                    text_insert_source(*p);
                }
                textIndexView++;
                p++;
            } while (*p != 0xFFFF);
        }
        textScrollTimerView = 0;
        textScrollYView = 0;
        textScrollStateView = 3;
        textScrollingView = 1;
        text_reset_state();
        textScrollStartedView = 0;
        textFaceReloadView = 0;
        textBankResultView = 0;
        textFaceBankView = -1;
        textFaceShownView = 0;
        textBankReadingView = 0;
        textWaitFramesView = 0;
        textNoScrollView = 0;
        textScrollHoldView = 0;
        textBlipToggleView = 0;
    }
    step = 1;
    savedMode = text_get_speed();
    result = TEXT_WINDOW_PRINTING;
    if (textInputView != 0) {
        held = inputPad1View->held;
        pressed = inputPad1View->pressed;
    } else {
        held = 0;
        pressed = 0;
    }
    state = text_get_state();
    switch (state) {
    case TEXT_STATE_PRINTING:
        result = TEXT_WINDOW_PRINTING;
        if (held & (PAD_CIRCLE | PAD_CROSS)) {
            text_set_speed(0);
        }
        break;
    case TEXT_STATE_SCROLLING:
        if (textScrollingView == 0) {
            result = TEXT_WINDOW_SCROLL_WAIT;
            if ((pressed & (PAD_CIRCLE | PAD_CROSS)) && textScrollStateView >= 4) {
                textScrollingView = 1;
                textScrollStateView = 0;
                result = TEXT_WINDOW_PRINTING;
            }
        } else {
            result = TEXT_WINDOW_SCROLLING;
            if (held & (PAD_CIRCLE | PAD_CROSS)) {
                step = 2;
            }
        }
        break;
    case TEXT_STATE_END:
        farm_work_sector_release();
        if ((pressed & (PAD_CIRCLE | PAD_CROSS)) && textStateView == TEXT_STATE_END) {
            storage8005E4B0View = 0;
            return TEXT_WINDOW_CLOSED;
        }
        result = TEXT_WINDOW_END_WAIT;
        break;
    case TEXT_STATE_PAGE_BREAK:
        result = TEXT_WINDOW_PAGE_WAIT;
        if (pressed & (PAD_CIRCLE | PAD_CROSS)) {
            result = TEXT_WINDOW_PRINTING;
            text_advance_glyph();
            if (textCurrentView[textIndexView - 1] == 0xFFFE) {
                if (textScrollStartedView == 0) {
                    textScrollStateView -= text_count_matches(
                        textCurrentView, 0xFFFE, textIndexView != 0 ? textIndexView : 1, 0);
                } else {
                    textScrollStateView = -1;
                }
            }
        }
        break;
    case TEXT_STATE_CHOICE_2:
    case TEXT_STATE_CHOICE_3:
    case TEXT_STATE_CHOICE_4:
        result = textStateView;
        if (text_choice_menu(x, y, &textChoiceView, textStateView - TEXT_STATE_CHOICE_2 + 2)) {
            storage8005E4B0View = 0;
            return text_choice_face_bank(textChoiceView + 1);
        }
        break;
    }
    textStateView = state;
    text_next_page();
    text_start_choice();
    lineY = y;
    if (textNoScrollView == 0) {
        if (textScrollingView == 1 &&
            text_count_matches(textCurrentView, 0xFFFE, textIndexView != 0 ? textIndexView : 1,
                               0) >= 4) {
            textScrollStartedView = 1;
            if (textScrollTimerView == 0) {
                text_begin_typing();
                textScrollStateView++;
            }
            if (vsync_elapsed(1, &textScrollTimerView, 1) == 1) {
                textScrollYView += step;
            }
            if (textScrollYView >= 14) {
                text_advance_glyph();
                textCurrentView = text_advance_line(textCurrentView, 1);
                textScrollTimerView = 0;
                textScrollYView = 0;
            } else {
                lineY = y - textScrollYView;
            }
        } else {
            textScrollYView = 0;
        }
    } else if (textScrollingView == 1) {
        textScrollStartedView = 1;
        textScrollStateView = 4;
        mark = text_find_nth(textCurrentView, TEXT_CODE_PAGE_MARK, 1, 0);
        if (textScrollTimerView != 0) {
            line = text_find_nth(textCurrentView, 0xFFFE, 4, 0);
            if (line != 0) {
                if (line[1] != 0xFFFF) {
                    if (mark != 0 && mark < line) {
                        line = text_find_nth(mark, 0xFFFE, 1, 0);
                    }
                    textCurrentView = line + 1;
                }
            } else if (mark != 0) {
                textCurrentView = mark + 1;
                if (*textCurrentView == 0xFFFE) {
                    textCurrentView++;
                }
            }
        }
        textScrollTimerView = 1;
        mark = text_find_nth(textCurrentView, TEXT_CODE_PAGE_MARK, 1, 0);
        line = text_find_nth(textCurrentView, 0xFFFE, 4, 0);
        if (line != 0) {
            if (mark != 0 && mark < line) {
                textIndexView = text_count_entries(textCurrentView, TEXT_CODE_PAGE_MARK, 0);
            } else {
                for (i = 0, total = 0, p = textCurrentView; i < 4; i++) {
                    total += text_count_entries(p, 0xFFFE, 0);
                    if (i < 3) {
                        while (*p != 0xFFFE) {
                            p++;
                        }
                    }
                    p++;
                }
                textIndexView = total;
            }
        } else {
            line = text_find_nth(textCurrentView, 0xFFFF, 1, 0);
            textIndexView = text_count_entries(
                textCurrentView, (mark != 0 && mark < line) ? TEXT_CODE_PAGE_MARK : 0xFFFF, 0);
        }
    }
    if (textScrollHoldView == 0 && textScrollStateView >= 4) {
        textScrollingView = 0;
        text_begin_typing();
    }
    ret = result;
    restore = gs_work_alloc(12);
    clip = gs_work_alloc(12);
    area.x = x;
    area.y = y + currentFrameView->drawEnv.clip.y;
    area.w = 320 - x;
    area.h = height;
    SetDrawArea(clip, &area);
    SetDrawArea(restore, &currentFrameView->drawEnv.clip);
    (((P_TAG *)(restore))->addr = (u_long)((u_long)(((P_TAG *)(ot))->addr))),
        (((P_TAG *)(ot))->addr = (u_long)(restore));
    text_draw_typed(ot, textCurrentView, x, lineY);
    (((P_TAG *)(clip))->addr = (u_long)((u_long)(((P_TAG *)(ot))->addr))),
        (((P_TAG *)(ot))->addr = (u_long)(clip));
    text_set_speed(savedMode);
    LoadPortraitPageFromMessege0(textFaceBankView, textFaceReloadView);
    text_draw_face(currentFrameView->otTags[0], x - 4, y - 84,
                   textFaceShownView ? textFaceBankView : -1);
    textLastGlyphView = textLastCodeView;
    return ret;
}

void text_window_clear(void)
{
    storage8005E4B0 = 0;
}
/* Ported from hmbtn-decomp (CC0), text_draw_string. */
#include "psyq_libgpu.h"
s32 sound_play(s16 sound);
extern s32 textBlipToggle;
s16 CalculatePortraitPageId(u16 code);
u8 *ParseDialogTextStream(u_long *display, u8 *scratch, u16 *stream, s16 x, s16 y, s32 scale,
                          s32 mode, s32 aux, s32 recursive, s32 lineReset);
extern s16 textFaceBank;
extern s32 textFaceReload;
extern s32 textFaceShown;
extern s16 textGlyphInterval;
u8 *text_glyph_sprite(u32 *ot, SPRT *sprt, u16 glyph, s16 x, s16 y, u8 r, u8 g, u8 b, u32 palette,
                      s32 semiTrans);
extern s32 textGlyphTimer;
extern u16 textIndex;
u16 *text_insert_source(u16 code);
extern u16 textLastCode;
extern s16 textLineGlyphs;
extern s16 textMarginX;
extern s32 textNoScroll;
extern s16 textOriginX;
extern s32 textPalette;
extern s16 textPenX;
extern s16 textPenY;
extern u16 textPhase;
extern s32 textScrollHold;
extern u16 *textSources[];
extern s16 textSpacingX;
extern s16 textSpacingY;
void ApplyPortraitPageDecode(s16 page);
extern s16 textWaitFrames;
extern s16 textWrapIndex;
s32 vsync_elapsed(s32 interval, s32 *last, s32 restart);

u8 *ParseDialogTextStream(u_long *ot, u8 *prim, u16 *text, s16 x, s16 y, s32 scale, s32 mode,
                          s32 semiTrans, s32 nested, s32 toOrigin)
{
    s32 fast;
    u16 *child;
    s16 glyphX;
    s16 glyphY;

    if (nested == 0) {
        textMarginXView = x;
        if (mode == 1) {
            textLineGlyphsView = 0;
            if (textPhaseView == TEXT_STATE_PRINTING) {
                if (vsync_elapsed(textGlyphIntervalView, &textGlyphTimerView, 1) == mode) {
                    if (textBlipToggleView != 0) {
                        sound_play(SOUND_TEXT_BLIP);
                        textBlipToggleView = 0;
                    } else {
                        textBlipToggleView = mode;
                    }
                    textIndexView++;
                } else if (text[textIndexView] > 0xE00 || (text[0] & 0x4000)) {
                    textIndexView++;
                }
            }
        }
    }
    textPenXView = x;
    textPenYView = y;
    fast = textPhaseView >= TEXT_STATE_CHOICE_2 && textPhaseView <= TEXT_STATE_CHOICE_4;
    while (1) {
        if (mode == 1) {
            if (textIndexView <= textLineGlyphsView) {
                break;
            }
            textLineGlyphsView++;
            if (textPhaseView != TEXT_STATE_SCROLLING && textWrapIndexView != textLineGlyphsView &&
                !fast) {
                textPhaseView = TEXT_STATE_PRINTING;
            }
        }
        if (*text == 0xFFFF) {
            if (mode != 0 && nested == 0 && !fast) {
                textPhaseView = TEXT_STATE_END;
            }
            break;
        }
        if (*text == 0xFFFE) {
            textPenYView += 12 + (s16)textSpacingYView;
            textPenXView = textMarginXView;
            if (toOrigin != 0) {
                textPenXView = textOriginXView;
            }
        } else if (*text == TEXT_CODE_PAGE_MARK) {
            if (textPhaseView != TEXT_STATE_SCROLLING) {
                textPhaseView = TEXT_STATE_PAGE_BREAK;
                textWrapIndexView = textLineGlyphsView;
            }
        } else if (*text == TEXT_CODE_PAGE_END) {
        } else if (*text >= TEXT_CODE_CHOICE_4 && *text <= TEXT_CODE_CHOICE_2) {
        } else if (*text >= TEXT_CODE_FACE_FIRST && *text <= TEXT_CODE_FACE_LAST) {
            textFaceBankView = CalculatePortraitPageId(*text);
        } else if (*text == TEXT_CODE_FACE_BANK) {
            ApplyPortraitPageDecode(textFaceBankView);
        } else if (*text == TEXT_CODE_FACE_HIDE) {
            textFaceShownView = 0;
        } else if (*text == TEXT_CODE_FACE_RELOAD) {
            textFaceReloadView = 1;
        } else if (*text == TEXT_CODE_FACE_SHOW) {
            textFaceShownView = 1;
        } else if (*text >= TEXT_CODE_WAIT_FIRST && *text <= TEXT_CODE_WAIT_LAST) {
            textWaitFramesView = TEXT_CODE_WAIT_LAST - *text;
        } else if (*text == TEXT_CODE_NO_SCROLL) {
            textNoScrollView = 1;
        } else if (*text == TEXT_CODE_FLAG_FF2) {
        } else if (*text == TEXT_CODE_SCROLL_HOLD) {
            textScrollHoldView = 1;
        } else if (*text & 0x4000) {
            child = text_insert_source(*text);
            if (child == 0) {
                child = textSourcesView[*text & 0xFFF];
            }
            if (child != 0) {
                prim = ParseDialogTextStream(ot, prim, child, textPenXView, textPenYView, scale,
                                             mode, semiTrans, 1, 0);
            }
        } else {
            if ((*text & 0xFFF) != TEXT_CODE_SPACE) {
                glyphX = x + (textPenXView - x) * scale / 0x1000;
                glyphY = y + (textPenYView - y) * scale / 0x1000;
                prim = text_glyph_sprite(ot, (struct Sprt *)prim, *text, glyphX, glyphY, 0x80, 0x80,
                                         0x80, textPaletteView, semiTrans);
            }
            textPenXView += 8 + textSpacingXView;
        }
        text++;
    }
    textLastCodeView = text[-1];
    return prim;
}

s16 text_end_y(void)
{
    s16 y = textPenY;

    if (textCurrent[textIndex - 1] == 0xFFFE) {
        y = (s16)(y - 0xC) - textSpacingY;
    }
    return y;
}
/* Ported from hmbtn-decomp (CC0), text_glyph_sprite. */
#include "psyq_libgpu.h"
u8 *text_glyph_sprite(u32 *ot, SPRT *sprt, u16 glyph, s16 x, s16 y, u8 r, u8 g, u8 b, u32 palette,
                      s32 semiTrans)
{
    DR_TPAGE *tpage;
    s32 column = glyph & 0xF;
    s32 row = (glyph & 0xF0) >> 4;

    setlen(sprt, 4);
    sprt->code = 0x64;
    if (palette >= 6) {
        palette = 0;
    }
    sprt->clut = getClut(896, textClutRows[palette]);
    sprt->r0 = r;
    sprt->g0 = g;
    sprt->b0 = b;
    sprt->code = semiTrans ? sprt->code | 2 : sprt->code & ~2;
    sprt->x0 = x;
    sprt->y0 = y;
    sprt->u0 = column * 8;
    sprt->v0 = row * 12;
    sprt->w = 8;
    sprt->h = 12;
    addPrim(ot, sprt);

    sprt++;
    tpage = (DR_TPAGE *)sprt;
    setlen(tpage, 1);
    tpage->code[0] = 0xE100060E;
    addPrim(ot, tpage);
    return (u8 *)(tpage + 1);
}

u16 *text_find_nth(u16 *stream, u16 marker, s32 occurrence, s32 maxWidth)
{
    u16 *cursor;
    u16 *start;
    u16 *found;
    s32 seen;
    u16 width;

    cursor = stream;
    seen = 0;
    found = 0;
    start = cursor;
    if (occurrence == 0) {
        return found;
    }
    while (1) {
        width = text_count_entries(start, *cursor, 0);
        if (maxWidth != 0 && width >= maxWidth) {
            break;
        }
        if (*cursor == 0xFFFF) {
            if (marker == 0xFFFF) {
                found = cursor;
            }
            break;
        }
        if (*cursor == marker) {
            seen++;
            if (seen >= occurrence) {
                found = cursor;
                break;
            }
        }
        cursor++;
    }
    return found;
}
u16 text_count_matches(u16 *stream, u16 marker, s16 limit, u16 *consumedOut)
{
    u16 *cursor;
    u16 matches;
    s32 consumed;
    u16 *expansion;
    u16 nestedConsumed;

    cursor = stream;
    matches = 0;
    consumed = 0;
    while (limit == 0 || consumed < limit) {
        if (*cursor == 0xFFFF) {
            consumed++;
            break;
        }
        if (*cursor == marker) {
            matches++;
        } else if (*cursor != 0xFFFE && (*cursor & 0x4000) != 0) {
            expansion = text_insert_source(*cursor);
            if (expansion == 0) {
                expansion = textSources[*cursor & 0xFFF];
            }
            if (expansion != 0) {
                if (limit != 0) {
                    matches +=
                        text_count_matches(expansion, marker, limit - consumed, &nestedConsumed);
                    consumed += nestedConsumed;
                } else {
                    matches += text_count_matches(expansion, marker, 0, 0);
                }
            }
        }
        consumed++;
        cursor++;
    }
    if (consumedOut != 0) {
        *consumedOut = consumed;
    }
    return matches;
}
u16 *text_advance_line(u16 *stream, s32 occurrence)
{
    u16 *found;
    u16 consumedWidth;

    found = text_find_nth(stream, 0xFFFE, occurrence, 0);
    consumedWidth = text_count_entries(stream, 0xFFFE, 0) + 1;
    if (found != 0) {
        textIndex -= consumedWidth;
        found++;
    }
    return found;
}
u16 text_count_entries(u16 *stream, u16 marker, s16 limit)
{
    u16 *cursor;
    s32 count;
    u16 *expansion;
    u16 *fallback;

    cursor = stream;
    count = 0;
    while (limit == 0 || count < limit) {
        if (*cursor == 0xFFFF || *cursor == marker) {
            count++;
            break;
        }
        if (*cursor != 0xFFFE && (*cursor & 0x4000) != 0) {
            expansion = text_insert_source(*cursor);
            if (expansion == 0) {
                fallback = textSources[*cursor & 0xFFF];
                if (fallback == 0) {
                    goto next_glyph;
                }
                expansion = fallback;
            }
            count += text_count_entries(expansion, marker, limit == 0 ? 0 : limit - count);
        }
    next_glyph:
        count++;
        cursor++;
    }
    return count;
}
void text_draw_typed(void *display, const u16 *stream, s16 x, s16 y)
{
    void *scratch;

    scratch = gs_work_alloc(0);
    gs_work_release(ParseDialogTextStream(display, scratch, stream, x, y, 0x1000, 1, 0, 0, 0));
}
u16 text_get_state(void)
{
    return textPhase;
}
void text_reset_state(void)
{
    textIndex = 0;
    textPhase = 0;
}
typedef struct TextLine41 {
    u16 c[41];
} TextLine41;

void *text_draw_lines_centered(void *display, void *scratch, const u16 *stream, s16 x, s16 y,
                               s16 width, s32 scale, s16 lineStep, s16 spacing)
{
    TextLine41 line;
    s32 done;
    u32 n;
    const u16 *src;

    src = stream;
    done = 0;
    do {
        line = *(TextLine41 *)src;
        line.c[40] = 0xFFFF;
        for (n = 0; n < 41; n++) {
            if (line.c[n] == 0xFFFE) {
                line.c[n] = 0xFFFF;
                n++;
                break;
            }
            if (line.c[n] == 0xFFFF) {
                done = 1;
                n++;
                break;
            }
        }
        src += n;
        scratch =
            text_draw_line_centered(display, (s32)scratch, line.c, x, y, width, scale, spacing);
        y += lineStep;
    } while (done != 1);
    return scratch;
}
s32 text_measure(const u16 *stream, s32 scale, s16 font);

void *text_draw_line_centered(void *display, s32 handle, const u16 *stream, s16 x, s16 y, s16 width,
                              s32 scale, s16 font)
{
    void *scratch;
    s32 left;

    text_set_spacing(font, 0);
    left = x + (width - text_measure(stream, scale, font)) / 2;
    scratch = gs_work_alloc(0);
    gs_work_release(ParseDialogTextStream(display, scratch, stream, left, y, scale, 0, 0, 0, 0));
    return gs_work_alloc(0);
}
s32 text_measure(const u16 *stream, s32 scale, s16 spacing)
{
    s32 width = 0;
    u16 code;
    const u16 *sub;

    while ((u16)(*stream + 2) >= 2) {
        code = *stream;
        if ((code & 0xF000) == 0x4000) {
            sub = textSources[code & 0xFFF];
            if (sub != 0) {
                width += text_measure(sub, scale, spacing);
            }
        } else {
            width += scale / 512;
            width += spacing;
        }
        stream++;
    }
    if ((stream[-1] & 0xFFF) == 0x6C) {
        width -= scale * 3 / 4096;
    }
    return width - spacing;
}
void *text_draw_line_right(void *display, s32 handle, const u16 *stream, s16 x, s16 y, s16 width,
                           s32 scale, s16 font)
{
    void *scratch;
    s32 left;

    text_set_spacing(font, 0);
    left = x + width - text_measure(stream, scale, font);
    scratch = gs_work_alloc(0);
    gs_work_release(ParseDialogTextStream(display, scratch, stream, left, y, scale, 0, 0, 0, 0));
    return gs_work_alloc(0);
}
void *text_draw_lines_right(void *display, void *scratch, const u16 *stream, s16 x, s16 y,
                            s16 width, s32 scale, s16 lineHeight, s16 spacing)
{
    TextLine41 line;
    s32 done;
    u32 n;
    const u16 *src;

    src = stream;
    done = 0;
    do {
        line = *(TextLine41 *)src;
        line.c[40] = 0xFFFF;
        for (n = 0; n < 41; n++) {
            if (line.c[n] == 0xFFFE) {
                line.c[n] = 0xFFFF;
                n++;
                break;
            }
            if (line.c[n] == 0xFFFF) {
                done = 1;
                n++;
                break;
            }
        }
        src += n;
        scratch = text_draw_line_right(display, (s32)scratch, line.c, x, y, width, scale, spacing);
        y += lineHeight;
    } while (done != 1);
    return scratch;
}
/* Semantics are recovered but source shape remains unresolved. Likely:
 *
 *     s32 text_choice_menu(s16 x, s16 y, s16 *selection, s16 count)
 *
 * Retail first loads inputPad1, tests flags +0 for 0x4000 then 0x1000,
 * and tests flags +8 for 0x40. The first two paths play sound 5 and cycle the
 * signed selection; the third plays sound 0 and returns selection + 1. A zero
 * result draws with currentFrame->+0xD8 at x-2 and
 * y + (textSpacingY + 12) * selection.
 *
 * New type evidence: boot initializes inputPad1 directly to 0x8005E870, and
 * boot.c independently establishes currentFrame as a pointer to the shared
 * context. They are therefore pointer variables, not arrays; the array
 * declarations above are a code-generation device which keeps this -G8 unit
 * on the retail %hi/%lo access path. Declaring either as its true pointer type
 * instead uses $gp and worsens the candidate to 83 instructions / 70 diffs.
 * The best second-attempt candidate using the wrapper form was 85 instructions
 * / 60 diffs (retail: 86), slightly worse than the recorded 84 / 59 candidate.
 * The remaining mismatch is store/branch scheduling and register allocation,
 * not the input or render-context field shape. */
s32 text_choice_menu(s16 x, s16 y, s16 *cursor, s16 count)
{
    s32 result = 0;

    if (inputPad1->flags00 & 0x4000) {
        sound_play(5);
        (*cursor)++;
        *cursor = *cursor < count ? *cursor : 0;
    } else if (inputPad1->flags00 & 0x1000) {
        sound_play(5);
        (*cursor)--;
        *cursor = *cursor < 0 ? count - 1 : *cursor;
    } else if (inputPad1->flags08 & 0x40) {
        sound_play(0);
        result = *cursor + 1;
    }
    if (result == 0) {
        text_draw_cursor(currentFrame->display, x - 2, y + (textSpacingY + 12) * *cursor);
    }
    return result;
}
void text_draw_cursor(void *display, s16 x, s16 y)
{
    ui_draw_icon_0b_1(display, x + 8, y + 6);
}
u32 text_choice_face_bank(u16 choice)
{
    u16 index = choice - 1;

    if (index >= 4) {
        index = 0;
    }
    return textChoiceFaceBanks[index];
}
/* CONFIRMED prototype: the first argument is an ordering-table link, and the
 * following three arguments each pass through an explicit sll/sra pair.
 * `pagePresent == -1` is the direct no-op guard. */
void text_draw_face(u32 *ot, s16 x, s16 y, s16 pagePresent);
/* Plain stores in clut, RGB, XY, WH, UV order (field order from hmbtn-decomp,
 * CC0): no volatile needed. */
#include "psyq_libgpu.h"
void text_draw_face(u32 *ot, s16 x, s16 y, s16 pagePresent)
{
    SPRT *sprite;
    DR_TPAGE *mode;

    if (pagePresent == -1) {
        return;
    }
    sprite = gs_work_alloc(20);
    mode = gs_work_alloc(8);
    setlen(sprite, 4);
    setcode(sprite, 0x64);
    sprite->clut = 0x7A80;
    sprite->r0 = 0x80;
    sprite->g0 = 0x80;
    sprite->b0 = 0x80;
    sprite->x0 = x;
    sprite->y0 = y;
    sprite->w = 80;
    sprite->h = 80;
    sprite->u0 = 0;
    sprite->v0 = 0xAF;
    addPrim(ot, sprite);
    setlen(mode, 1);
    mode->code[0] = 0xE100068F;
    addPrim(ot, mode);
}

s16 CalculatePortraitPageId(u16 code)
{
    return 0xFEF - code;
}
void ApplyPortraitPageDecode(s16 page)
{
    if (textBankId != page && textBankResult != 0) {
        DecodeContainer0DFromStart(textBankResult);
        farm_work_sector_release();
        textBankId = page;
        textBankReading = 0;
    }
}
void LoadPortraitPageFromMessege0(s16 page, s32 singleAttempt)
{
    s32 offset;

    if (page == textBankLoaded || page < 0) {
        return;
    }
    offset = page * 0x1800;
    if (singleAttempt == 0 || textBankReading == 0) {
        if (farm_work_sector_get() == 0) {
            SystemError(0x54, 0);
        }
        LoadAFileEntryRange(1, offset, 0x1800, storage8006C910, 1, 1);
        textBankReading = 1;
        textBankLoading = page;
    }
    do {
        textBankResult = LoadAFileEntryRange(1, offset, 0x1800, storage8006C910, 1, 0);
        if (textBankResult != 0) {
            textBankLoaded = textBankLoading;
            break;
        }
    } while (singleAttempt == 0);
}
void text_next_page(void)
{
    u16 *stream;
    u16 *found;
    s32 arg3;

    stream = textCurrent;
    arg3 = 1;
    if (textIndex != 0) {
        arg3 = textIndex;
    }

    found = text_find_nth(stream, 0xFFB, 1, arg3);
    if (found != 0) {
        textCurrent = found + 1;
        textScrollTimer = 0;
        textScrollY = 0;
        textScrollState = 3;
        textScrolling = 1;
        text_reset_state();
        textScrollStarted = 0;
        if (*textCurrent == 0xFFFE) {
            textCurrent++;
        }
    }
}
/* The table-driven sibling of text_next_page: same search-and-arm sequence,
 * but run over the three marker/id pairs in textPageBreaks / textPagePhases and
 * stopping at the first marker that is actually present in the stream.
 *
 * Two shapes here are retail's and worth keeping:
 *
 *   - the 0xFFFE skip runs BEFORE the block of global stores, not after it
 *     as in text_next_page, and it re-reads textCurrent afterwards rather than
 *     reusing `found`. Both are visible in the ordering of the sw/lw pair.
 *
 *   - the index is compared with sltiu, unsigned, so it is written as an
 *     unsigned loop counter. A signed int here changes the compare.
 *
 * PROBABLE: text_count_entries's return feeds textIndex, which is this function's
 * own `arg3` input on the next entry -- so the call is measuring something
 * about the arm it just selected and carrying it forward. */
void text_start_choice(void)
{
    u16 *found;
    u32 i;
    s32 arg3;

    arg3 = 1;
    if (textIndex != 0) {
        arg3 = textIndex;
    }

    for (i = 0; i < 3; i++) {
        found = text_find_nth(textCurrent, textPageBreaks[i], 1, arg3);
        if (found != 0) {
            textCurrent = found + 1;
            if (textCurrent[0] == 0xFFFE) {
                textCurrent = found + 2;
            }

            textScrollState = 3;
            textScrollTimer = 0;
            textScrollY = 0;
            textScrolling = 1;
            textScrollStarted = 0;
            textChoice = 0;
            textPhase = textPagePhases[i];
            textIndex = text_count_entries(textCurrent, 0xFFFF, 0);
            return;
        }
    }
}
/* Ported from hmbtn-decomp (CC0), text_insert_source. */
extern TextBankStarts mainUiTextTableRuntimeOffset;
extern u16 textNumberBuffer[4];
void text_expand_request(s32 mode, s32 unused);
u16 *text_write_number(s32 value, u16 *buffer, s32 length);
s16 text_find_staged_value(void);
enum TextInsert {
    TEXT_INSERT_PLAYER_NAME = 10,
    TEXT_INSERT_HORSE_NAME,
    TEXT_INSERT_DOG_NAME,
    TEXT_INSERT_FARM_NAME,
    TEXT_INSERT_FAMILY_NAME,
    TEXT_INSERT_FAMILY_MODE_NAME,
    TEXT_INSERT_STAGED_VALUE,
    TEXT_INSERT_BASKET_COUNT,
    TEXT_INSERT_FODDER,
    TEXT_INSERT_LUMBER,
    TEXT_INSERT_CHICKEN_FEED,
    TEXT_INSERT_FISH_FOOD,
    TEXT_INSERT_SLOT0_LIVESTOCK,
    TEXT_INSERT_SLOT1_LIVESTOCK,
    TEXT_INSERT_MINIGAME_SOURCE,
    TEXT_INSERT_EVENT_LIVESTOCK,
    TEXT_INSERT_EVENT_LIVESTOCK_ALT,
    TEXT_INSERT_LIST_LIVESTOCK_AGED,
    TEXT_INSERT_LIST_FLAGGED_LIVESTOCK_AGED,
    TEXT_INSERT_LIST_CHICKENS_AGED,
    TEXT_INSERT_LIST_LIVESTOCK_OTHER,
    TEXT_INSERT_LIST_FLAGGED_LIVESTOCK_OTHER,
    TEXT_INSERT_LIST_CHICKENS_OTHER,
    TEXT_INSERT_ANN_BIRTHDAY_DAY,
    TEXT_INSERT_HELD_CHICKEN
};
u16 *text_insert_source(u16 code)
{
    void *result;
    s32 i;
    s16 index;

    result = 0;
    switch (code & 0xFFF) {
    case TEXT_INSERT_PLAYER_NAME:
        result = gameStateView.player.name;
        break;
    case TEXT_INSERT_HORSE_NAME:
        result = gameStateView.animals.horse.actor.name;
        break;
    case TEXT_INSERT_DOG_NAME:
        result = gameStateView.animals.dog.actor.name;
        break;
    case TEXT_INSERT_FARM_NAME:
        result = gameStateView.progress.estate.farmName;
        break;
    case TEXT_INSERT_FAMILY_NAME:
        result = gameStateView.family.childName;
        break;
    case TEXT_INSERT_FAMILY_MODE_NAME:
        switch (gameStateView.family.addressMode) {
        case FAMILY_MODE_PLAYER_NAME:
            result = gameStateView.player.name;
            break;
        case FAMILY_MODE_HONEY:
            result = ((
                u16 *)(mainUiTextTableRuntimeOffsetView.names +
                       ((s32)(textBankView.names.honey) - (s32)((u32) & ((TextBank *)0)->names))));
            break;
        case FAMILY_MODE_DARLING:
            result = ((u16 *)(mainUiTextTableRuntimeOffsetView.names +
                              ((s32)(textBankView.names.darling) -
                               (s32)((u32) & ((TextBank *)0)->names))));
            break;
        default:
            result = 0;
            break;
        }
        break;
    case TEXT_INSERT_STAGED_VALUE:
        result = text_write_number(text_find_staged_value(), textNumberBufferView, 4);
        break;
    case TEXT_INSERT_BASKET_COUNT:
        result =
            text_write_number(gameStateView.progress.estate.basket.count, textNumberBufferView, 4);
        break;
    case TEXT_INSERT_FODDER:
        result = text_write_number(gameStateView.progress.estate.fodder, textNumberBufferView, 4);
        break;
    case TEXT_INSERT_LUMBER:
        result = text_write_number(gameStateView.progress.estate.lumber, textNumberBufferView, 4);
        break;
    case TEXT_INSERT_CHICKEN_FEED:
        result =
            text_write_number(gameStateView.progress.estate.chickenFeed, textNumberBufferView, 4);
        break;
    case TEXT_INSERT_FISH_FOOD:
        result = text_write_number(gameStateView.progress.estate.fishFood, textNumberBufferView, 4);
        break;
    case TEXT_INSERT_SLOT0_LIVESTOCK:
        index = gameStateView.animals.slots[0].animal;
        if (index != -1) {
            result = gameStateView.animals.livestock[index].actor.name;
        }
        break;
    case TEXT_INSERT_SLOT1_LIVESTOCK:
        index = gameStateView.animals.slots[1].animal;
        if (index != -1) {
            result = gameStateView.animals.livestock[index].actor.name;
        }
        break;
    case TEXT_INSERT_MINIGAME_SOURCE:
        result = gameStateView.minigames.textSource;
        break;
    case TEXT_INSERT_EVENT_LIVESTOCK:
    case TEXT_INSERT_EVENT_LIVESTOCK_ALT:
        for (i = 0; i < 20; i++) {
            if (gameStateView.animals.livestock[i].actor.flags & ACTOR_FLAG_IN_EVENT) {
                result = gameStateView.animals.livestock[i].actor.name;
            }
        }
        break;
    case TEXT_INSERT_ANN_BIRTHDAY_DAY:
        result = text_write_number(gameStateView.villagers[CHARACTER_ANN].birthday.day + 1,
                                   textNumberBufferView, 4);
        break;
    case TEXT_INSERT_LIST_LIVESTOCK_AGED:
        ((void (*)(s32))text_expand_request)(0);
        result = textListBufferView;
        break;
    case TEXT_INSERT_LIST_FLAGGED_LIVESTOCK_AGED:
        ((void (*)(s32))text_expand_request)(1);
        result = textListBufferView;
        break;
    case TEXT_INSERT_LIST_CHICKENS_AGED:
        ((void (*)(s32))text_expand_request)(2);
        result = textListBufferView;
        break;
    case TEXT_INSERT_LIST_LIVESTOCK_OTHER:
        ((void (*)(s32))text_expand_request)(3);
        result = textListBufferView;
        break;
    case TEXT_INSERT_LIST_FLAGGED_LIVESTOCK_OTHER:
        ((void (*)(s32))text_expand_request)(4);
        result = textListBufferView;
        break;
    case TEXT_INSERT_LIST_CHICKENS_OTHER:
        ((void (*)(s32))text_expand_request)(5);
        result = textListBufferView;
        break;
    case TEXT_INSERT_HELD_CHICKEN:
        for (i = 0; i < 10; i++) {
            if (gameStateView.animals.chickens[i].actor.flags & ACTOR_FLAG_HELD) {
                result = gameStateView.animals.chickens[i].actor.name;
            }
        }
        break;
    }
    /* Retail padding after the 25-entry text-insert jump table. */
    asm(".section .rodata\n.word 0\n.text");
    return result;
}

extern s32 (*textListGenerators[])(s32);
extern void *memmove(void *dst, const void *src, unsigned int n);
extern void *memcpy(void *dst, const void *src, unsigned int n);

void text_expand_request(s32 value, s32 unused)
{
    u16 *p;
    s32 len;
    s32 n;

    textListBuffer[0] = 0xFFFF;
    p = textCurrent + textIndex;
    len = textListGenerators[value % 3](value / 3);
    for (n = 0; p[n] != 0xFFFF; n++) {
    }
    memmove(p + len, p + 1, n * 2);
    memcpy(p, textListBuffer, len * 2);
    textListBuffer[0] = 0xFFFF;
}
u16 *text_append_name(u16 *dst, u32 *text, s32 select);

s32 text_list_livestock(s32 select)
{
    u16 *dst = textListBuffer;
    u8 *record = storage800749F4;
    s32 i;

    for (i = 0; i < 20; i++) {
        if (!(*(u32 *)(record + 0x34) & 0x10000)) {
            dst = text_append_name(dst, (u32 *)record, select);
        }
        record += 0xB8;
    }
    return dst - textListBuffer;
}
u16 *text_append_name(u16 *dst, u32 *text, s32 select);

s32 text_list_flagged_livestock(s32 select)
{
    u16 *dst = textListBuffer;
    u8 *record = storage800749F4;
    s32 i;

    for (i = 0; i < 20; i++) {
        if (*(u32 *)(record + 0x34) & 0x10000) {
            dst = text_append_name(dst, (u32 *)record, select);
        }
        record += 0xB8;
    }
    return dst - textListBuffer;
}
u16 *text_append_name(u16 *dst, u32 *text, s32 select);

s32 text_list_chickens(s32 select)
{
    u16 *dst = textListBuffer;
    u8 *record = storage800742EC;
    s32 i;

    for (i = 0; i < 10; i++) {
        dst = text_append_name(dst, (u32 *)record, select);
        record += 0xB4;
    }
    return dst - textListBuffer;
}
u16 *text_append_name(u16 *dst, u32 *text, s32 select)
{
    u32 flags = text[13];
    u32 enabled = flags & 0x100;
    s32 append;

    if (select != 0) {
        append = ((flags >> 19) ^ 1) & 1;
    } else {
        append = flags & 0x80000;
    }
    if (enabled != 0 && append != 0) {
        dst = text_list_append(text_list_append(dst, (u16 *)(text + 1)), textDiedSuffix);
    }
    return dst;
}
u16 *text_write_number(s32 value, u16 *dst, s32 digits)
{
    *text_format_number(value, dst, digits - 1) = 0xFFFF;
    if (*dst == 0xFFD) {
        do {
            dst++;
        } while (*dst == 0xFFD);
    }
    return dst;
}
u16 text_char_code(u8 c)
{
    u8 cls = storage8005E025[c];

    if (cls & 4) {
        return c + 0x10;
    }
    if ((cls & 3) != 0) {
        return c - 0x41;
    }
    switch (c) {
    case '!':
        return 0x63;
    case '?':
        return 0x62;
    case '-':
        return 0x69;
    case '(':
        return 0x6A;
    case ')':
        return 0x6B;
    case '\n':
        return 0xFFFE;
    default:
        return 0xFFD;
    }
}

u16 *text_format_number(int value, u16 *dst, int digits)
{
    u8 buf[10];
    int i;
    int neg;
    int acc;

    if (digits >= 10) {
        digits = 10;
    }
    neg = (u32)value >> 31;
    if (neg) {
        value = -value;
    }
    for (i = 0; i < digits; i++) {
        buf[i] = value % 10;
        value = value / 10;
    }
    if (neg) {
        buf[digits - 1] = 10;
    }
    acc = 0;
    for (i = digits - 1; i >= 0; i--) {
        if (buf[i] == 10) {
            *dst = 0x69;
        } else if (acc == 0 && buf[i] == 0 && i != 0) {
            *dst = 0xFFD;
        } else {
            acc += buf[i];
            *dst = buf[i] + 0x40;
        }
        dst++;
    }
    return dst;
}

u16 *text_write_season(int index, u16 *dst)
{
    u16 *src = &seasonTextCodes[index * 6];
    int i;

    for (i = 5; i >= 0; i--) {
        *dst++ = *src++;
    }
    return dst;
}

u16 *CopyWeekdayAbbreviation(int day, u16 *dst)
{
    u16 *src = gAwWeekdayAbbreviations[day];
    int i;

    for (i = 2; i >= 0; i--) {
        *dst++ = *src++;
    }
    return dst;
}
extern u16 textDigits[];

void text_draw_number(s16 value, s16 digits, void *display, s16 x, s16 y)
{
    s32 n;
    s32 i;
    u16 *p;

    if (digits != -1) {
        if (digits >= 10) {
            return;
        }
        n = digits;
    } else {
        n = 9;
    }
    *text_format_number(value, textDigits, n) = 0xFFFF;
    p = textDigits;
    if (digits == -1) {
        for (i = 0; i < n && *p == 0xFFD; i++) {
            p++;
        }
    }
    DrawTextWrapper(display, p, x, y);
}
void text_copy_string(u16 *dst, u16 *src)
{
    u16 c;

    do {
        c = *src++;
        *dst++ = c;
    } while (c != 0xFFFF);
}
void text_draw_in_frame(s32 frameSlot, void *display, const u16 *stream)
{
    s32 mode;

    mode = text_get_palette();
    text_set_palette(3);
    window_frame_draw_at(frameSlot, 0, 0x1A, 0xA4, 0x10B, 0x3E);
    DrawTextWrapper(display, stream, 0x1E, 0xA8);
    text_set_palette(mode);
}
void text_reset_bank_state(void)
{
    textFaceBank = -1;
    textBankLoaded = -1;
    textBankId = -3;
}
/* Portrait page preparation: initialize the portrait loader, derive the page
 * from the 16-bit portrait code, load it in normal mode, then apply the page
 * decode state. */
void text_select_bank(u16 code)
{
    s16 page;

    text_reset_bank_state();
    page = CalculatePortraitPageId(code);
    LoadPortraitPageFromMessege0(page, 0);
    ApplyPortraitPageDecode(page);
}
void text_draw_face_above(u32 *ot, u16 code, s16 x, s16 y)
{
    text_draw_face(ot, x - 4, y - 0x54, CalculatePortraitPageId(code));
}
s32 text_bank_ready(u16 code)
{
    s16 current = text_get_bank();
    s16 page = CalculatePortraitPageId(code);

    if (current != page) {
        LoadPortraitPageFromMessege0(page, 1);
    }
    if (textBankLoaded == page) {
        ApplyPortraitPageDecode(page);
        return 1;
    }
    return 0;
}
void text_draw_face_by_code(u32 *ot, u16 code, s16 x, s16 y)
{
    text_draw_face(ot, x, y, CalculatePortraitPageId(code));
}
typedef struct ItemSlot6 {
    s16 id;
    s16 unk02;
    s16 value;
} ItemSlot6;
extern ItemSlot6 storage80071A84[];
extern ItemSlot6 storage80071A90[];
extern ItemSlot6 storage80070EC2[];
extern s32 math_int_pow(s32 arg0, s32 arg1);

s16 text_find_staged_value(void)
{
    ItemSlot6 *list;
    s32 count;
    s32 k;
    s32 i;

    list = 0;
    count = 0;
    for (k = 0; k < 3; k++) {
        switch (k) {
        case 0:
            list = storage80071A84;
            count = 1;
            break;
        case 1:
            list = storage80071A90;
            count = math_int_pow(2, ((s16 *)list)[-57] + 1);
            break;
        case 2:
            list = storage80070EC2;
            count = 0x40;
            break;
        }
        for (i = 0; i < count; i++) {
            if (list[i].id < 0x1A) {
                if (list[i].id >= 0x15) {
                    return list[i].value;
                }
            }
        }
    }
    return 0;
}
