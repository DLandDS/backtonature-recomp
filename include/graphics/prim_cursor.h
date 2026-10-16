#ifndef BTNX_GRAPHICS_RENDER_TYPES_4_H
#define BTNX_GRAPHICS_RENDER_TYPES_4_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "graphics/sdk_types.h"
#include "game/ending_offer_state.h"

typedef union PrimCursor {
    u8 *bytes;
    u_long *tag;
    POLY_FT4 *ft4;
    SPRT *sprt;
    DR_MODE *mode;
    PolyFT4 *ft4Words;
} PrimCursor;

#endif
