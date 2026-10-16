#ifndef BTNX_UI_UI_TYPES_H
#define BTNX_UI_UI_TYPES_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum MenuState {
    MENU_STATE_IDLE = 0,
    MENU_STATE_BEGIN_LOAD = 1,
    MENU_STATE_LOAD_RESULT = 2,
    MENU_STATE_OPEN = 3,
    MENU_STATE_RUN = 4,
    MENU_STATE_CLOSE = 5,
    MENU_STATE_FINISH = 6,
};

#endif
