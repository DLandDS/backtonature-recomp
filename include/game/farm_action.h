#ifndef BTNX_GAME_FARM_TYPES_12_H
#define BTNX_GAME_FARM_TYPES_12_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/farm_actor.h"

typedef struct FarmAction {
    FarmActor actor;
    s16 countdown;
} FarmAction;

enum FarmTask {
    FARM_TASK_REST = 0,
    FARM_TASK_WALK = 1,
    FARM_TASK_WANDER = 2,
    FARM_TASK_VISITOR_WALK = 3,
    FARM_TASK_VISITOR_SIT = 4,
    FARM_TASK_MESSAGE = 5,
    FARM_TASK_COUNTDOWN = 6,
    FARM_TASK_DOG_HERD = 7,
    FARM_TASK_RABBIT_A = 8,
    FARM_TASK_RABBIT_B = 9,
    FARM_TASK_SQUIRREL = 10,
    FARM_TASK_FOX_A = 11,
    FARM_TASK_FOX_B = 12,
    FARM_TASK_MONKEY = 13,
    FARM_TASK_PLACED_CICADA = 14,
    FARM_TASK_PLACED_BEETLE = 15,
    FARM_TASK_PLACED_CRICKET = 16,
    FARM_TASK_CAT = 17,
    FARM_TASK_DOG_WALK = 18,
    FARM_TASK_DOG_LEAVE = 19,
    FARM_TASK_DOG_SEEK = 20,
};

typedef struct FarmChicken {
    FarmActor actor;
    u16 grazingTime;
    u16 daysSick;
    u16 pregnancy;
    s16 unusedValue;
} FarmChicken;

typedef struct FarmMessage {
    s16 message;
    s8 kind;
    s8 timer;
} FarmMessage;

typedef struct FarmCueStep {
    s16 pose;
    s16 frames;
} FarmCueStep;

#endif
