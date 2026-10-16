#ifndef BTNX_GAME_FARM_TYPES_17_H
#define BTNX_GAME_FARM_TYPES_17_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/farm_estate_record.h"
#include "game/farm_action.h"
#include "game/farm_fish.h"
#include "game/farm_bet_row.h"

typedef struct FarmGroundProbe {
    s32 x;
    s32 y;
    s32 z;
} FarmGroundProbe;

struct FarmGroundProbe;

typedef struct FarmScreenPoint {
    s32 x;
    s32 y;
    s32 depth;
} FarmScreenPoint;

enum EstateMessage {
    ESTATE_MESSAGE_BIRTH_KAI_POPURI_PRESENT = 0x31,
    ESTATE_MESSAGE_BIRTH_KAI_POPURI_AWAY = 0x32,
    ESTATE_MESSAGE_BIRTH_RIVAL_FIRST = 0x33,
    ESTATE_MESSAGE_BIRTH_MAYOR = 0x37,
};

typedef struct FarmLedger {
    s32 earnings;
    s32 expenses;
} FarmLedger;

enum EstateFlag {
    ESTATE_FLAG_VASE_SET = (1 << 1),
    ESTATE_FLAG_CATCH_ED = (1 << 2),
    ESTATE_FLAG_CATCH_EE = (1 << 3),
    ESTATE_FLAG_CATCH_EF = (1 << 4),
    ESTATE_FLAG_CATCH_F0 = (1 << 5),
    ESTATE_FLAG_CATCH_F1 = (1 << 6),
    ESTATE_FLAG_CATCH_F2 = (1 << 7),
    ESTATE_FLAG_CATCHES = (ESTATE_FLAG_CATCH_ED | ESTATE_FLAG_CATCH_EE | ESTATE_FLAG_CATCH_EF |
                           ESTATE_FLAG_CATCH_F0 | ESTATE_FLAG_CATCH_F1 | ESTATE_FLAG_CATCH_F2),
    ESTATE_FLAG_BARN_UPGRADED = (1 << 8),
    ESTATE_FLAG_BARN_FULL = (1 << 9),
    ESTATE_FLAG_ITEM_83 = (1 << 10),
    ESTATE_FLAG_BASKET = (1 << 11),
    ESTATE_FLAG_GREENHOUSE = (1 << 13),
    ESTATE_FLAG_CHICKEN_NOTICE = (1 << 14),
    ESTATE_FLAG_VISITORS = (1 << 15),
    ESTATE_FLAG_BIT16 = (1 << 16),
    ESTATE_FLAG_BIT17 = (1 << 17),
    ESTATE_FLAG_NOON_CHIME = (1 << 18),
    ESTATE_FLAG_EVENING_CHIME = (1 << 19),
    ESTATE_FLAG_COOP_BUILT = (1 << 20),
    ESTATE_FLAG_BIT21 = (1 << 21),
    ESTATE_FLAG_ROW97_BONUS = (1 << 22),
    ESTATE_FLAG_STRAY_DOGS = (1 << 23),
    ESTATE_FLAG_DOG_WALK = (1 << 24),
    ESTATE_FLAG_TRIGGER_ARMED = (1 << 25),
    ESTATE_FLAG_TAKE_ONCE = (1 << 26),
    ESTATE_FLAG_ANIMAL_AGED = (1 << 27),
};

enum EstateStatus {
    ESTATE_STATUS_SHIPPED_5D = (1 << 0),
    ESTATE_STATUS_SHIPPED_5F = (1 << 1),
    ESTATE_STATUS_GROUP_A_DONE = (1 << 2),
    ESTATE_STATUS_GROUP_B_DONE = (1 << 3),
    ESTATE_STATUS_GROUP_C_DONE = (1 << 4),
    ESTATE_STATUS_NO_BASKET = (1 << 5),
    ESTATE_STATUS_NO_RUCKSACK = (1 << 6),
    ESTATE_STATUS_RUCKSACK_UPGRADE_OPEN = (1 << 7),
    ESTATE_STATUS_AFFECTION_OVER_50000 = (1 << 8),
    ESTATE_STATUS_NO_COOP = (1 << 9),
    ESTATE_STATUS_NO_HOUSE = (1 << 10),
    ESTATE_STATUS_NO_BARN = (1 << 11),
    ESTATE_STATUS_HOUSE_LEVEL_1 = (1 << 12),
    ESTATE_STATUS_NO_GREENHOUSE = (1 << 13),
    ESTATE_STATUS_NO_ANIMAL_MEDICINE = (1 << 14),
    ESTATE_STATUS_BARN_ORDER_OPEN = (1 << 15),
    ESTATE_STATUS_COOP_ORDER_OPEN = (1 << 16),
    ESTATE_STATUS_BARN_FULL_ORDER_OPEN = (1 << 17),
    ESTATE_STATUS_NO_BRUSH = (1 << 18),
    ESTATE_STATUS_NO_MILKER = (1 << 19),
    ESTATE_STATUS_NO_CLIPPERS = (1 << 20),
    ESTATE_STATUS_NO_COW_MIRACLE_POTION = (1 << 21),
    ESTATE_STATUS_NO_SHEEP_MIRACLE_POTION = (1 << 22),
    ESTATE_STATUS_BIT23 = (1 << 23),
};

typedef struct FarmEstate {
    u8 utensils;
    u8 discardCount;
    u16 houseLevel;
    u16 barnLevel;
    u16 coopLevel;
    u16 greenhouseLevel;
    u16 pendingRequest;
    s16 stampTimer;
    s16 orderTimer;
    s16 shipmentTotal;
    s16 lumberDeliveries;
    u16 farmName[9];
    u8 pad026[0x02C - 0x026];
    u32 estateFlags;
    s32 noticesSeen;
    u16 lumber;
    u16 fodder;
    s16 pondFish;
    u16 fishFood;
    u16 chickenFeed;
    FarmStoredItem iceboxItems[64];
    FarmStoredItem toolboxItems[64];
    FarmStoredItem cabinetItems[64];
    FarmShipment shipment;
    FarmLedger ledger[4];
    FarmMessage queues[2][20];
    s16 powerBerries;
    u16 eventBits;
    FarmBasket basket;
    u8 barnFeedBins[22];
    u8 coopFeedBins[10];
    FarmRecipe recipeRecords[80];
    u32 statusFlags;
    s16 rucksackTimer;
    s16 carpenterOrder;
} FarmEstate;

enum EstateRun {
    ESTATE_RUN_ITEMS = 17,
    ESTATE_RUN_STAMPS = 18,
};

enum EstateStampRecord {
    ESTATE_STAMP_BASKET = 0,
    ESTATE_STAMP_DOG_BALL = 1,
};

enum EstateRecordFlag {
    ESTATE_RECORD_ENABLED = 0x1,
    ESTATE_RECORD_BOUNCE = 0x100,
    ESTATE_RECORD_RUCKSACK = 0x8000,
    ESTATE_RECORD_CLAIMED = 0x4000,
};

typedef struct FarmRuns {
    FarmEstateRecord *records[19];
    s16 counts[19];
} FarmRuns;

#endif
