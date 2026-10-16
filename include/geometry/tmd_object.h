#ifndef BTNX_GEOMETRY_GEOMETRY_TYPES_2_H
#define BTNX_GEOMETRY_GEOMETRY_TYPES_2_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct TmdObject {
    u32 vertTop;
    u32 nVert;
    u32 normalTop;
    u32 nNormal;
    u32 primitiveTop;
    u32 nPrimitive;
    s32 scale;
} TmdObject;

enum TmdPrimKey {
    TMD_PRIM_F3 = 0x2000,
    TMD_PRIM_F3_FLAG4 = 0x2004,
    TMD_PRIM_F3_RAW = 0x2101,
    TMD_PRIM_FT3 = 0x2400,
    TMD_PRIM_FT3_RAW = 0x2501,
    TMD_PRIM_F4 = 0x2800,
    TMD_PRIM_F4_FLAG4 = 0x2804,
    TMD_PRIM_F4_RAW = 0x2901,
    TMD_PRIM_FT4 = 0x2C00,
    TMD_PRIM_FT4_RAW = 0x2D01,
    TMD_PRIM_G3 = 0x3000,
    TMD_PRIM_G3_FLAG4 = 0x3004,
    TMD_PRIM_G3_RAW = 0x3101,
    TMD_PRIM_GT3 = 0x3400,
    TMD_PRIM_GT3_RAW = 0x3501,
    TMD_PRIM_G4 = 0x3800,
    TMD_PRIM_G4_FLAG4 = 0x3804,
    TMD_PRIM_G4_RAW = 0x3901,
    TMD_PRIM_GT4 = 0x3C00,
    TMD_PRIM_GT4_RAW = 0x3D01,
    TMD_PRIM_LINE_F2_FLAG1 = 0x4001,
    TMD_PRIM_LINE_G2_FLAG1 = 0x5001,
};

typedef struct TmdHeader {
    u32 id;
    u32 flags;
    s32 nObj;
    TmdObject objects[1];
} TmdHeader;

typedef struct ModelCenters {
    s32 clip;
    s16 coords[1];
} ModelCenters;

typedef struct Model {
    TmdHeader *tmd;
    ModelCenters *centers;
} Model;

#endif
