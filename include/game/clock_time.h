#ifndef BTNX_GAME_SHARED_TYPES_3_H
#define BTNX_GAME_SHARED_TYPES_3_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum ClockTime {
    CLOCK_DAY_START_HOUR = 6,
    CLOCK_NOON_HOUR = 12,
    CLOCK_DAY_START_MINUTE = 360,
    CLOCK_NOON_MINUTE = 720,
    CLOCK_EVENING_MINUTE = 1080,
    CLOCK_DAY_MINUTES = 1440,
};

enum ClockSize {
    MINUTES_PER_HOUR = 60,
    HOURS_PER_DAY = 24,
    DAYS_PER_WEEK = 7,
    DAYS_PER_SEASON = 30,
    SEASONS_PER_YEAR = 4,
    MAX_YEAR = 98,
    CLOCK_EVENT_COUNT = 10,
};

#endif
