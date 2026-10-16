#ifndef BTNX_INPUT_INPUT_TYPES_H
#define BTNX_INPUT_INPUT_TYPES_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct InputRecord {
    u32 triggered;
    u32 held;
    u32 pressed;
    u32 previous;
    u32 repeatFrames;
    s32 repeating;
    s32 delay;
    s32 rate;
    s32 repeating2;
    u32 triggered2;
    u32 repeatFrames2;
    s32 delay2;
    s32 rate2;
    s16 analog;
    u8 actuator0;
    u8 actuator1;
} InputRecord;

enum PadButton {
    PAD_L2 = 0x0001,
    PAD_R2 = 0x0002,
    PAD_L1 = 0x0004,
    PAD_R1 = 0x0008,
    PAD_TRIANGLE = 0x0010,
    PAD_CIRCLE = 0x0020,
    PAD_CROSS = 0x0040,
    PAD_SQUARE = 0x0080,
    PAD_SELECT = 0x0100,
    PAD_START = 0x0800,
    PAD_UP = 0x1000,
    PAD_RIGHT = 0x2000,
    PAD_DOWN = 0x4000,
    PAD_LEFT = 0x8000,
    PAD_ARROWS_MASK = PAD_UP | PAD_RIGHT | PAD_DOWN | PAD_LEFT,
};

enum PadType {
    PAD_TYPE_DIGITAL = 4,
    PAD_TYPE_ANALOG = 7,
};

typedef struct PadBuffer {
    u8 status;
    u8 type;
    u8 buttons;
    u8 buttonsHigh;
    u8 rightX;
    u8 rightY;
    u8 leftX;
    u8 leftY;
    u8 pad8[0x1A];
} PadBuffer;

typedef struct PadPorts {
    u8 port[2];
} PadPorts;

#endif
