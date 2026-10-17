#ifndef BTN_NATIVE_GTE_H
#define BTN_NATIVE_GTE_H
#include "runtime.h"

typedef struct { int16_t x, y, z; } BtnVector;
typedef struct { int16_t m[3][3]; int32_t t[3]; } BtnMatrix;
typedef struct { int16_t x, y; } BtnScreenPoint;
typedef struct {
    BtnMatrix transform;
    int32_t ofx, ofy, dqb; /* Signed 16.16 offsets; depth cue uses register units. */
    int16_t dqa, zsf3, zsf4;
    uint16_t h, sz[4], otz;
    int16_t ir[3];
    uint16_t ir0;
    int32_t mac[4];
    BtnScreenPoint sxy[3];
    uint32_t flag;
} BtnGte;

/* A synchronous C subset for the reference scene, not a COP2 interpreter:
 * RTPS/RTPT(sf=1,lm=0), NCLIP, AVSZ3/4, and ordinary MVMVA.
 * Fixed-width state; shifts, MAC wrap and saturation avoid signed C overflow.
 */
void btn_gte_init(BtnGte *gte);
void btn_gte_rtps(BtnGte *gte, BtnVector vector);
void btn_gte_rtpt(BtnGte *gte, const BtnVector vectors[3]);
int32_t btn_gte_nclip(BtnGte *gte);
uint16_t btn_gte_avsz(BtnGte *gte, bool four);
void btn_gte_mvmva(BtnGte *gte, const BtnMatrix *matrix, BtnVector vector, bool shift, bool positive);
uint32_t btn_gte_divide(uint16_t h, uint16_t z, uint32_t *flag);

#endif
