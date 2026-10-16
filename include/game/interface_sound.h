#ifndef BTNX_GAME_SHARED_TYPES_42_H
#define BTNX_GAME_SHARED_TYPES_42_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "graphics/sdk_types.h"
#include "graphics/prim_cursor.h"

enum InterfaceSound {
    SOUND_MENU_CONFIRM = 0,
    SOUND_MENU_CANCEL = 1,
    SOUND_MENU_REFUSE = 2,
    SOUND_MENU_OPEN_CLOSE = 3,
    SOUND_TEXT_BLIP = 4,
    SOUND_MENU_CURSOR = 5,
};

typedef struct FrameState {
    DRAWENV drawEnv;
    DISPENV dispEnv;
    u8 pad70[0x84 - 0x70];
    GsOT ots[4];
    u8 padD4[0xD8 - 0xD4];
    u_long *otTags[4];
    u8 padE8[0xEC - 0xE8];
    s32 otTagCounts[4];
    u8 padFC[0x100 - 0xFC];
    s32 otLengths[4];
    s32 otDepth;
    s32 otDepthBits;
    PrimCursor primCursor;
    s32 primSize;
    u_long *ot;
    u8 *primStart;
    u32 workSize;
    u8 *workBase;
    u8 *primBuffer;
} FrameState;

enum MainScene {
    SCENE_DEBUG = 0,
    SCENE_PDA_SAMPLE = 1,
    SCENE_FARM = 2,
    SCENE_MG1 = 3,
    SCENE_MG2 = 4,
    SCENE_MG3 = 5,
    SCENE_FARM_MINIGAME_EXIT = 6,
    SCENE_SAVE = 7,
    SCENE_LOAD = 8,
    SCENE_PDA = 9,
    SCENE_LIVESTOCK_TRADE = 10,
    SCENE_MG4 = 11,
    SCENE_MG5 = 12,
    SCENE_TITLE = 13,
    SCENE_FARM_LOADED = 14,
    SCENE_FARM_SAVED = 15,
    SCENE_STAFF_ROLL = 16,
    SCENE_MG_DEBUG = 17,
    SCENE_MEMCARD = 18,
    SCENE_FARM_MENU_RETURN = 19,
    SCENE_FARM_STAFF_EXIT = 20,
};

enum MainSpeedMode {
    MAIN_SPEED_HALF = 0,
    MAIN_SPEED_NORMAL = 1,
    MAIN_SPEED_FAST_2 = 2,
    MAIN_SPEED_FAST_3 = 3,
};

enum { OVERLAY_LOW, OVERLAY_HIGH };

#endif
