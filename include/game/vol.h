#ifndef BTNX_GAME_SHARED_TYPES_41_H
#define BTNX_GAME_SHARED_TYPES_41_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct Vol {
    s16 seqHandle;
    s16 pad;
} Vol;

enum FireworkSound {
    SOUND_FIREWORK_LAUNCH = 0x46,
    SOUND_FIREWORK_BURST = 0x47,
};

#endif
