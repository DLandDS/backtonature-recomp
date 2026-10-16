#ifndef BTNX_GAME_FARM_TYPES_35_H
#define BTNX_GAME_FARM_TYPES_35_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/us_vector.h"

typedef struct FarmZoneModel {
    s32 flags;
    s32 name;
    union {
        s32 frames[2];
        s16 twin;
    } link;
    u16 steps;
    u8 pad12[0x14 - 0x12];
    SVECTOR position;
    USVector angle;
    USVector spin;
} FarmZoneModel;

#endif
