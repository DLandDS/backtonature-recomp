#ifndef BTN_NATIVE_TITLE_SPEECH_H
#define BTN_NATIVE_TITLE_SPEECH_H
#include "title_dialog.h"

enum { BTN_SPEECH_MESSAGES=5, BTN_SPEECH_WORDS=128, BTN_SPEECH_WAIT_RANGE=7 };
typedef struct { uint16_t words[BTN_SPEECH_WORDS]; unsigned count; } BtnSpeechMessage;
typedef struct {
    unsigned message, start, end, shown, elapsed;
    bool active, closing, waiting, started, blip;
} BtnTitleSpeech;

/* ponytail: five audited mayor messages, one portrait and explicit page breaks;
 * extend the parser only when another title path needs more controls.
 * Unsupported commands fail instead of disappearing silently. */
bool btn_speech_message(BtnSpeechMessage *out, const uint16_t *words, size_t count, unsigned portrait);
void btn_speech_begin(BtnTitleSpeech *speech, unsigned message, const BtnSpeechMessage *text);
/* Each simulation call represents VSync(2). Presses at a wait advance a page;
 * holding A/B while printing selects speed 0, one stream word per call. */
int btn_speech_step(BtnTitleSpeech *speech, const BtnSpeechMessage *text,
    const BtnInput *input, unsigned interval);
bool btn_speech_face(const BtnTitleSpeech *speech, const BtnSpeechMessage *text);
size_t btn_speech_visible(const BtnTitleSpeech *speech, const BtnSpeechMessage *text,
    uint16_t out[BTN_SPEECH_WORDS]);
#endif
