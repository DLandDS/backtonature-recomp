#ifndef BTNX_GAME_FARM_TYPES_13_H
#define BTNX_GAME_FARM_TYPES_13_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "graphics/anim_file.h"
#include "game/farm_actor.h"

typedef struct FarmLivestock {
    FarmActor actor;
    u16 grazeCount;
    u16 daysSick;
    u16 growthDays;
    u16 pregnancy;
    u16 daysSinceShearing;
    s16 helperNumber;
} FarmLivestock;

typedef struct FarmPet {
    FarmActor actor;
    s16 training;
    u8 padAE[0xE0 - 0xAE];
    VramPlace projectCoords[2];
} FarmPet;

#endif
