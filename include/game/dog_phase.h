#ifndef BTNX_GAME_ANIMAL_TYPES_3_H
#define BTNX_GAME_ANIMAL_TYPES_3_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum DogPhase {
    DOG_PHASE_IDLE = 0,
    DOG_PHASE_NIGHT = 2,
    DOG_PHASE_HIT = 3,
    DOG_PHASE_TRY_ACTION = 4,
    DOG_PHASE_SEEK_ITEM = 5,
    DOG_PHASE_WALK_TO_ACTION = 6,
    DOG_PHASE_FETCH = 7,
    DOG_PHASE_PLACED = 9,
    DOG_PHASE_START_ACTION = 0x11,
    DOG_PHASE_IDLE_ACTION = 0x12,
};

enum DogActionRow {
    DOG_ROW_OWN = 0,
    DOG_ROW_TO_PLAYER = 1,
};

#endif
