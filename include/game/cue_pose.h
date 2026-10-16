#ifndef BTNX_GAME_SHARED_TYPES_11_H
#define BTNX_GAME_SHARED_TYPES_11_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum CuePose {
    CUE_POSE_END = -1,
};

enum CueIcon {
    CUE_ICON_TALK = 0x22,
    CUE_ICON_SCOLD = 0x1F,
    CUE_ICON_NEGLECT = 0x24,
    CUE_ICON_PRAISE = 0x25,
};

enum CuePhase {
    CUE_PHASE_SHORT = 0x1E,
    CUE_PHASE_LONG = 0x2D,
};

enum CueStep {
    CUE_STEP_START = 0,
    CUE_STEP_PLAY = 1,
    CUE_STEP_STOP = -1,
};

#endif
