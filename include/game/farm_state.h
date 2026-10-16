#ifndef BTNX_GAME_FARM_TYPES_40_H
#define BTNX_GAME_FARM_TYPES_40_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/farm_time.h"
#include "game/vec3.h"
#include "game/farm_tile_rule.h"
#include "game/item_category.h"
#include "game/picture_grid.h"
#include "game/farm_fish.h"
#include "game/farm_bet_row.h"
#include "game/farm_ground_probe.h"
#include "game/farm_event_entry.h"
#include "game/character.h"
#include "game/farm_cell_kind.h"
#include "game/farm_family.h"
#include "game/field_cell_look.h"
#include "game/farm_helper.h"
#include "input/input_demo.h"
#include "game/farm_grid_entry.h"
#include "game/farm_player_definitions.h"
#include "game/farm_zone_header.h"
#include "game/farm_zone.h"
#include "game/farm_drift.h"
#include "game/pending_result.h"
#include "game/farm_system.h"

typedef struct FarmState {
    u32 unk00;
    u32 unk04;
    u32 frameCount;
    u32 globalFlags;
    u32 debugFlags;
    s32 lastCaller;
    s32 blockIndex;
    InputDemo *demo;
    ItemInfo *itemInfo;
    PictureGrid *pictureGrids;
    FarmTileRule *tileRules;
    FarmCrop *cropTable;
    FarmRequestSlot *request;
    u32 (*pictureCodes)[4];
    u32 eventFlags[37];
    FarmSystem system;
    FarmWork work;
    s16 pendingMode;
    PendingResult pendingResults[5];
    u32 statusFlags;
    struct FarmEventEntry *currentEvent;
    CharacterId villagerCursor;
    FarmGridEntry image;
    VECTOR camera;
    FarmProgress progress;
    FarmPlayer player;
    FarmTileRow fieldTiles[25];
    FarmSlotRow slots7x7[7];
    FarmRequests requests;
    FarmAnimals animals;
    FarmVillager villagers[CHARACTER_COUNT];
    FarmRuns runs;
    FarmHelper helpers[7];
    FarmWorld world;
    FarmRaceBets raceBets;
    FarmMinigames minigames;
    FarmDayCounts days;
    FarmDrift drift;
    FarmFamily family;
    FarmEnding ending;
} FarmState;

enum FarmMode {
    FARM_MODE_IDLE = 0,
    FARM_MODE_ZONE_WALK = 1,
    FARM_MODE_ZONE_WALK_WAIT = 2,
    FARM_MODE_EXIT_LOAD = 3,
    FARM_MODE_CYCLE_AREA = 4,
    FARM_MODE_EXIT_ARRIVE = 5,
    FARM_MODE_MG_DEBUG_SCREEN = 6,
    FARM_MODE_SLEEP = 12,
    FARM_MODE_ASLEEP = 13,
    FARM_MODE_SAVE_SCREEN = 14,
    FARM_MODE_LOAD_SCREEN = 15,
    FARM_MODE_PDA = 16,
    FARM_MODE_LIVESTOCK_TRADE = 17,
    FARM_MODE_MINIGAME_1 = 18,
    FARM_MODE_MINIGAME_2 = 19,
    FARM_MODE_MINIGAME_3 = 20,
    FARM_MODE_MINIGAME_4 = 21,
    FARM_MODE_MINIGAME_5 = 22,
    FARM_MODE_OVERVIEW = 23,
    FARM_MODE_TARGET_LOAD = 24,
    FARM_MODE_TARGET_ARRIVE = 25,
    FARM_MODE_DEMO_END = 26,
    FARM_MODE_TO_TITLE = 27,
    FARM_MODE_STAFF_ROLL = 28,
    FARM_MODE_OVERNIGHT_ASLEEP = 30,
    FARM_MODE_OVERNIGHT = 31,
};

enum FarmView {
    FARM_VIEW_NORMAL = 0,
    FARM_VIEW_IDLE_1 = 1,
    FARM_VIEW_IDLE_2 = 2,
    FARM_VIEW_OVERVIEW = 3,
};

typedef struct FarmModeGlobal {
    u8 pad0[0x1C08 - 0x0];
    s32 resourceBuffer[0x680];
    FarmTmdQuad *quads[25][45];
    FarmModeScript script;
    FarmZoneHeader previousHead;
    FarmModeScript *data;
    FarmZoneHeader *previous;
    FarmZone *exit;
    FarmScroll scroll;
    FarmSlotTable slots;
    u8 pad5470[0x5474 - 0x5470];
    LimitPair place;
    s32 loadedSlot;
    s32 area;
    s32 transitionTicks;
    s32 enteredArea;
    s32 absentArea;
    s32 view;
    u8 pad5490[0x5498 - 0x5490];
    s32 modeScene;
    s32 currentMode;
    u32 eventMode;
    s16 pictureIds[2][9];
    FarmGridEntry pictureEntries[2][9];
} FarmModeGlobal;

typedef struct FarmLoadArea {
    u8 *sectionBuffer;
    u8 *loadWindow;
    u8 anim8[0x1000];
    u8 sectorBuffer[0x3800];
    u8 spanBuffer[0x10000];
    u8 slotAnims[0x6000];
    u8 itemArt[0x2000];
    u8 stageData[0x1400];
    u8 seasonArt[0x1800];
    u8 slotSixArt[0xBF8];
    FarmModeGlobal mode;
} FarmLoadArea;

typedef struct FarmPayload {
    u8 data[0x10];
    s16 value;
} FarmPayload;

#endif
