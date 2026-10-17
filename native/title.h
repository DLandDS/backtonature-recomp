#ifndef BTN_NATIVE_TITLE_H
#define BTN_NATIVE_TITLE_H

#include "animation.h"
#include "audio.h"
#include "image_bank.h"

#define BTN_TITLE_HZ 30u /* main_run_frame uses VSync(2); nominal NTSC rate. */
typedef enum { BTN_TITLE_PROMPT, BTN_TITLE_MENU, BTN_TITLE_UNAVAILABLE } BtnTitlePage;
typedef struct {
    BtnTitlePage page;
    unsigned selection, ticks, page_ticks;
} BtnTitleState;

typedef struct {
    BtnTitleState state;
    BtnGraphics *assets, *graphics;
    BtnDrawList *list;
    BtnAnimation animation;
    uint8_t *animation_data, *image_data, *music_data, *sequence_data, *effects_data;
    BtnAudioBank music_bank, effects_bank;
    BtnAudio music, effects;
    BtnSequence sequence;
    uint16_t effect_definitions[2][7];
} BtnTitle;

/* CPU-native reconstruction of the audited Start/menu branch, not an overlay
 * interpreter. Intro, scenery motion and new-game setup are not ported yet.
 * Scene state resets independently of resource loading. */
void btn_title_reset(BtnTitleState *state);
int btn_title_step(BtnTitleState *state, const BtnInput *input);
const char *btn_title_notice(const BtnTitleState *state);
/* Zero-initialize; close also cleans up a failed open. Archive stays caller-owned. */
bool btn_title_open(BtnTitle *title, BtnArchive *archive, const char *directory);
void btn_title_close(BtnTitle *title);
bool btn_title_tick(BtnTitle *title, const BtnInput *input);
bool btn_title_draw(BtnTitle *title, uint32_t *pixels, size_t count);
bool btn_title_audio(BtnTitle *title, int16_t *stereo, size_t frames);

#endif
