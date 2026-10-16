#ifndef BTNX_GRAPHICS_RENDER_TYPES_H
#define BTNX_GRAPHICS_RENDER_TYPES_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

/* SDK types our psyq_*.h headers do not declare (PSY-Q 4.7 libgpu.h, sys/types.h). */
typedef unsigned char u_char;

typedef unsigned short u_short;

typedef unsigned int u_int;

typedef unsigned long u_long;

typedef struct {
    u_long tag;
    u_long code[15];
} DR_ENV;

typedef struct {
    RECT clip;
    short ofs[2];
    RECT tw;
    u_short tpage;
    u_char dtd;
    u_char dfe;
    u_char isbg;
    u_char r0, g0, b0;
    DR_ENV drEnv;
} DRAWENV;

typedef struct {
    RECT disp;
    RECT screen;
    u_char isinter;
    u_char isrgb24;
    u_char pad0, pad1;
} DISPENV;

typedef struct {
    unsigned p : 24;
    unsigned char num : 8;
} GsOT_TAG;

typedef struct {
    unsigned long length;
    GsOT_TAG *org;
    unsigned long offset;
    unsigned long point;
    GsOT_TAG *tag;
} GsOT;

#endif
