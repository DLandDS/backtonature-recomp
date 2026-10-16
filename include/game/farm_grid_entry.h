#ifndef BTNX_GAME_FARM_TYPES_27_H
#define BTNX_GAME_FARM_TYPES_27_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct FarmGridEntry {
    s16 x, y;
    s16 x2, y2;
    s16 w, h;
    s16 w2, h2;
    s16 offset[2][2];
    s16 clutX, clutY;
    s16 hidden;
} FarmGridEntry;

typedef struct FarmPictureSlot {
    FarmGridEntry picture;
    u8 used;
    u8 pad1F;
    u16 id;
} FarmPictureSlot;

#endif
