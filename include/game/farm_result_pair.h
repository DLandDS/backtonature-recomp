#ifndef BTNX_GAME_FARM_TYPES_33_H
#define BTNX_GAME_FARM_TYPES_33_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

struct FarmResultPair;

typedef struct FarmImageSlot {
    u16 id;
    s16 duration;
    s16 x;
    s16 y;
    u16 next;
    u16 timer;
    u16 count;
    RECT image;
} FarmImageSlot;

typedef struct FarmBlitFrame {
    u16 id;
    s16 duration;
    u16 next;
    s16 x;
    s16 y;
    RECT image;
} FarmBlitFrame;

#endif
