#ifndef BTNX_GAME_SHARED_TYPES_8_H
#define BTNX_GAME_SHARED_TYPES_8_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef union FlagWord {
    u16 half[2];
    s32 word;
} FlagWord;

typedef s32 (*ItemUseFn)(s8 stamina, s8 fatigue, s16 happiness, s32 unused);

#endif
