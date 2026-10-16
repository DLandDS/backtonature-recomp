#ifndef BTNX_GAME_SHARED_TYPES_31_H
#define BTNX_GAME_SHARED_TYPES_31_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct ZonePoint {
    s16 x;
    s16 y;
    s16 z;
} ZonePoint;

enum ZoneFlag {
    ZONE_FLAG_BIT3 = (1 << 3),
    ZONE_FLAG_MARKER = (1 << 7),
    ZONE_FLAG_TRIGGER = (1 << 8),
    ZONE_FLAG_HAS_EVENT = (1 << 9),
    ZONE_FLAG_BIT13 = (1 << 13),
    ZONE_FLAG_ACTIVE = (1 << 14),
    ZONE_FLAG_FACING = (1 << 12),
    ZONE_FLAG_DEAD = (ZONE_FLAG_BIT3 | ZONE_FLAG_BIT13 | ZONE_FLAG_ACTIVE),
};

#endif
