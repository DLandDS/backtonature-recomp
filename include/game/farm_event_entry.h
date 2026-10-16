#ifndef BTNX_GAME_FARM_TYPES_19_H
#define BTNX_GAME_FARM_TYPES_19_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/fish_food_add_mode.h"

typedef struct FarmEventEntry {
    u16 id;
    u8 varId;
    u8 varElement;
    u16 rangeLow;
    u16 rangeHigh;
    s32 window;
    u8 roleCondition;
    u8 bondVillager;
    u16 bondAmount;
    s32 needFlags;
    s32 needClear;
    s32 prerequisites;
    FarmEventFlags triggerFlags;
    u8 pad20;
    u8 availability;
    u8 triggerMode;
    u8 triggerValue;
} FarmEventEntry;

typedef struct FarmScriptStep {
    u8 type;
    u8 nextDone;
    u8 nextOther;
    u8 flags;
    s16 command;
    s16 operand;
    s32 x;
    s32 y;
} FarmScriptStep;

#endif
