#ifndef BTNX_GEOMETRY_GEOMETRY_TYPES_H
#define BTNX_GEOMETRY_GEOMETRY_TYPES_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

typedef struct {
    MATRIX rotation;
    MATRIX translation;
    SVECTOR angle;
    VECTOR position;
    long flag;
} CameraWork;

#endif
