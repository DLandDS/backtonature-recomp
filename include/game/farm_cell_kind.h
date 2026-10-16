#ifndef BTNX_GAME_FARM_TYPES_21_H
#define BTNX_GAME_FARM_TYPES_21_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "graphics/anim_file.h"
#include "game/vec3.h"
#include "game/item_cue.h"
#include "game/farm_player.h"
#include "game/character.h"

enum FarmCellKind {
    ROUTE_CELL_STAND = 0,
    ROUTE_CELL_WAIT_WALK = 1,
    ROUTE_CELL_ENTER = 2,
    ROUTE_CELL_EVENT = 3,
    ROUTE_CELL_SCHEDULE = 4,
    ROUTE_CELL_ENTER_FLAGGED = 5,
    ROUTE_CELL_WAIT_WALK_2 = 6,
    ROUTE_CELL_KEPT_PLACE = 7,
    ROUTE_CELL_SHIPMENT = 9,
    ROUTE_CELL_STEP = 0x10,
    ROUTE_CELL_EVENT_2 = 0x11,
    ROUTE_CELL_EVENT_FLAGGED = 0x12,
    ROUTE_CELL_STAND_2 = 0xFFFF,
    ROUTE_CELL_JUMP = 0x8000,
};

typedef struct FarmCell {
    u16 kind;
    s16 id;
    u16 pose;
    s16 x;
    s16 y;
    s16 z;
} FarmCell;

typedef FarmCell FarmRouteRow[17];

typedef struct FarmVillager {
    CharacterId id;
    s16 friendship;
    u16 affection;
    s16 facing;
    s16 lockCount;
    s16 delay;
    s16 column;
    s16 row;
    s16 wait;
    s16 talkCooldown;
    s16 destination;
    s16 prevDestination;
    u16 unused18;
    u16 action;
    u16 prevAction;
    u16 flags;
    u16 placeFlags;
    u16 step;
    FarmBirthday birthday;
    s32 scriptOffset;
    LimitPair place;
    VECTOR position;
    VECTOR fixedPosition;
    VECTOR speed;
    VECTOR ePosition;
    VECTOR eSpeed;
    AnimState sprite80;
    AnimState sprite;
    ItemCue cue;
    VillagerStep schedule[30];
} FarmVillager;

#endif
