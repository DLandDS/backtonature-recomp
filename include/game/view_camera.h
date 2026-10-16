#ifndef BTNX_GAME_SHARED_TYPES_28_H
#define BTNX_GAME_SHARED_TYPES_28_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct ViewCamera {
    s32 eyeX;
    s32 eyeY;
    s32 eyeZ;
    s32 targetX;
    s32 targetY;
    s32 targetZ;
    s32 roll;
    MATRIX *lockedView;
} ViewCamera;

#endif
