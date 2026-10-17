#ifndef BTN_NATIVE_AUDIO_H
#define BTN_NATIVE_AUDIO_H

#include "runtime.h"

#define BTN_AUDIO_RATE 44100u
#define BTN_AUDIO_VOICES 24
#define BTN_XA_MAX_FRAMES 4032u

/* All views borrow caller-owned bytes. Keep them alive until playback stops. */
typedef struct { int old, older; } BtnAdpcm;
typedef struct {
    const uint8_t *data;
    size_t size, body;
    unsigned programs, samples;
    int tone_slot[128];
    uint32_t sample_offset[256], sample_size[256];
    bool reverb;
} BtnAudioBank;

typedef struct {
    const uint8_t *data;
    size_t size, block, loop;
    BtnAdpcm decoder;
    int16_t pcm[28];
    unsigned cursor, phase, step, stage, level, counter;
    uint16_t adsr1, adsr2;
    float left, right;
    double pitch;
    int bend;
    uint64_t stamp;
    unsigned priority, channel, note;
    bool active, repeating, ended, raw;
} BtnAudioVoice;
typedef struct { unsigned program, volume, pan, nrpn; int bend; } BtnAudioChannel;
typedef struct {
    BtnAudioVoice voices[BTN_AUDIO_VOICES];
    BtnAudioChannel channels[16];
    uint64_t stamp;
    float gain, fade_start;
    uint32_t fade_left, fade_total;
    unsigned stolen, peak_voices;
} BtnAudio;
typedef struct {
    const uint8_t *data;
    size_t size, cursor, loop_cursor;
    uint32_t tempo, resolution;
    uint64_t wait, fraction;
    unsigned running, loop_running, repeats;
    bool playing, repeat;
} BtnSequence;

bool btn_adpcm_block(BtnAdpcm *state, const uint8_t block[16], int16_t pcm[28]);
bool btn_audio_bank_open(BtnAudioBank *bank, const void *bytes, size_t size);
void btn_audio_init(BtnAudio *audio);
/* Sample numbers are 1-based, matching VAB tone VAG numbers. */
bool btn_audio_sample_play(BtnAudio *audio, const BtnAudioBank *bank,
    unsigned sample, bool loop);
bool btn_audio_note_on(BtnAudio *audio, const BtnAudioBank *bank,
    unsigned channel, unsigned note, unsigned velocity);
/* UI sound definitions select one tone explicitly, unlike a sequence note. */
bool btn_audio_tone_on(BtnAudio *audio, const BtnAudioBank *bank,
    unsigned program, unsigned tone, unsigned note, unsigned fine, unsigned velocity);
void btn_audio_note_off(BtnAudio *audio, unsigned channel, unsigned note);
void btn_audio_stop(BtnAudio *audio);
bool btn_audio_volume(BtnAudio *audio, float gain, uint32_t fade_frames);
bool btn_audio_render(BtnAudio *audio, int16_t *stereo, size_t frames);
bool btn_sequence_open(BtnSequence *sequence, const void *bytes, size_t size, bool repeat);
bool btn_sequence_render(BtnSequence *sequence, BtnAudio *audio,
    const BtnAudioBank *bank, int16_t *stereo, size_t frames);

typedef struct {
    BtnAdpcm channels[2];
    unsigned rate, channel_count;
} BtnXaDecoder;
typedef struct {
    unsigned file, channel, submode, coding, rate, channels;
    const uint8_t *payload;
} BtnXaSector;
/* Accept complete 2352-byte Mode 2 sectors or 2336-byte sectors starting at
 * the duplicated subheader. A 2048-byte extraction cannot recover XA audio.
 * EDC is not checked. Emphasis, 8-bit and damaged subheaders are rejected.
 */
bool btn_xa_sector(BtnXaSector *sector, const void *bytes, size_t size);
bool btn_xa_decode(BtnXaDecoder *decoder, const BtnXaSector *sector,
    int16_t *stereo, size_t capacity_frames, size_t *frames);

#endif
