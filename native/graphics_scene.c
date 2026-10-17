#include "graphics_scene.h"

void btn_graphics_test_texture(BtnGraphics *g)
{
    for (unsigned y=0; y<16; ++y) for (unsigned x=0; x<16; ++x)
        g->words[(400+y)*BTN_VRAM_W+640+x]=(uint16_t)((x/4==y/4 ? 0x8000u : 0u)
            | (x*2) | ((y*2)<<5) | (16u<<10));
}
static BtnVertex vertex(int x,int y,uint8_t r,uint8_t green,uint8_t b)
{
    return (BtnVertex){x,y,0,0,r,green,b};
}
bool btn_graphics_test_scene(BtnGraphics *g, BtnDrawList *list, unsigned tick, uint32_t *gte_flags)
{
    static const int16_t sine[16]={0,1567,2896,3784,4096,3784,2896,1567,0,-1567,-2896,-3784,-4096,-3784,-2896,-1567};
    static const BtnVector model[8]={{-65,-65,-65},{65,-65,-65},{-65,65,-65},{65,65,-65},
        {-65,-65,65},{65,-65,65},{-65,65,65},{65,65,65}};
    static const unsigned faces[6][4]={{0,1,2,3},{5,4,7,6},{4,0,6,2},{1,5,3,7},{4,5,0,1},{2,3,6,7}};
    btn_draw_list_clear(list);
    BtnPrimitive p={0};
    p.type=BTN_RECTANGLE; p.w=BTN_WIDTH; p.h=BTN_HEIGHT;
    p.v[0]=vertex(0,0,24,32,48);
    if (!btn_draw_list_add(list,1023,&p)) return false;
    p.w=72; p.h=44; p.v[0]=vertex(-12,40,240,64,48);
    if (!btn_draw_list_add(list,900,&p)) return false;
    p.material.semi=true;
    for (unsigned mode=0; mode<4; ++mode) {
        p.material.page=(uint16_t)(mode<<5);
        p.v[0]=vertex(20+(int)mode*72,62,64,192,240);
        if (!btn_draw_list_add(list,890,&p)) return false;
    }
    p=(BtnPrimitive){0}; p.type=BTN_LINE;
    p.v[0]=vertex(-30,215,255,255,0); p.v[1]=vertex(340,110,0,255,255);
    if (!btn_draw_list_add(list,800,&p)) return false;
    BtnGte gte; btn_gte_init(&gte);
    unsigned angle=(tick/4)%16;
    gte.transform.m[0][0]=sine[(angle+4)%16]; gte.transform.m[0][2]=sine[angle];
    gte.transform.m[2][0]=(int16_t)-sine[angle]; gte.transform.m[2][2]=sine[(angle+4)%16];
    gte.transform.t[2]=1100;
    uint32_t flags=0;
    for (unsigned face=0; face<6; ++face) {
        p=(BtnPrimitive){0}; p.type=BTN_QUAD;
        p.material.textured=true; p.material.raw=true; p.material.page=(uint16_t)((2u<<7)|16u|10u);
        for (unsigned n=0; n<4; ++n) {
            btn_gte_rtps(&gte,model[faces[face][n]]); flags|=gte.flag;
            p.v[n]=(BtnVertex){gte.sxy[2].x,gte.sxy[2].y,(n&1u) ? 15 : 0,144+((n&2u) ? 15 : 0),128,128,128};
        }
        /* Face normal uses the first three projected vertices. */
        for (unsigned n=0; n<3; ++n) gte.sxy[n]=(BtnScreenPoint){(int16_t)p.v[n].x,(int16_t)p.v[n].y};
        if (btn_gte_nclip(&gte)<=0) continue;
        unsigned bucket=btn_gte_avsz(&gte,true)/4u;
        if (bucket>=BTN_OT_BUCKETS) bucket=BTN_OT_BUCKETS-1;
        if (!btn_draw_list_add(list,bucket,&p)) return false;
    }
    if (gte_flags) *gte_flags=flags;
    return btn_draw_list_render(g,list);
}
