#ifndef BTNX_GAME_SHARED_TYPES_24_H
#define BTNX_GAME_SHARED_TYPES_24_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum PictureCellFlag {
    PICTURE_CELL_BIT0 = 0x1,
    PICTURE_CELL_BIT1 = 0x2,
    PICTURE_CELL_HALF_SHIFT = 0x4,
    PICTURE_CELL_CENTER = 0x8,
};

#endif
