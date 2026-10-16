#ifndef BTNX_GAME_FARM_TYPES_30_H
#define BTNX_GAME_FARM_TYPES_30_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "graphics/anim_file.h"
#include "game/vec3.h"
#include "game/item_cue.h"
#include "game/farm_fish.h"
#include "game/farm_player.h"
#include "game/character.h"
#include "game/farm_wedding.h"
#include "game/stored_item_kind.h"
#include "game/farm_player_toy.h"

typedef struct FarmPlayer {
    u16 name[9];
    s16 stamina;
    s16 maxStamina;
    s16 fatigue;
    s16 toolCharge;
    s16 idleFrames;
    s16 modeState;
    s16 rucksackLevel;
    s16 itemSlots;
    CharacterId talkVillager;
    s16 talkAnimal;
    s16 scriptRunning;
    s16 currentAction;
    u16 wokeAt;
    u16 sleptAt;
    s16 countdown;
    s16 facingIndex;
    s16 chickToy;
    s16 cowReins;
    u16 latches;
    u16 facing38;
    u16 prevFacing3A;
    u16 mode;
    s16 previousMode;
    u16 toolExp[5];
    s16 *scriptSteps;
    void *target;
    s32 targetFlags;
    s32 happiness;
    s32 gold;
    s32 medals;
    s32 scriptHeld;
    s32 scriptPressed;
    u32 stateFlags;
    s16 counts70[6];
    u8 seq7C[0x7E - 0x7C];
    FarmBirthday birthday;
    u8 pad82[0x84 - 0x82];
    FarmStoredItem held[2];
    FarmStoredItem tools[8];
    FarmStoredItem items[8];
    FarmStoredItem shown;
    u8 padF6[0xFC - 0xF6];
    LimitPair place;
    VECTOR position;
    s32 velocity[3];
    u8 pad11C[0x120 - 0x11C];
    VECTOR trail[10];
    VECTOR slowTrail[10];
    AnimState secondSprite;
    AnimState sprite;
    FarmWedding wedding;
    ItemCue cue;
    FarmPlayerToy toys[5];
    TrailPose poses[10];
} FarmPlayer;

#endif
