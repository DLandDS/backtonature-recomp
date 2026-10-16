/* PROVISIONAL translation-unit boundary.
 * Range: 0x80017A38..0x8001AF34.
 * Basis: Dialog rendering/parsing, portrait loading, text formatting, and calendar text; correlated
 * with three low-rodata jump tables. Keep the boundary provisional until rodata/relocation evidence
 * proves it. */
#include "common.h"
#include "include_asm.h"
#include "game_types.h"

/* TENTATIVE: ASCII character-class table; low bits select letter classes. */
extern u8 storage8005E025[];
extern s16 textFaceBank;
extern s32 textBankReading;
extern s16 textBankLoaded;
extern s16 textBankId;
extern s32 textBankLoading;
/* PROBABLE: decoded portrait container retained between portrait changes. */
extern void *textBankResult;
/* TENTATIVE: portrait-page destination buffer, outside the gp window. */
extern u8 storage8006C910[];
/* PROBABLE: fallback expansion streams indexed by low 12 glyph-code bits. */
extern u16 *textSources[];
/* Text suffix {0x0051, 0xFFFE, 0xFFFF}; trailing 0x0000 is padding. */
extern u16 textDiedSuffix[];
extern u16 seasonTextCodes[];
/* CONFIRMED from asm/data/39314.data.s: two parallel u16[4] tables, three
 * live entries and a zero terminator. textPageBreaks holds control markers
 * {0x0FFA, 0x0FF9, 0x0FF8} and textPagePhases the ids {7, 8, 9} that pair with
 * them. text_next_page handles marker 0x0FFB the same way but singly, so
 * these are the same family one step down. The `< 3` bound in text_start_choice
 * is the live length, not the allocation. */
extern u16 textPageBreaks[];
extern u16 textPagePhases[];
/* PROBABLE: dialog page-advance state. text_begin_typing arms it, text_advance_glyph
 * consumes states 1 and 3 and bumps the counter, text_reset_state clears both. */
extern u16 textIndex;
extern u16 textPhase;
/* TENTATIVE: current text speed or window style, selected from textSpeedIntervals. */
extern s16 textGlyphInterval;
/* TENTATIVE: three-entry lookup indexed by the selector passed to
 * text_set_speed; out-of-range selectors fall back to entry 1. */
extern u16 textSpeedIntervals[];
/* TENTATIVE: four-entry word table selected by a 1-based dialog kind, with
 * out-of-range kinds falling back to entry 0. */
extern u32 textChoiceFaceBanks[];
/* PROBABLE: the dialog stream cursor and the page state that travels with it.
 * Widths are taken from the load/store widths at every reference site. */
extern u16 *textCurrent;
extern s16 textScrollState;
extern s16 textScrollY;
/* CONFIRMED s16 from the `sh` at 0x8001A238. text_next_page does not clear
 * this one, so it belongs to the table-driven arm in text_start_choice only. */
extern s16 textChoice;
extern s32 textScrollTimer;
extern s32 textScrolling;
extern s32 textScrollStarted;
extern s16 textSpacingY;
extern s16 textPenY;
extern s16 textSpacingX;
extern s32 textInput;
extern s32 textPalette;
extern s16 textWaitFrames;
extern u16 textLastGlyph;
extern s16 textOffsetX;
extern s16 textOffsetY;
extern s16 textOriginX;
extern s16 textOriginY;
extern s32 storage8005E4B0;

/* PROBABLE: dialog navigation input state. Only the two sampled flag words
 * are known here: 0x4000/0x1000 select cyclic movement and 0x40 accepts. */
typedef struct {
    u32 flags00;
    u32 unk04;
    u32 flags08;
} DialogNavigationInput;

/* PROBABLE: the text-render context owns the display-list pointer at +0xD8. */
typedef struct {
    u8 unk00[0xD8];
    void *display;
} DialogRenderContext;

/* These are arrays solely to keep cc1 from treating their pointer-sized first
 * entries as small data. Retail reaches both through %hi/%lo, not $gp. */
extern DialogNavigationInput *inputPad1;  /* nongp */
extern DialogRenderContext *currentFrame; /* nongp */

/* Defined below by INCLUDE_ASM; declared so the C above can call them. */
/* TENTATIVE: temporary display-list storage acquired and released around the
 * text parser. Exact ownership semantics await the boot-unit callees. */
void *gs_work_alloc(s32 size);
void gs_work_release(void *ptr);
u8 *ParseDialogTextStream(u_long *display, u8 *scratch, u16 *stream, s16 x, s16 y, s32 scale,
                          s32 mode, s32 aux, s32 recursive, s32 lineReset);
u16 *text_find_nth(u16 *stream, u16 marker, s32 occurrence, s32 maxWidth);
u16 text_count_entries(u16 *stream, u16 marker, s16 maxWidth);
/* TENTATIVE: CLUT Y-coordinate table indexed by a clamped tpage/clut row
 * selector. Addressed with %hi/%lo in text_glyph_sprite, so it is not a complete
 * type here -- keep it sizeless or the object flips to %gp_rel addressing. */
