#ifndef BTNX_GAME_SHARED_TYPES_7_H
#define BTNX_GAME_SHARED_TYPES_7_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum TileRuleStatus {
    TILE_RULE_NONE = 0,
    TILE_RULE_BLOCKED = 1,
    TILE_RULE_OPEN = 2,
};

enum TileUsable {
    TILE_USABLE_REQUEST = 0x800,
    TILE_USABLE_TOOL = 0x2000,
    TILE_USABLE_ALT_TOOL = 0x4000,
    TILE_USABLE_HAND = 0x8000,
};

#endif
