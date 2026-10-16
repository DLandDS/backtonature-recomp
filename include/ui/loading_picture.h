#ifndef BTNX_UI_UI_TYPES_2_H
#define BTNX_UI_UI_TYPES_2_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "graphics/sdk_types.h"

typedef struct LoadingPicture {
    s32 spriteStart;
    s32 packStart;
    u8 files[0x1800 - 8];
} LoadingPicture;

typedef struct LoadingPackets {
    u_long ots[2][32];
    SPRT sprites[2][32];
    DR_MODE modes[2][32];
    u8 pad900[0x1300 - 0x900];
} LoadingPackets;

enum NameEntryStep {
    NAME_STEP_CANCELLED = -1,
    NAME_STEP_IDLE = 0,
    NAME_STEP_ACCEPTED = 1,
    NAME_STEP_EDITING = 2,
};

#endif
