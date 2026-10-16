#ifndef BTNX_GAME_SHARED_TYPES_9_H
#define BTNX_GAME_SHARED_TYPES_9_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct PictureGrid {
    u8 cells[13][13];
    u8 pad[1];
    u16 unkAA;
    u16 unkAC;
    u16 unkAE;
} PictureGrid;

#endif
