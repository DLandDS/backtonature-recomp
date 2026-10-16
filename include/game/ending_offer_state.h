#ifndef BTNX_GAME_SHARED_TYPES_37_H
#define BTNX_GAME_SHARED_TYPES_37_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "graphics/sdk_types.h"

enum EndingOfferState {
    ENDING_OFFER_FREE = 0,
    ENDING_OFFER_OFFERED = 1,
    ENDING_OFFER_ACCEPTED = 2,
    ENDING_OFFER_PAID = 3,
    ENDING_OFFER_CLOSED = 0xFF,
};

typedef struct ClipPlane {
    s16 x;
    s16 y;
    s16 z;
} ClipPlane;

typedef void (*Bits16Fn)();

typedef struct ClutFade {
    u_long *clut;
    s16 direction;
    s16 unused6;
    s16 span;
    s16 position;
    RECT load;
} ClutFade;

struct LineF2;

struct PolyF4;

struct PolyG4;

struct Tile1;

typedef struct PolyFT4 {
    u8 addr[3];
    u8 len;
    u8 r0, g0, b0;
    u8 code;
    s32 xy0;
    u32 uvClut0;
    s32 xy1;
    u32 uvTpage1;
    s32 xy2;
    u32 uv2;
    s32 xy3;
    u32 uv3;
} PolyFT4;

#endif
