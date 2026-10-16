#ifndef BTNX_GAME_FARM_TYPES_28_H
#define BTNX_GAME_FARM_TYPES_28_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct FarmCellTexture {
    u8 uv[8];
    u16 clut;
} FarmCellTexture;

enum ItemReaction {
    ITEM_REACTION_BEST = 0,
    ITEM_REACTION_GOOD = 1,
    ITEM_REACTION_OK = 2,
    ITEM_REACTION_BAD = 3,
    ITEM_REACTION_WORST = 4,
    ITEM_REACTION_QUIET = 5,
    ITEM_REACTION_UPSET = 6,
};

#endif
