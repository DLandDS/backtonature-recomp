#ifndef BTNX_GAME_FARM_TYPES_4_H
#define BTNX_GAME_FARM_TYPES_4_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct FarmDateEntry {
    u8 season;
    u8 day;
    s16 message;
} FarmDateEntry;

typedef struct {
    s16 value;
    s8 arg;
    u8 pad[1];
} FarmHourEvent;

#endif
