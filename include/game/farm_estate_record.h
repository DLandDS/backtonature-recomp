#ifndef BTNX_GAME_FARM_TYPES_8_H
#define BTNX_GAME_FARM_TYPES_8_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/item.h"

typedef struct FarmEstateRecord {
    s16 location;
    ItemId itemId;
    u16 recordFlags;
    u16 unk6;
    VECTOR position;
} FarmEstateRecord;

#endif
