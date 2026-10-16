#ifndef BTNX_INPUT_INPUT_TYPES_2_H
#define BTNX_INPUT_INPUT_TYPES_2_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"
#include "game/demo_recording.h"

typedef struct InputDemo {
    u32 frame;
    u16 buttons;
    union {
        DemoRecording copy;
        u16 inputs[2048];
    } recording;
} InputDemo;

#endif
