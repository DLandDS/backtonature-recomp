#ifndef BTNX_GAME_FARM_TYPES_25_H
#define BTNX_GAME_FARM_TYPES_25_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "graphics/anim_file.h"
#include "game/item.h"
#include "game/farm_actor.h"
#include "game/farm_fish.h"
#include "game/field_cell_look.h"

typedef struct FarmHelper {
    FarmActor actor;
    u16 number;
    u16 harvestSkill;
    u16 waterSkill;
    u16 animalSkill;
    u16 jobDays;
    ItemId item;
    u16 flags;
    s16 targetAnimal;
    s16 idleCount;
    s16 savedTaskNo;
    u16 savedTaskId;
    u16 savedTaskTime;
    FarmInitPair pairC4;
    AnimState itemSprite;
} FarmHelper;

#endif
