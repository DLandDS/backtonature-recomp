#ifndef BTNX_GAME_FARM_TYPES_11_H
#define BTNX_GAME_FARM_TYPES_11_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum SeasoningFlag {
    SEASONING_FLAG_SUGAR = 1 << 7,
    SEASONING_FLAG_SALT = 1 << 8,
    SEASONING_FLAG_VINEGAR = 1 << 9,
    SEASONING_FLAG_SOY_SAUCE = 1 << 10,
    SEASONING_FLAG_MISO = 1 << 11,
};

#endif
