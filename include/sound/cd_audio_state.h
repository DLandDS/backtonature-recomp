#ifndef BTNX_SOUND_SOUND_TYPES_4_H
#define BTNX_SOUND_SOUND_TYPES_4_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum CdAudioState {
    CD_AUDIO_IDLE = 0,
    CD_AUDIO_REQUESTED = 1,
    CD_AUDIO_FADING = 2,
};

enum CdAudioTrack {
    CD_TRACK_COW_PRAISE = 0x09,
    CD_TRACK_COW_NEGLECT = 0x0A,
    CD_TRACK_SHEEP_PRAISE = 0x0B,
    CD_TRACK_SHEEP_NEGLECT = 0x0C,
    CD_TRACK_HORSE_PRAISE = 0x0D,
    CD_TRACK_HORSE_NEGLECT = 0x0E,
    CD_TRACK_CHICKEN_NOTICE = 0x13,
    CD_TRACK_VISITORS = 0x14,
    CD_TRACK_CHIME = 0x15,
    CD_TRACK_COOKING_RESULT = 0x16,
    CD_TRACK_PHOTO_JINGLE = 0x19,
};

enum SoundRequest {
    SOUND_REQUEST_RESET = 0,
    SOUND_REQUEST_SEQ_2 = 999,
    SOUND_REQUEST_SEQ_1 = 1000,
};

enum SoundSong {
    SONG_TITLE = 0x1A,
};

#endif
