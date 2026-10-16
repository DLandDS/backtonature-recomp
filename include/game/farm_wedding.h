#ifndef BTNX_GAME_FARM_TYPES_22_H
#define BTNX_GAME_FARM_TYPES_22_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/farm_date.h"
#include "game/character.h"

typedef struct FarmWedding {
    FarmDate date;
    CharacterId groom;
    CharacterId bride;
} FarmWedding;

#endif
