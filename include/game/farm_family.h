#ifndef BTNX_GAME_FARM_TYPES_23_H
#define BTNX_GAME_FARM_TYPES_23_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/farm_date.h"
#include "game/item.h"

typedef struct FarmFamily {
    u16 addressMode;
    u16 childGrowthDays;
    u16 pregnancyDays;
    u16 wifeRequests;
    s16 pregnancyTicks;
    s16 wifeIndex;
    s16 wifeVillager;
    s16 wifeIgnoredDays;
    s16 childLevel;
    s32 childNamed;
    u16 childName[9];
    FarmDate selection;
    u8 pad32[0x34 - 0x32];
} FarmFamily;

typedef struct FarmFieldTile {
    ItemId value;
    u16 ground;
    u8 kind;
    u8 unused5;
    u16 growthFlags;
} FarmFieldTile;

enum FieldTileSpecial {
    FIELD_TILE_BIG_STONE_1 = 0x16C,
    FIELD_TILE_BIG_STONE_2 = 0x16D,
    FIELD_TILE_BIG_STONE_3 = 0x16E,
    FIELD_TILE_ROCK_1 = 0x16F,
    FIELD_TILE_ROCK_2 = 0x170,
    FIELD_TILE_ROCK_3 = 0x171,
    FIELD_TILE_STUMP_1 = 0x172,
    FIELD_TILE_STUMP_2 = 0x173,
    FIELD_TILE_STUMP_3 = 0x174,
    FIELD_TILE_LUMBER = 0x175,
};

#endif
