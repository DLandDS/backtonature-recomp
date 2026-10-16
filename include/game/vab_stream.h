#ifndef BTNX_GAME_SHARED_TYPES_40_H
#define BTNX_GAME_SHARED_TYPES_40_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/vab_stream_state.h"

typedef struct {
    s32 current;
    u8 *dest;
    u32 chunkSize;
    u32 chunkCount;
    u32 chunkIndex;
    s32 offset;
    s32 vabId;
    enum VabStreamState state;
} VabStream;

#endif
