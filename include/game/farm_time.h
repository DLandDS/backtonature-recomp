#ifndef BTNX_GAME_FARM_TYPES_3_H
#define BTNX_GAME_FARM_TYPES_3_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/farm_date.h"

typedef struct FarmTime {
    s16 hour;
    s16 minute;
} FarmTime;

typedef union FarmTimeOfDay {
    FarmTime hm;
    s32 word;
} FarmTimeOfDay;

typedef struct FarmElapsed {
    s16 hours;
    u16 minutes;
} FarmElapsed;

typedef struct FarmClock {
    FarmDate date;
    FarmTimeOfDay time;
    FarmElapsed elapsed;
    s16 unused10;
    s16 minuteOfDay;
} FarmClock;

enum FarmDayValue {
    DAY_VALUE_INVITATION_YEAR = 3,
};

typedef struct FarmDayCounts {
    u8 count0[0x80];
    u8 count1[8];
    u8 values[0x80];
} FarmDayCounts;

#endif
