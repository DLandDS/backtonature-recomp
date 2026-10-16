#ifndef BTNX_GAME_FARM_TYPES_24_H
#define BTNX_GAME_FARM_TYPES_24_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/farm_family.h"

enum FieldCellLook {
    FIELD_LOOK_BARE = 0,
    FIELD_LOOK_TILLED = 1,
    FIELD_LOOK_WATERED = 2,
    FIELD_LOOK_SOWN = 3,
    FIELD_LOOK_WATERED_SOWN = 4,
    FIELD_LOOK_CROP = 5,
};

typedef struct FarmSlotRow {
    FarmFieldTile rows[7];
} FarmSlotRow;

typedef struct FarmTileRow {
    FarmFieldTile rows[45];
} FarmTileRow;

typedef struct FarmInitPair {
    s16 kind;
    s16 count;
} FarmInitPair;

#endif
