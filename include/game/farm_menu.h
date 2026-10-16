#ifndef BTNX_GAME_FARM_TYPES_15_H
#define BTNX_GAME_FARM_TYPES_15_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct FarmMenu {
    u16 *text[10];
    s16 entryValue[10];
    u8 entryFlag[10];
    s16 label46[10];
    u8 columns[2];
    s16 selected;
    u8 pad5E[2];
} FarmMenu;

typedef struct FarmMenuRequest {
    FarmMenu *menu;
    s16 first[2];
    s16 second[2];
    s16 third[2];
    s32 count;
} FarmMenuRequest;

#endif
