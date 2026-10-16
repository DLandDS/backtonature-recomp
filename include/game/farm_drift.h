#ifndef BTNX_GAME_FARM_TYPES_38_H
#define BTNX_GAME_FARM_TYPES_38_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct FarmDrift {
    s32 red;
    s32 green;
    s32 blue;
    s32 stepRed;
    s32 stepGreen;
    s32 stepBlue;
    s32 stepsTaken;
    s32 stepsToTake;
    s32 index;
} FarmDrift;

enum Weather {
    WEATHER_SUNNY = 0,
    WEATHER_RAIN = 1,
    WEATHER_SNOW = 2,
    WEATHER_STORM = 3,
    WEATHER_BLIZZARD = 4,
};

typedef struct FarmScroll {
    s32 z;
    s32 x;
    s32 unused8;
    s32 yaw;
    s32 unused10;
    u16 dx;
    u16 dy;
    s16 a18;
    s16 b1A;
    s16 c1C;
    u16 pad1E;
    SVECTOR centre;
} FarmScroll;

typedef struct FarmParticle {
    s16 x;
    s16 y;
    s16 sprite;
    u16 frame;
    s16 delay;
    u16 age;
    u16 flags;
} FarmParticle;

enum FarmParticleFlag {
    PARTICLE_FLAG_FAST = 0x1000,
    PARTICLE_FLAG_DOUBLED = 0x2000,
    PARTICLE_FLAG_MOVING = 0x4000,
};

typedef struct FarmRequestSlot {
    s16 value;
    s16 args[5];
} FarmRequestSlot;

#endif
