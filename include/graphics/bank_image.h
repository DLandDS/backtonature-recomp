#ifndef BTNX_GAME_SHARED_TYPES_H
#define BTNX_GAME_SHARED_TYPES_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct BankImage {
    u32 sizeFlags;
    u16 x, y;
    u16 w, h;
    u8 data[0];
} BankImage;

typedef struct ImageBank {
    u8 type;
    u8 pad01;
    u16 count;
    BankImage images[0];
} ImageBank;

#endif
