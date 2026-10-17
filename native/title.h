#ifndef BTN_NATIVE_TITLE_H
#define BTN_NATIVE_TITLE_H

#include "animation.h"
#include "audio.h"
#include "image_bank.h"
#include "title_setup.h"
#include "title_dialog.h"
#include "title_speech.h"

#define BTN_TITLE_HZ 30u /* main_run_frame uses VSync(2); nominal NTSC rate. */
typedef enum {
    BTN_TITLE_PROMPT, BTN_TITLE_MENU, BTN_TITLE_UNAVAILABLE, BTN_TITLE_INTRO,
    BTN_TITLE_SETUP, BTN_TITLE_READY, BTN_TITLE_ATTRACT
} BtnTitlePage;
typedef struct {
    BtnTitlePage page;
    unsigned selection, ticks, page_ticks;
    unsigned fade_mode, fade_level, fade_length, intro_stage, intro_ticks;
    int logo_y, copyright_y, start_x;
    BtnTitlePage target;
    bool pending;
} BtnTitleState;

typedef struct {
    BtnTitleState state;
    BtnGraphics *assets, *graphics;
    BtnDrawList *list;
    BtnAnimation animation, ui_animation;
    uint8_t *animation_data, *image_data, *music_data, *sequence_data, *effects_data, *font_data;
    uint8_t *ui_animation_data, *ui_image_data;
    uint8_t *speech_image_data;
    BtnAudioBank music_bank, effects_bank;
    BtnAudio music, effects;
    BtnSequence sequence;
    uint16_t effect_definitions[6][7];
    BtnNameLayout name_layout;
    uint16_t name_captions[2][128];
    uint16_t yes_no[16], setup_prompts[2][64];
    uint16_t setup_questions[4][16];
    uint16_t birthday_caption[14], summary_format[BTN_SETUP_SUMMARY_FORMAT_WORDS], review_options[64];
    unsigned text_clut_y;
    BtnTitleSetup setup;
    BtnTitleDialog dialog;
    BtnSpeechMessage speech_messages[BTN_SPEECH_MESSAGES];
    BtnTitleSpeech speech;
    BtnAnimationCursor speech_wait;
    unsigned speech_interval;
    BtnAnimationCursor actors[6], actor_frames[14];
    unsigned actor_ticks[6], background[2], background_frame[2];
    uint16_t wave_angles[6];
    uint32_t random;
} BtnTitle;

/* CPU-native title and new-game setup; never an overlay interpreter.
 * Scene state resets independently of resource loading. */
void btn_title_reset(BtnTitleState *state);
/* SDK ccos/csin integer path for nonnegative angles, wrapping at 4096. The
 * caller supplies six validated CORDIC steps from the executable. */
int btn_title_wave(const uint16_t steps[6], unsigned angle, bool sine);
/* Nine 320x240 tile anchors. The second layer reverses both grid directions. */
void btn_title_tile_positions(unsigned phase, int sway, bool second_layer, int xy[9][2]);
int btn_title_step(BtnTitleState *state, const BtnInput *input);
const char *btn_title_notice(const BtnTitleState *state);
/* Zero-initialize; close also cleans up a failed open. Archive stays caller-owned. */
bool btn_title_open(BtnTitle *title, BtnArchive *archive, const char *directory);
void btn_title_close(BtnTitle *title);
bool btn_title_tick(BtnTitle *title, const BtnInput *input);
bool btn_title_draw(BtnTitle *title, uint32_t *pixels, size_t count);
bool btn_title_audio(BtnTitle *title, int16_t *stereo, size_t frames);
/* Available only after setup confirmation and its transition have completed.
 * This typed handoff does not claim that a farm/gameplay scene is implemented. */
const BtnNewGame *btn_title_new_game(const BtnTitle *title);

#endif
