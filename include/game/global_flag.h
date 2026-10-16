#ifndef BTNX_GAME_SHARED_TYPES_36_H
#define BTNX_GAME_SHARED_TYPES_36_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum GlobalFlag {
    GLOBAL_FLAG_BIT12 = (1 << 12),
    GLOBAL_FLAG_NEW_GAME = (1 << 13),
    GLOBAL_FLAG_CONTINUE = (1 << 14),
    GLOBAL_FLAG_DEMO_START = (1 << 15),
    GLOBAL_FLAG_INPUT_REPLAY = (1 << 16),
    GLOBAL_FLAG_TRANSITION_BEGIN = (1 << 17),
    GLOBAL_FLAG_TRANSITION_END = (1 << 18),
    GLOBAL_FLAG_MINIGAME_EXIT = (1 << 19),
    GLOBAL_FLAG_MINUTE_PASSED = (1 << 20),
    GLOBAL_FLAG_BIT21 = (1 << 21),
    GLOBAL_FLAG_CAMERA_BUSY = (1 << 22),
    GLOBAL_FLAG_TRANSITION = (1 << 23),
    GLOBAL_FLAG_HOUR_PASSED = (1 << 25),
    GLOBAL_FLAG_PAUSED = (1 << 27),
    GLOBAL_FLAG_MENU_OPEN = (1 << 28),
    GLOBAL_FLAG_CLOCK_TICKED = (1 << 30),
};

#endif
