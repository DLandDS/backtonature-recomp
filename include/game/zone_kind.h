#ifndef BTNX_GAME_SHARED_TYPES_33_H
#define BTNX_GAME_SHARED_TYPES_33_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum ZoneKind {
    ZONE_OPTIONS = 1,
    ZONE_PICK_MENU = 2,
    ZONE_MESSAGE_MACHINE = 3,
    ZONE_STORAGE_7 = 4,
    ZONE_COOKING_MENU = 5,
    ZONE_PENDING_OP = 6,
    ZONE_CALENDAR = 8,
    ZONE_FODDER_STORE = 9,
    ZONE_FISH_FOOD_STORE = 10,
    ZONE_KIND_12 = 12,
    ZONE_LUMBER_STOCK = 13,
    ZONE_EGG_BOX = 14,
    ZONE_WOOL_BOX = 15,
    ZONE_VALUE_MENU = 16,
    ZONE_LUMBER_SPOT = 18,
    ZONE_REQUEST_BOX = 20,
    ZONE_FEE_DESK = 21,
    ZONE_STORAGE_6 = 22,
    ZONE_STORAGE_4 = 23,
    ZONE_COOKING_RECORDS = 24,
    ZONE_CHICKEN_FEED_STORE = 26,
    ZONE_MARRIAGE_MENU = 30,
    ZONE_DAY_TIER_MENU = 31,
    ZONE_ITEM_CHECK = 32,
    ZONE_MILK_BOX = 33,
    ZONE_SHIPPING_BOX = 34,
    ZONE_ENDING_TRACK = 35,
    ZONE_DELIVERY = 36,
    ZONE_SHOP_COUNTER = 37,
    ZONE_EGG_TARGET = 38,
    ZONE_EVENT_TRIGGER = 39,
    ZONE_COOP_FEED_BIN = 40,
    ZONE_BARN_FEED_BIN = 41,
    ZONE_KIND_42 = 42,
    ZONE_KIND_43 = 43,
    ZONE_GIFT_TARGET = 44,
    ZONE_POND = 45,
    ZONE_HONEY = 47,
    ZONE_TREASURE = 48,
    ZONE_KIND_49 = 49,
};

typedef struct ResultClampBounds {
    s16 firstMin;
    s16 unk2;
    s16 secondMax;
    u16 pad6;
    s16 firstMax;
    s16 unkA;
    s16 secondMin;
} ResultClampBounds;

#endif
