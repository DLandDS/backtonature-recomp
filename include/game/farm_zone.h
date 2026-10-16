#ifndef BTNX_GAME_FARM_TYPES_36_H
#define BTNX_GAME_FARM_TYPES_36_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/farm_result_pair.h"
#include "game/farm_zone_header.h"
#include "game/farm_zone_model.h"
#include "game/zone_point.h"

typedef struct FarmZone {
    u16 flags;
    u16 id;
    s16 param;
    u16 storyFlag;
    s16 facing;
    s16 event;
    s16 modelIndex;
    ZonePoint min;
    ZonePoint max;
    u16 targetX;
    u16 targetY;
    u16 targetZ;
    u8 pad20[0x26 - 0x20];
    ZonePoint entry;
} FarmZone;

typedef struct FarmModeScript {
    FarmZoneHeader head;
    FarmZone zones[50];
    FarmBlitFrame frames[8];
    u16 clampMin[4];
    u16 clampMax[4];
    FarmZoneModel models[15];
} FarmModeScript;

typedef struct FarmModeScriptImage {
    u8 bytes[0xBDC];
} FarmModeScriptImage;

typedef struct FarmSlotTable {
    u8 pad0[0x4];
    FarmImageSlot entries[8];
} FarmSlotTable;

typedef struct FarmTmdQuad {
    u32 header;
    u8 u0;
    u8 v0;
    u16 cba;
    u8 u1;
    u8 v1;
    u16 tsb;
    u8 u2;
    u8 v2;
    u16 pad0E;
    u8 u3;
    u8 v3;
} FarmTmdQuad;

enum FarmZoneEntryId {
    FARM_ZONE_FREE = 0,
    FARM_ZONE_CLOSED = 1,
    FARM_ZONE_OPENING = 2,
    FARM_ZONE_HELD_OPEN = 3,
    FARM_ZONE_CLOSING = 4,
};

typedef struct FarmZoneEntry {
    u16 id;
    u16 facing;
    u16 holdTimer;
    u8 pad6[0x8 - 0x6];
    s32 swingSpeed;
    s16 turnX;
    s16 turnY;
    s16 turnZ;
    u8 pad12[0x14 - 0x12];
    FarmZone *zone;
} FarmZoneEntry;

enum FarmOverviewCameraStep {
    OVERVIEW_CAMERA_SET_UP = 0,
    OVERVIEW_CAMERA_RETEXTURE = 1,
    OVERVIEW_CAMERA_MOVE = 2,
    OVERVIEW_CAMERA_FADE_OUT = 3,
};

typedef struct FarmOverviewCamera {
    u16 step;
    u8 pad002[2];
    s32 driftSpeed;
    s32 driftY;
    s32 cameraX;
    u8 pad010[4];
    s32 cameraZ;
} FarmOverviewCamera;

typedef struct FarmZoneBox {
    s32 xMin;
    s32 zMin;
    s32 xMax;
    s32 zMax;
    u16 kind;
    u16 resultKind;
    s32 result0;
    s32 result2;
} FarmZoneBox;

typedef struct FarmZoneRow {
    FarmZoneBox zones[2];
} FarmZoneRow;

#endif
