#ifndef BTNX_GAME_FARM_TYPES_31_H
#define BTNX_GAME_FARM_TYPES_31_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct FarmThrownItem {
    s16 blinking;
    s16 kind;
    s16 timer;
    u16 phase;
    u16 dir;
    u8 pad0A[2];
    s32 velocityY;
    s32 groundY;
    s32 vx;
    s32 vy;
    s32 vz;
    u8 pad20[4];
    VECTOR position;
} FarmThrownItem;

#endif
