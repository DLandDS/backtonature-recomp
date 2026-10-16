#ifndef BTNX_GAME_SHARED_TYPES_27_H
#define BTNX_GAME_SHARED_TYPES_27_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum ReceiveItemState {
    RECEIVE_ITEM_HOLD = 0,
    RECEIVE_ITEM_HOLD_2 = 1,
    RECEIVE_ITEM_EAT = 2,
    RECEIVE_ITEM_DRINK = 3,
};

enum ThrownItemPhase {
    THROWN_ITEM_FLY = 0,
    THROWN_ITEM_HOLD = 1,
    THROWN_ITEM_BLINK = 2,
    THROWN_ITEM_LANDED = 3,
    THROWN_ITEM_EGG_LANDED = 4,
};

#endif
