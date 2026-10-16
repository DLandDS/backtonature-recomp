#ifndef BTNX_GAME_SHARED_TYPES_6_H
#define BTNX_GAME_SHARED_TYPES_6_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum NudgeAxis {
    NUDGE_Z_UP = 0,
    NUDGE_X_UP = 1,
    NUDGE_Z_DOWN = 2,
    NUDGE_X_DOWN = 3,
    NUDGE_Y_UP = -1,
    NUDGE_Y_DOWN = -2,
};

typedef struct AFileRecord {
    u32 flag;
    char path[0x20];
    u32 pos;
} AFileRecord;

enum CdReadStatus { CD_READ_DONE = 0, CD_READ_ERROR = -1 };

enum {
    AFILE_DEBUG = 8,
    AFILE_PDA_SAMPLE = 10,
    AFILE_FARM = 11,
    AFILE_SAVELOAD = 15,
    AFILE_MG1 = 17,
    AFILE_MG2 = 19,
    AFILE_MG3 = 21,
    AFILE_MG4 = 23,
    AFILE_MG5 = 25,
    AFILE_TITLE = 27,
    AFILE_STAFF = 28,
    AFILE_SWIND = 30,
    AFILE_MCARD = 31
};

#endif
