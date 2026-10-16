#ifndef BTNX_GAME_SHARED_TYPES_25_H
#define BTNX_GAME_SHARED_TYPES_25_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum StoredItemKind {
    STORED_ITEM_STASH = 0,
    STORED_ITEM_SEED = 1,
    STORED_ITEM_GOODS = 2,
    STORED_ITEM_FOOD = 3,
    STORED_ITEM_DRINK = 4,
    STORED_ITEM_CARRY = 5,
};

enum PictureRow {
    PICTURE_ROW_TOOL = 0,
    PICTURE_ROW_PICKUP = 1,
    PICTURE_ROW_OTHER = 2,
};

typedef struct TrailPose {
    u16 animation;
    s16 facing;
} TrailPose;

#endif
