#include "gte.h"
#include <limits.h>
#include <string.h>

/* C11 does not define arithmetic right shift of negative signed integers. */
static int64_t sar(int64_t value, unsigned shift)
{
    int64_t scale = INT64_C(1) << shift;
    return value >= 0 ? value / scale : -1 - (-(value+1) / scale);
}
static int32_t wrap32(int64_t value)
{
    uint32_t bits = (uint32_t)(uint64_t)value;
    return bits <= INT32_MAX ? (int32_t)bits : -1 - (int32_t)(UINT32_MAX-bits);
}
static void summary(BtnGte *g)
{
    if (g->flag & UINT32_C(0x7F87E000)) g->flag |= UINT32_C(0x80000000);
}
static int32_t saturate(BtnGte *g, int64_t value, int32_t low, int32_t high, unsigned flag)
{
    if (value < low) { g->flag |= UINT32_C(1) << flag; return low; }
    if (value > high) { g->flag |= UINT32_C(1) << flag; return high; }
    return (int32_t)value;
}
static int64_t mac44(BtnGte *g, int64_t value, unsigned row)
{
    const int64_t half = INT64_C(1) << 43, whole = INT64_C(1) << 44;
    if (value >= half) g->flag |= UINT32_C(1) << (30-row);
    if (value < -half) g->flag |= UINT32_C(1) << (27-row);
    uint64_t bits = (uint64_t)value & ((UINT64_C(1) << 44)-1);
    return bits >= (uint64_t)half ? (int64_t)bits-whole : (int64_t)bits;
}
static int32_t mac0(BtnGte *g, int64_t value)
{
    if (value > INT32_MAX) g->flag |= UINT32_C(1) << 16;
    if (value < INT32_MIN) g->flag |= UINT32_C(1) << 15;
    g->mac[0] = wrap32(value);
    return g->mac[0];
}
static int64_t transform_row(BtnGte *g, const BtnMatrix *m, const int16_t v[3], unsigned row)
{
    int64_t sum = (int64_t)m->t[row] * 4096 + (int64_t)m->m[row][0] * v[0];
    sum = mac44(g,sum,row);
    for (unsigned col = 1; col < 3; ++col)
        sum = mac44(g,sum+(int64_t)m->m[row][col]*v[col],row);
    return sum;
}

void btn_gte_init(BtnGte *g)
{
    memset(g,0,sizeof(*g));
    for (unsigned i = 0; i < 3; ++i) g->transform.m[i][i] = 4096;
    g->ofx = BTN_WIDTH/2 * 65536; g->ofy = BTN_HEIGHT/2 * 65536;
    g->h = 1000; g->zsf3 = 1365; g->zsf4 = 1024;
}

uint32_t btn_gte_divide(uint16_t h, uint16_t z, uint32_t *flag)
{
    if ((uint32_t)h >= (uint32_t)z * 2u) {
        if (flag) *flag |= UINT32_C(0x80020000);
        return 0x1FFFFu;
    }
    unsigned shift = 0;
    uint32_t denominator = z;
    while (denominator < 0x8000u) { denominator *= 2; ++shift; }
    uint64_t numerator = (uint32_t)h << shift;
    uint32_t index = (denominator-0x7FC0u) >> 7;
    int table = (int)((0x40000u/(index+0x100u)+1u)/2u)-0x101;
    uint32_t reciprocal = (uint32_t)(table < 0 ? 0 : table)+0x101u;
    uint32_t correction = (0x2000080u-denominator*reciprocal) >> 8;
    correction = (0x80u+correction*reciprocal) >> 8;
    uint64_t result = (numerator*correction+0x8000u) >> 16;
    return result > 0x1FFFFu ? 0x1FFFFu : (uint32_t)result;
}

static void project(BtnGte *g, BtnVector vertex, bool depth_cue)
{
    int16_t v[3] = {vertex.x,vertex.y,vertex.z};
    int64_t depth = 0;
    for (unsigned row = 0; row < 3; ++row) {
        int64_t result = sar(transform_row(g,&g->transform,v,row),12);
        g->mac[row+1] = wrap32(result);
        g->ir[row] = (int16_t)saturate(g,g->mac[row+1],-32768,32767,24-row);
        if (row == 2) depth = result;
    }
    memmove(g->sz,g->sz+1,3*sizeof(g->sz[0]));
    g->sz[3] = (uint16_t)saturate(g,depth,0,65535,18);
    uint32_t factor = btn_gte_divide(g->h,g->sz[3],&g->flag);
    memmove(g->sxy,g->sxy+1,2*sizeof(g->sxy[0]));
    int64_t x = (int64_t)factor*g->ir[0]+g->ofx, y = (int64_t)factor*g->ir[1]+g->ofy;
    mac0(g,x); mac0(g,y);
    g->sxy[2].x = (int16_t)saturate(g,sar(x,16),-1024,1023,14);
    g->sxy[2].y = (int16_t)saturate(g,sar(y,16),-1024,1023,13);
    if (depth_cue) {
        int64_t cue = (int64_t)factor*g->dqa+g->dqb;
        mac0(g,cue);
        g->ir0 = (uint16_t)saturate(g,sar(cue,12),0,4096,12);
    }
}
void btn_gte_rtps(BtnGte *g, BtnVector vertex)
{
    g->flag = 0; project(g,vertex,true); summary(g);
}
void btn_gte_rtpt(BtnGte *g, const BtnVector vertices[3])
{
    g->flag = 0;
    for (unsigned i = 0; i < 3; ++i) project(g,vertices[i],i==2);
    summary(g);
}
void btn_gte_mvmva(BtnGte *g, const BtnMatrix *m, BtnVector vertex, bool shift, bool positive)
{
    int16_t v[3] = {vertex.x,vertex.y,vertex.z};
    g->flag = 0;
    for (unsigned row = 0; row < 3; ++row) {
        g->mac[row+1] = wrap32(sar(transform_row(g,m,v,row),shift ? 12u : 0u));
        g->ir[row] = (int16_t)saturate(g,g->mac[row+1],positive ? 0 : -32768,32767,24-row);
    }
    summary(g);
}
int32_t btn_gte_nclip(BtnGte *g)
{
    const BtnScreenPoint *v = g->sxy;
    g->flag = 0;
    int64_t area = (int64_t)v[0].x*(v[1].y-v[2].y) + (int64_t)v[1].x*(v[2].y-v[0].y)
        + (int64_t)v[2].x*(v[0].y-v[1].y);
    int32_t result = mac0(g,area); summary(g); return result;
}
uint16_t btn_gte_avsz(BtnGte *g, bool four)
{
    g->flag = 0;
    int64_t total = (int64_t)g->sz[1]+g->sz[2]+g->sz[3]+(four ? g->sz[0] : 0);
    int64_t result = total*(four ? g->zsf4 : g->zsf3);
    mac0(g,result);
    g->otz = (uint16_t)saturate(g,sar(result,12),0,65535,18);
    summary(g); return g->otz;
}
