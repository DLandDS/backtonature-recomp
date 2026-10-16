#ifndef BTNX_GAME_FARM_TYPES_14_H
#define BTNX_GAME_FARM_TYPES_14_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/item.h"
#include "game/farm_action.h"
#include "game/farm_livestock.h"

typedef struct FarmFish {
    ItemId item;
    u16 days;
} FarmFish;

typedef struct FarmPond {
    FarmFish fish[99];
    u16 fed;
} FarmPond;

struct FarmActionSlot;

struct FarmHelper;

enum FarmRowSpotSlot {
    ROW_SPOT_MODE_SLOT_0 = -1,
    ROW_SPOT_MODE_SLOT_1 = -2,
};

enum FarmWalkFlags {
    FARM_WALK_AROUND_PLAYER = 0x8031,
    FARM_WALK_TO_PLAYER = 0x8030,
};

enum FarmLivestockMessage {
    LIVESTOCK_MSG_SICK_RECENT = 0x6C0,
    LIVESTOCK_MSG_SICK_LONG = 0x6C1,
    LIVESTOCK_MSG_PREGNANT = 0x6C2,
    LIVESTOCK_MSG_PREGNANT_START = 0x6C3,
    LIVESTOCK_MSG_PREGNANT_OVERDUE = 0x6C4,
    LIVESTOCK_MSG_HEALTHY = 0x6BE,
    LIVESTOCK_MSG_FOND = 0x6BD,
    LIVESTOCK_MSG_DEVOTED = 0x6BF,
};

enum FarmClaimResult {
    FARM_CLAIM_OK = 0,
    FARM_CLAIM_SLOT_TAKEN = 0x1E6,
    FARM_CLAIM_NO_ITEM = 0x1A1,
};

typedef struct FarmModeSlot {
    s16 value;
    s16 animal;
} FarmModeSlot;

typedef struct FarmAnimals {
    FarmChicken chickens[10];
    FarmLivestock livestock[20];
    FarmPond pond;
    u8 pad16F6[0x16F8 - 0x16F6];
    FarmPet dog;
    FarmPet horse;
    FarmAction actions[6];
    FarmChicken extraChickens[3];
    FarmModeSlot slots[4];
    u16 spawnGate;
    s16 moveSpeeds[5];
    s16 fishCount;
    s16 pendingTrack;
} FarmAnimals;

typedef struct FarmStoredItem {
    s16 id;
    s16 modifier;
    s16 count;
} FarmStoredItem;

typedef struct FarmRecipe {
    u16 id;
    s16 value;
    u16 options;
    u16 values[8];
} FarmRecipe;

#endif
