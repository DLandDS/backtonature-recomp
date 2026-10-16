#ifndef BTNX_GAME_SHARED_TYPES_2_H
#define BTNX_GAME_SHARED_TYPES_2_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum Weekday {
    WEEKDAY_SUNDAY,
    WEEKDAY_MONDAY,
    WEEKDAY_TUESDAY,
    WEEKDAY_WEDNESDAY,
    WEEKDAY_THURSDAY,
    WEEKDAY_FRIDAY,
    WEEKDAY_SATURDAY
};

enum ClockPeriod {
    CLOCK_PERIOD_DAWN,
    CLOCK_PERIOD_DAY,
    CLOCK_PERIOD_DUSK,
    CLOCK_PERIOD_NIGHT,
};

#endif
