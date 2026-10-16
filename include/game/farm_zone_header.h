#ifndef BTNX_GAME_FARM_TYPES_34_H
#define BTNX_GAME_FARM_TYPES_34_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/vec3.h"

typedef struct FarmZoneHeader {
    LimitPair place;
    s32 scene;
    u16 flags;
    s16 resourceSet;
    s16 resourceId;
    s16 sceneId;
} FarmZoneHeader;

#endif
