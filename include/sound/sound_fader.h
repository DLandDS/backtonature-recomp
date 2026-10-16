#ifndef BTNX_SOUND_SOUND_TYPES_2_H
#define BTNX_SOUND_SOUND_TYPES_2_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "sound/sound_fader_mode.h"

typedef struct SoundFader {
    s32 unused0;
    enum SoundFaderMode mode;
    s32 volume;
    s32 step;
    s32 maxVolume;
    s32 ready;
} SoundFader;

typedef struct SoundSequence {
    u16 unused0;
    u16 icon;
    u16 day;
} SoundSequence;

#endif
