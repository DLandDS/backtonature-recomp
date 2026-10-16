#ifndef BTNX_GAME_SHARED_TYPES_5_H
#define BTNX_GAME_SHARED_TYPES_5_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct Vec3 {
    s32 x;
    s32 y;
    s32 z;
} Vec3;

typedef struct LimitPair {
    s16 value;
    s16 location;
} LimitPair;

typedef void (*Func)(void);

#endif
