#ifndef BTNX_GRAPHICS_RENDER_TYPES_2_H
#define BTNX_GRAPHICS_RENDER_TYPES_2_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "graphics/bank_image.h"

typedef struct AnimFile {
    s32 magic;
    u8 pad04[0xC];
    s32 section[5];
} AnimFile;

typedef struct AnimImage {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} AnimImage;

typedef struct AnimPartList {
    s16 count;
    u16 offset;
} AnimPartList;

typedef struct AnimPart {
    u16 image;
    u16 palette;
    s16 dx;
    s16 dy;
    u8 attr;
    u8 place;
    u8 pad[2];
} AnimPart;

typedef struct AnimRange {
    s16 frames;
    u16 offset;
} AnimRange;

typedef struct AnimFrame {
    u16 parts;
    s16 dx;
    s16 dy;
    u8 duration;
    u8 control;
    u8 pad[2];
} AnimFrame;

enum AnimStatus {
    ANIM_STATUS_PAUSED = 4,
};

typedef struct AnimState {
    u16 status;
    u16 flags;
    u8 animation;
    u8 frame;
    u16 ticks;
    s32 scale;
    s16 pageOffX;
    s16 pageOffY;
    s16 clutOffX;
    s16 clutOffY;
    AnimFile *source;
    ImageBank *bank;
    AnimImage *images;
    AnimPartList *partLists;
    u8 *parts;
    AnimRange *ranges;
    u8 *frames;
} AnimState;

enum AnimFlag {
    ANIM_FLAG_SEMI_TRANSPARENT = 0x20,
    ANIM_FLAG_ENDED = 0x100,
    ANIM_FLAG_JUMPED = 0x200,
};

enum AnimDrawFlag {
    ANIM_DRAW_FLIP_X = 0x8,
    ANIM_DRAW_FLIP_Y = 0x10,
    ANIM_DRAW_CURSOR = 0x40,
};

typedef struct VramPlace {
    s16 clutX;
    s16 clutY;
    s16 pageX;
    s16 pageY;
} VramPlace;

typedef struct AnimPackToc {
    s32 count;
    s32 start[0];
} AnimPackToc;

#endif
