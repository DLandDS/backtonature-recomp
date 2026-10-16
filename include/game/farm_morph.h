#ifndef BTNX_GAME_FARM_TYPES_37_H
#define BTNX_GAME_FARM_TYPES_37_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "geometry/tmd_object.h"
#include "game/us_vector.h"

typedef struct FarmMorph {
    Model links[3];
    u16 x;
    u16 y;
    u16 z;
    u8 pad1E[0x20 - 0x1E];
    SVECTOR *from;
    SVECTOR *to;
    SVECTOR *out;
    u16 flags;
    s16 step;
    s16 steps;
    u8 pad32[0x34 - 0x32];
} FarmMorph;

typedef struct FarmSceneModel {
    Model model;
    SVECTOR position;
    USVector angle;
    USVector spin;
    USVector turn;
    u16 flags;
    u8 pad2A[0x2C - 0x2A];
    FarmMorph *morph;
} FarmSceneModel;

#endif
