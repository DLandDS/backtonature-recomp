#ifndef BTNX_GAME_FARM_TYPES_7_H
#define BTNX_GAME_FARM_TYPES_7_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "graphics/anim_file.h"
#include "game/farm_date.h"
#include "game/vec3.h"
#include "game/item.h"
#include "game/item_cue.h"

typedef struct FarmActor {
    ItemId kind;
    u16 targetIndex;
    u16 name[9];
    s16 affection;
    s16 waitTimer;
    FarmDate date;
    u16 facing;
    u16 action;
    s16 taskNo;
    s16 taskId;
    s16 taskTime;
    s16 interruptTime;
    LimitPair place;
    u16 unused32;
    u32 flags;
    VECTOR position;
    VECTOR speed;
    VECTOR destination;
    VECTOR target;
    AnimState sprite;
    ItemCue cue;
} FarmActor;

#endif
