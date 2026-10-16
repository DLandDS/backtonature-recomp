#ifndef BTNX_GAME_FARM_TYPES_18_H
#define BTNX_GAME_FARM_TYPES_18_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum FishFoodAddMode {
    FISH_FOOD_ADD_ONE = 0,
    FISH_FOOD_ADD_AMOUNT = 1,
};

typedef struct FarmResultRow {
    s32 words[3];
    s32 key;
} FarmResultRow;

typedef union FarmEventFlags {
    s32 word;
    u8 bytes[4];
} FarmEventFlags;

#endif
