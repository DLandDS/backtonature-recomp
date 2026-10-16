#ifndef BTNX_GAME_SHARED_TYPES_15_H
#define BTNX_GAME_SHARED_TYPES_15_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum LumberAddMode {
    LUMBER_ADD_CARRIED = 0,
    LUMBER_ADD_PICKED = 1,
    LUMBER_ADD_STUMP = 2,
    LUMBER_ADD_BOUGHT = 3,
};

enum FodderAddMode {
    FODDER_ADD_CARRIED = 0,
    FODDER_ADD_PICKED = 1,
    FODDER_ADD_BOUGHT = 2,
};

#endif
