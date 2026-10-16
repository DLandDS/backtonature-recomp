#ifndef BTNX_SOUND_SOUND_TYPES_3_H
#define BTNX_SOUND_SOUND_TYPES_3_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct {
    s16 vabId;
    s16 category;
    s16 prog;
    s16 tone;
    s16 note;
    s16 fine;
    s16 vol;
} SoundDef;

#endif