extern u16 textClutRows[];
void text_reset_state(void);
void DecodeContainer0DFromStart(void *container);
void farm_work_sector_release(void);
void *farm_work_sector_get(void);
void *LoadAFileEntryRange(s32 entry, s32 offset, s32 size, void *destination, s32 arg4, s32 arg5);
void SystemError(s32 code, s32 detail);
u16 *text_insert_source(u16 code);
/* TENTATIVE: control-code expansion streams selected by text_insert_source. */
extern u16 storage80071A00[];
extern u16 storage80075AD8[];
extern u16 storage800759E8[];
extern u16 storage80070D18[];
extern u16 storage8007AFBC[];
extern u16 textBank[];
extern u16 storage80051768[];
extern s32 storage80054BC0;
extern u16 textNumberBuffer[4];
extern u16 textListBuffer[];
extern u8 storage800742EC[];
extern u8 storage800749F4[];
extern u16 storage80070D38;
extern u16 storage80070D3A;
extern u16 storage80070D3E;
extern u16 storage80070D40;
extern u16 storage800712C0;
extern u8 gameState[];
extern u8 storage800749F8[];
u16 *text_write_number(s32 value, u16 *dst, s32 digits);
s16 text_find_staged_value(void);
void text_expand_request(s32 value, s32 unused);
s32 sound_play(s16 sound);
void text_draw_cursor(void *display, s16 x, s16 y);
void window_frame_draw_at(s32 arg0, s32 unused, s16 arg2, s16 arg3, s16 arg4, s16 arg5);
u16 *text_format_number(s32 value, u16 *dst, s32 digits);
void ui_draw_icon_0b_1(void *display, s16 x, s16 y);
void *text_draw_lines_centered(void *display, void *scratch, const u16 *stream, s16 x, s16 y,
                               s16 width, s32 scale, s16 arg7, s16 arg8);
void *text_draw_lines_right(void *display, void *scratch, const u16 *stream, s16 x, s16 y,
                            s16 width, s32 scale, s16 arg7, s16 arg8);
void *text_draw_line_centered(void *display, s32 arg1, const u16 *stream, s16 x, s16 y, s16 width,
                              s32 scale, s16 font);
void *text_draw_line_right(void *display, s32 arg1, const u16 *stream, s16 x, s16 y, s16 width,
                           s32 scale, s16 font);
/* CONFIRMED: seven three-glyph weekday abbreviations at 0x800490BC. */
extern u16 gAwWeekdayAbbreviations[7][3];

/* Typed views preserve the original declaration and compiler behavior. */
#define currentFrameView                 (*(FrameState * (*)) & currentFrame)
#define farmAreaView                     (*(FarmLoadArea(*)) & farmArea)
#define gameStateView                    (*(FarmState(*)) & gameState)
#define inputPad1View                    (*(InputRecord * (*)) & inputPad1)
#define mainUiTextTableRuntimeOffsetView (*(TextBankStarts(*)) & mainUiTextTableRuntimeOffset)
#define storage8005E4B0View              (*(u16 * (*)) & storage8005E4B0)
#define textBankReadingView              (*(s32(*)) & textBankReading)
#define textBankResultView               (*(u8 * (*)) & textBankResult)
#define textBankView                     (*(TextBank(*)) & textBank)
#define textBlipToggleView               (*(s32(*)) & textBlipToggle)
#define textChoiceView                   (*(s16(*)) & textChoice)
#define textCurrentView                  (*(u16 * (*)) & textCurrent)
#define textCurrentViewValue (*(*(u16 *(*))&textCurrent)
#define textFaceBankView      (*(s16(*)) & textFaceBank)
#define textFaceReloadView    (*(s32(*)) & textFaceReload)
#define textFaceShownView     (*(s32(*)) & textFaceShown)
#define textFmtDecView        (*(char (*)[]) & textFmtDec)
#define textFmtHexView        (*(char (*)[]) & textFmtHex)
#define textFmtIntView        (*(char (*)[]) & textFmtInt)
#define textGlyphIntervalView (*(s16(*)) & textGlyphInterval)
#define textGlyphTimerView    (*(s32(*)) & textGlyphTimer)
#define textGlyphsView        (*(u16(*)[64]) & textGlyphs)
#define textIndexView         (*(u16(*)) & textIndex)
#define textInputView         (*(s32(*)) & textInput)
#define textLastCodeView      (*(u16(*)) & textLastCode)
#define textLastGlyphView     (*(u16(*)) & textLastGlyph)
#define textLineGlyphsView    (*(s16(*)) & textLineGlyphs)
#define textListBufferView    (*(u16(*)[224]) & textListBuffer)
#define textMarginXView       (*(s16(*)) & textMarginX)
#define textNoScrollView      (*(s32(*)) & textNoScroll)
#define textNumberBufferView  (*(u16(*)[4]) & textNumberBuffer)
#define textOffsetXView       (*(s16(*)) & textOffsetX)
#define textOffsetYView       (*(s16(*)) & textOffsetY)
#define textOriginXView       (*(s16(*)) & textOriginX)
#define textOriginYView       (*(s16(*)) & textOriginY)
#define textPaletteView       (*(s32(*)) & textPalette)
#define textPenXView          (*(s16(*)) & textPenX)
#define textPenYView          (*(s16(*)) & textPenY)
#define textPhaseView         (*(u16(*)) & textPhase)
#define textScrollHoldView    (*(s32(*)) & textScrollHold)
#define textScrollStartedView (*(s32(*)) & textScrollStarted)
#define textScrollStateView   (*(s16(*)) & textScrollState)
#define textScrollTimerView   (*(s32(*)) & textScrollTimer)
#define textScrollYView       (*(s16(*)) & textScrollY)
#define textScrollingView     (*(s32(*)) & textScrolling)
#define textSourcesView       (*(u16 * (*)[12]) & textSources)
#define textSpacingXView      (*(s16(*)) & textSpacingX)
#define textSpacingYView      (*(u16(*)) & textSpacingY)
#define textStateView         (*(u16(*)) & textState)
#define textWaitFramesView    (*(s16(*)) & textWaitFrames)
#define textWrapIndexView     (*(s16(*)) & textWrapIndex)
