#ifndef BTNX_GAME_SHARED_TYPES_35_H
#define BTNX_GAME_SHARED_TYPES_35_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum OvernightStep {
    OVERNIGHT_FADE_OUT = 0,
    OVERNIGHT_FADE_SETTLED = 1,
    OVERNIGHT_GROW_FARM = 2,
    OVERNIGHT_HOLD = 3,
    OVERNIGHT_CHANGE_SCENE = 4,
    OVERNIGHT_UPDATES = 5,
    OVERNIGHT_FINISH = 6,
};

#endif
