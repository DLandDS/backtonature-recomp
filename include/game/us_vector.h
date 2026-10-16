#ifndef BTNX_GAME_SHARED_TYPES_30_H
#define BTNX_GAME_SHARED_TYPES_30_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct USVector {
    u16 vx, vy;
    u16 vz, pad;
} USVector;

enum SceneEntryFlag {
    SCENE_ENTRY_SLOT_MASK = 0xF,
    SCENE_ENTRY_SLOTTED = (1 << 8),
    SCENE_ENTRY_MARKED = (1 << 4),
    SCENE_ENTRY_TILT = (1 << 5),
    SCENE_ENTRY_PAN = (1 << 6),
    SCENE_ENTRY_LINK = (1 << 7),
    SCENE_ENTRY_TWIN = (1 << 10),
    SCENE_ENTRY_HIDDEN = (1 << 12),
    SCENE_ENTRY_SPIN = (1 << 13),
    SCENE_ENTRY_MORPH = (1 << 14),
    SCENE_ENTRY_ABSENT = (1 << 15),
};

#endif
