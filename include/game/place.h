#ifndef BTNX_GAME_SHARED_TYPES_29_H
#define BTNX_GAME_SHARED_TYPES_29_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum Place {
    PLACE_FARM = 1,
    PLACE_2 = 2,
    PLACE_3 = 3,
    PLACE_4 = 4,
    PLACE_5 = 5,
    PLACE_GREENHOUSE = 6,
    PLACE_8 = 8,
    PLACE_9 = 9,
    PLACE_B = 0xB,
    PLACE_C = 0xC,
    PLACE_D = 0xD,
    PLACE_E = 0xE,
    PLACE_F = 0xF,
    PLACE_13 = 0x13,
    PLACE_14 = 0x14,
    PLACE_18 = 0x18,
    PLACE_19 = 0x19,
    PLACE_1A = 0x1A,
    PLACE_1B = 0x1B,
    PLACE_1C = 0x1C,
    PLACE_1D = 0x1D,
    PLACE_1E = 0x1E,
    PLACE_1F = 0x1F,
    PLACE_22 = 0x22,
    PLACE_27 = 0x27,
    PLACE_28 = 0x28,
    PLACE_2A = 0x2A,
    PLACE_2C = 0x2C,
    PLACE_30 = 0x30,
    PLACE_33 = 0x33,
    PLACE_34 = 0x34,
    PLACE_35 = 0x35,
    PLACE_EVENT_CELL = 0x36,
    PLACE_MINE = 0x37,
    PLACE_38 = 0x38,
    PLACE_ANY = 0x3F,
    PLACE_42 = 0x42,
    PLACE_MINE_43 = 0x43,
    PLACE_44 = 0x44,
    PLACE_48 = 0x48,
};

enum AreaFlag {
    AREA_FLAG_BIT12 = (1 << 12),
    AREA_FLAG_BIT15 = (1 << 15),
};

enum AreaScene {
    AREA_HOUSE0_SPRING = 0x50,
    AREA_HOUSE0_SUMMER = 0x51,
    AREA_HOUSE0_FALL = 0x52,
    AREA_HOUSE0_WINTER = 0x53,
    AREA_HOUSE1_SPRING = 0x54,
    AREA_HOUSE1_SUMMER = 0x55,
    AREA_HOUSE1_FALL = 0x56,
    AREA_HOUSE1_WINTER = 0x57,
    AREA_HOUSE2_SPRING = 0x58,
    AREA_HOUSE2_SUMMER = 0x59,
    AREA_HOUSE2_FALL = 0x5A,
    AREA_HOUSE2_WINTER = 0x5B,
    AREA_MILESTONE_RESULT = 0xE5,
};

#endif
