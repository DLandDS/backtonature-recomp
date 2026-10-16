#ifndef BTNX_GAME_FARM_TYPES_39_H
#define BTNX_GAME_FARM_TYPES_39_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "graphics/anim_file.h"
#include "game/farm_time.h"
#include "game/farm_ground_probe.h"
#include "game/farm_event_entry.h"
#include "game/rival_couple.h"
#include "game/farm_wedding.h"
#include "game/farm_settings.h"
#include "game/farm_drift.h"

typedef struct FarmSystem {
    u8 menuState;
    u8 workSectorBusy;
    u8 pad02[0x04 - 0x02];
    s16 overnightStep;
    u32 clockFrames;
} FarmSystem;

typedef struct FarmWork {
    u8 sector[0x2000];
    u8 results[0xCE0];
    FarmEventEntry events[100];
    AnimState sprites[45];
} FarmWork;

typedef struct FarmProgress {
    u32 entryStep;
    u32 entrySubStep;
    s32 entryArgs[5];
    s32 entryArgs2[5];
    FarmClock clock;
    FarmEstate estate;
} FarmProgress;

enum FarmRequestState {
    FARM_REQUEST_START = 0,
    FARM_REQUEST_RUN = 1,
    FARM_REQUEST_FINISH = 2,
    FARM_REQUEST_LOAD_WAIT = 3,
};

typedef struct FarmRequests {
    enum FarmRequestState state;
    FarmRequestSlot items[5];
} FarmRequests;

typedef struct FarmWorld {
    s16 weather;
    s16 weatherTomorrow;
    FarmSettings options;
    FarmWedding rivalWeddings[RIVAL_COUPLE_COUNT];
} FarmWorld;

typedef struct FarmMinigames {
    u16 chickenSkill;
    u16 *textSource;
    s32 mg2Done;
    s16 mg3Team;
    s16 courseId;
    u32 mg3Done;
    s32 mg4PlayerWon;
    s16 mg4Winner;
    s32 mg5PlayerWon;
    s16 mg5Slot;
} FarmMinigames;

typedef struct FarmEndingTracks {
    u8 stages[10];
    u8 visits[2];
} FarmEndingTracks;

typedef struct FarmEnding {
    u16 route;
    u8 offers[9];
    u8 pad0B;
    FarmEndingTracks tracks;
} FarmEnding;

enum FarmDebugFlag {
    DEBUG_FLAG_DRAW_EVENT_NUMBERS = (1 << 7),
    DEBUG_FLAG_NO_HARVEST_SPRITES = (1 << 8),
    DEBUG_FLAG_NO_FIELD_DRAW = (1 << 9),
    DEBUG_FLAG_NO_BREAKING_ANIMATIONS = (1 << 10),
    DEBUG_FLAG_NO_EVENT_TRIGGERS = (1 << 11),
    DEBUG_FLAG_NO_EVENTS = (1 << 12),
    DEBUG_FLAG_NO_ANIMALS = (1 << 13),
    DEBUG_FLAG_NO_2D_FIXTURES = (1 << 14),
    DEBUG_FLAG_DRAW_MESSAGE_NUMBERS = (1 << 15),
    DEBUG_FLAG_WINTER_HOUSE = (1 << 16),
    DEBUG_FLAG_AUTUMN_HOUSE = (1 << 17),
    DEBUG_FLAG_SUMMER_HOUSE = (1 << 18),
    DEBUG_FLAG_SPRING_HOUSE = (1 << 19),
    DEBUG_FLAG_PUT_TIME = (1 << 23),
    DEBUG_FLAG_HISPEED_TIME = (1 << 24),
    DEBUG_FLAG_PEOPLE_PRO = (1 << 25),
    DEBUG_FLAG_DOOR_SCHEDULE = (1 << 26),
    DEBUG_FLAG_SCR_LIMIT = (1 << 27),
    DEBUG_FLAG_MYPOS_PUT = (1 << 28),
    DEBUG_FLAG_ENTER_ARTR = (1 << 29),
    DEBUG_FLAG_MUTEKI = (1 << 30),
    DEBUG_FLAG_CAMERA_CONTROL = (1 << 31),
};

enum FarmStatusFlag {
    STATUS_FLAG_DEBUG_EVENTS_300_AND_UP = (1 << 0),
    STATUS_FLAG_NO_TARGET_ACTION = (1 << 1),
    STATUS_FLAG_COLLAPSED = (1 << 2),
    STATUS_FLAG_SKIP_ANIMAL_UPDATE = (1 << 3),
    STATUS_FLAG_EVENT_HOLDS_RESOURCE_E = (1 << 4),
    STATUS_FLAG_GIFT_PENDING = (1 << 5),
    STATUS_FLAG_STARTUP_PENDING = (1 << 6),
    STATUS_FLAG_FROM_MG1 = (1 << 7),
    STATUS_FLAG_FROM_MG2 = (1 << 8),
    STATUS_FLAG_FROM_MG3 = (1 << 9),
    STATUS_FLAG_FROM_MG4 = (1 << 10),
    STATUS_FLAG_FROM_MG5 = (1 << 11),
    STATUS_FLAG_ASLEEP = (1 << 12),
    STATUS_FLAG_TARGET_RELOAD = (1 << 13),
    STATUS_FLAG_MUSIC_HOLD = (1 << 14),
};

#endif
