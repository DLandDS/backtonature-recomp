#ifndef BTNX_GAME_FARM_TYPES_16_H
#define BTNX_GAME_FARM_TYPES_16_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/farm_menu.h"

typedef struct FarmBetRow {
    s16 runner;
    s16 training;
    s16 affection;
    s16 chance;
    s16 rivalChance;
    s16 odds;
    s16 tickets;
} FarmBetRow;

typedef struct FarmRaceBets {
    s16 race;
    FarmBetRow rows[3][6];
    u8 pad0FE[0x100 - 0xFE];
    s32 horseWon;
    s16 medalTotal;
    u8 pad106[0x108 - 0x106];
} FarmRaceBets;

typedef struct FarmShipment {
    u16 counts[25];
    s32 shippedValue;
} FarmShipment;

typedef struct FarmBasket {
    u16 count;
    u16 items[25];
    s32 contentsValue;
} FarmBasket;

typedef struct FarmBuyDialogue {
    s16 chosen[2];
    u8 pad04[4];
    s16 stored[2];
    u8 pad0C[4];
    s16 cancelled[2];
    u8 pad14[4];
    FarmMenuRequest menu;
} FarmBuyDialogue;

#endif
