#ifndef BTNX_GAME_FARM_TYPES_26_H
#define BTNX_GAME_FARM_TYPES_26_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct FarmSettings {
    s16 messageType;
    s16 vibration;
    s16 stereo;
    s16 statusLatch;
    s16 unused8;
    s16 unusedA;
} FarmSettings;

#endif
