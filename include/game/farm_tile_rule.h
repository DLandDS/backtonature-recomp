#ifndef BTNX_GAME_FARM_TYPES_9_H
#define BTNX_GAME_FARM_TYPES_9_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/item.h"

typedef struct FarmTileRule {
    u16 minPower;
    u16 minReach;
    u16 status;
    u16 usableBy;
} FarmTileRule;

enum Crop {
    CROP_NONE = -1,
    CROP_TURNIP,
    CROP_POTATO,
    CROP_CUCUMBER,
    CROP_STRAWBERRY,
    CROP_CABBAGE,
    CROP_TOMATO,
    CROP_CORN,
    CROP_ONION,
    CROP_PUMPKIN,
    CROP_PINEAPPLE,
    CROP_EGGPLANT,
    CROP_CARROT,
    CROP_SWEET_POTATO,
    CROP_SPINACH,
    CROP_GREEN_PEPPER,
    CROP_GRASS,
    CROP_MOON_DROP,
    CROP_PINK_CAT,
    CROP_BLUE_MAGIC_RED,
    CROP_RED_MAGIC_RED,
    CROP_TOY_FLOWER,
    CROP_ORANGE_CUP,
};

typedef struct FarmCrop {
    u16 season;
    u16 grownStage;
    s16 regrowStage;
    u16 days[4];
    ItemId harvestItem;
} FarmCrop;

#endif
