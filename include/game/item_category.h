#ifndef BTNX_GAME_FARM_TYPES_10_H
#define BTNX_GAME_FARM_TYPES_10_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/flag_word.h"

enum ItemCategory {
    ITEM_CATEGORY_CROP = 0,
    ITEM_CATEGORY_TOOL = 1,
    ITEM_CATEGORY_SEED = 2,
    ITEM_CATEGORY_CONSUMABLE = 3,
    ITEM_CATEGORY_OBJECT = 4,
};

enum ItemInfoFlag {
    ITEM_FLAG_BIT4 = 0x10,
    ITEM_FLAG_SEED = 0x40,
    ITEM_FLAG_BIT8 = 0x100,
    ITEM_FLAG_TOOL = 0x4000,
    ITEM_FLAG_PICKUP = 0x8000,
};

typedef struct ItemInfo {
    u16 itemFlags;
    u8 category;
    s8 crop;
    s16 tileRule[5];
    s16 picture;
    s16 buyPrice;
    s16 sellPrice;
    FlagWord targets;
    FlagWord classFlags;
    ItemUseFn use;
    s8 stamina;
    s8 fatigue;
    s16 plantAnimation;
    s16 halfWidth;
    s16 halfDepth;
} ItemInfo;

#endif
