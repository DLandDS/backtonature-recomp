#include "animation.h"
#include "graphics_scene.h"
#include "image_bank.h"
#include <SDL3/SDL_error.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr,"line %d: %s (%s)\n",__LINE__,#c,SDL_GetError()); exit(1); } } while (0)
static void le16(uint8_t *p,unsigned n) { p[0]=(uint8_t)n; p[1]=(uint8_t)(n>>8); }
static void le32(uint8_t *p,uint32_t n) { for(unsigned i=0;i<4;++i) p[i]=(uint8_t)(n>>(i*8)); }
static size_t bank_fixture(uint8_t *p,unsigned flags,const uint8_t *payload,unsigned length)
{
    memset(p,0,64); p[0]=13; le16(p+2,1); le32(p+4,(flags<<24)|length);
    le16(p+8,900); le16(p+10,300); le16(p+12,2); le16(p+14,2);
    memcpy(p+16,payload,length);
    return 16+((length+3u)&~3u);
}
static void decoder(BtnGraphics *g)
{
    const uint8_t raw[]={1,2,3,4,5,6,7,8}, row[]={248,1,2,3,4,5,6,7,8};
    const uint8_t column[]={248,1,5,2,6,3,7,4,8};
    uint8_t storage[65],*p=storage+1; BtnImageBank bank; uint16_t out[4];
    const uint8_t *payloads[]={raw,row,column}; unsigned lengths[]={8,9,9},flags[]={4,0,8};
    for(unsigned i=0;i<3;++i) {
        size_t size=bank_fixture(p,flags[i],payloads[i],lengths[i]);
        CHECK(btn_image_bank_parse(&bank,p,size)); CHECK(bank.count==1 && bank.consumed==size);
        CHECK(btn_image_bank_upload(g,&bank,0,0,0,0,0));
        CHECK(btn_vram_read(g,(BtnRect){900,300,2,2},out,4));
        CHECK(out[0]==0x0201 && out[1]==0x0403 && out[2]==0x0605 && out[3]==0x0807);
        for(size_t n=0;n<size;++n) CHECK(!btn_image_bank_parse(&bank,p,n));
    }
    size_t size=bank_fixture(p,6,raw,8);
    CHECK(btn_image_bank_parse(&bank,p,size)); CHECK(btn_image_bank_upload(g,&bank,0,0,0,0,1));
    CHECK(btn_vram_read(g,(BtnRect){900,300,2,2},out,4));
    CHECK(out[0]==0x0201 && out[1]==0x8403 && out[2]==0x0605 && out[3]==0x8807);
    const uint8_t repeat[]={8,0xAB};
    size=bank_fixture(p,0,repeat,2); CHECK(btn_image_bank_parse(&bank,p,size));
    CHECK(btn_image_bank_upload(g,&bank,-10,-10,0,0,0));
    CHECK(btn_vram_read(g,(BtnRect){890,290,2,2},out,4) && out[3]==0xABAB);
    const uint8_t zero[]={0,1,0,2,0,3,0,4,0,5,0,6,0,7,0,8};
    size=bank_fixture(p,0,zero,16); CHECK(btn_image_bank_parse(&bank,p,size));
    CHECK(btn_image_bank_upload(g,&bank,0,0,0,0,0));
    CHECK(btn_vram_read(g,(BtnRect){900,300,2,2},out,4) && out[3]==0x0807);
    uint16_t before=g->words[300*1024+900];
    le32(p+4,1); p[16]=8; CHECK(btn_image_bank_parse(&bank,p,20));
    CHECK(!btn_image_bank_upload(g,&bank,0,0,0,0,0) && g->words[300*1024+900]==before);
    CHECK(!btn_image_bank_upload(g,&bank,1024,0,0,0,0));
    CHECK(!btn_image_bank_block(&bank,1,&(BtnImageBlock){0}));
    /* -128 is a 128-byte literal, including on hosts where char is unsigned. */
    uint8_t big[148]={13,0,1,0}; le32(big+4,129); le16(big+8,800); le16(big+10,300);
    le16(big+12,64); le16(big+14,1); big[16]=128;
    for(unsigned i=0;i<128;++i) big[17+i]=(uint8_t)i;
    CHECK(btn_image_bank_parse(&bank,big,sizeof(big))); CHECK(btn_image_bank_upload(g,&bank,0,0,0,0,0));
    CHECK(g->words[300*1024+863]==0x7F7E);
}
static BtnPrimitive rectangle(int x,int y,int w,int h,uint8_t r,uint8_t green,uint8_t b)
{
    BtnPrimitive p={0}; p.type=BTN_RECTANGLE; p.w=w; p.h=h;
    p.v[0]=(BtnVertex){x,y,0,0,r,green,b}; return p;
}
static void transfers(BtnGraphics *g)
{
    uint16_t data[]={1,2,3,4,5,6},out[6]={0};
    CHECK(btn_vram_upload(g,(BtnRect){10,10,3,2},data,6));
    CHECK(btn_vram_copy(g,(BtnRect){10,10,3,2},11,11));
    CHECK(btn_vram_read(g,(BtnRect){11,11,3,2},out,6) && !memcmp(out,data,sizeof(out)));
    CHECK(!btn_vram_upload(g,(BtnRect){1023,511,2,1},data,6));
    CHECK(!btn_vram_read(g,(BtnRect){INT_MAX,0,1,1},out,6));
    CHECK(!btn_vram_read(g,(BtnRect){0,0,3,2},out,5));
    CHECK(!btn_vram_copy(g,(BtnRect){0,0,1,1},-1,0));
}
static void pixels(BtnGraphics *g)
{
    btn_graphics_init(g);
    BtnPrimitive p=rectangle(-1,-1,3,3,255,0,0);
    CHECK(btn_draw(g,&p)); CHECK(g->words[0]==31 && g->words[1025]==31 && !g->words[2]);
    g->offset_x=10; CHECK(btn_draw(g,&p)); CHECK(g->words[9]==31); g->offset_x=0;
    for(unsigned mode=0;mode<4;++mode) {
        g->words[0]=31; p=rectangle(0,0,1,1,248,0,0); p.material.semi=true; p.material.page=(uint16_t)(mode<<5);
        CHECK(btn_draw(g,&p)); const unsigned expected[]={30,31,0,31}; CHECK(g->words[0]==expected[mode]);
    }
    g->set_mask=true; p=rectangle(0,0,1,1,255,0,0); CHECK(btn_draw(g,&p)); CHECK(g->words[0]==0x801F);
    g->set_mask=false; g->check_mask=true; p.v[0].g=255; CHECK(btn_draw(g,&p)); CHECK(g->words[0]==0x801F);
    g->check_mask=false;
    /* Index zero is transparent only when its CLUT value is zero. */
    g->words[256*1024+640]=0x0021; g->words[500*1024+768]=0x001F;
    g->words[500*1024+769]=0; g->words[500*1024+770]=0x8000;
    p=rectangle(0,0,3,1,128,128,128); p.material.textured=p.material.raw=true;
    p.material.page=26; p.material.clut=(uint16_t)((500u<<6)|48u);
    g->words[0]=7; g->words[1]=7; g->words[2]=7;
    CHECK(btn_draw(g,&p)); CHECK(g->words[0]==7 && g->words[1]==0x8000 && g->words[2]==31);
    p.material.page=(uint16_t)(26u|128u); g->words[256*1024+640]=0x0002;
    CHECK(btn_texture_sample(g,&p.material,0,0)==0x8000);
    p.material.page=(uint16_t)(26u|256u); CHECK(btn_texture_sample(g,&p.material,0,0)==2);
    g->words[256*1024+648]=123; p.material.mask_x=1; p.material.offset_x=1;
    CHECK(btn_texture_sample(g,&p.material,0,0)==123);
    p.material.page=(uint16_t)(26u|384u); CHECK(!btn_draw(g,&p));
    CHECK(btn_color_xrgb(0x7FFF)==0xFFFFFF);
}
static void ordering(BtnGraphics *g,BtnDrawList *list)
{
    btn_graphics_init(g); btn_draw_list_clear(list);
    BtnPrimitive red=rectangle(0,0,2,2,255,0,0),green=rectangle(0,0,2,2,0,255,0);
    CHECK(btn_draw_list_add(list,0,&red)); CHECK(btn_draw_list_add(list,0,&green));
    CHECK(btn_draw_list_render(g,list) && g->words[0]==31);
    btn_draw_list_clear(list); CHECK(btn_draw_list_add(list,1,&green)); CHECK(btn_draw_list_add(list,0,&red));
    CHECK(btn_draw_list_render(g,list) && g->words[0]==31);
    list->nodes[1].next=1; CHECK(!btn_draw_list_render(g,list));
    CHECK(!btn_draw_list_add(list,BTN_OT_BUCKETS,&red));
    btn_draw_list_clear(list); list->count=BTN_DRAW_CAPACITY; CHECK(!btn_draw_list_add(list,0,&red));
    btn_graphics_init(g);
    BtnPrimitive quad={0}; quad.type=BTN_QUAD; quad.material.semi=true; quad.material.page=32;
    quad.v[0]=(BtnVertex){0,0,0,0,8,0,0}; quad.v[1]=(BtnVertex){4,0,0,0,8,0,0};
    quad.v[2]=(BtnVertex){0,4,0,0,8,0,0}; quad.v[3]=(BtnVertex){4,4,0,0,8,0,0};
    CHECK(btn_draw(g,&quad));
    unsigned filled=0; for(unsigned y=0;y<5;++y) for(unsigned x=0;x<5;++x) {
        uint16_t pixel=g->words[y*1024+x]; CHECK(pixel<=1); if(pixel) ++filled;
    }
    CHECK(filled==16); /* Shared edge writes once, no additive seam. */
}
static void geometry(void)
{
    BtnGte g; btn_gte_init(&g);
    const BtnVector v[3]={{0,0,1000},{100,0,1000},{0,100,1000}};
    btn_gte_rtpt(&g,v); CHECK(g.flag==0 && g.sxy[0].x==160 && g.sxy[1].x==260 && g.sxy[2].y==220);
    CHECK(btn_gte_nclip(&g)==10000); CHECK(btn_gte_avsz(&g,false)==999);
    g.zsf3=4096; CHECK(btn_gte_avsz(&g,false)==3000);
    uint32_t flag=0; CHECK(btn_gte_divide(1000,1000,&flag)==65536 && !flag);
    CHECK(btn_gte_divide(1000,500,&flag)==0x1FFFF && (flag&0x80020000u)==0x80020000u);
    flag=0; CHECK(btn_gte_divide(0,0,&flag)==0x1FFFF && flag);
    flag=0; CHECK(btn_gte_divide(0xFE3F,0x7F20,&flag)==0x1FFFF && !flag);
    btn_gte_rtps(&g,(BtnVector){32767,-32768,0}); CHECK(g.sxy[2].x==1023 && g.sxy[2].y==-1024 && g.flag>>31);
    btn_gte_rtps(&g,(BtnVector){0,0,-1}); CHECK(g.sz[3]==0 && (g.flag&(1u<<18)));
    BtnMatrix matrix={{{4096,0,0},{0,4096,0},{0,0,4096}},{0,0,0}};
    btn_gte_mvmva(&g,&matrix,(BtnVector){-1,2,-3},true,false);
    CHECK(g.ir[0]==-1 && g.ir[1]==2 && g.ir[2]==-3 && !g.flag);
    btn_gte_mvmva(&g,&matrix,(BtnVector){-1,2,-3},true,true); CHECK(g.ir[0]==0 && (g.flag&(1u<<24)));
    matrix.m[0][0]=1;
    btn_gte_mvmva(&g,&matrix,(BtnVector){-1,0,0},true,false); CHECK(g.mac[1]==-1);
    matrix.t[0]=INT32_MAX; matrix.m[0][0]=32767;
    btn_gte_mvmva(&g,&matrix,(BtnVector){32767,0,0},true,false); CHECK(g.flag&(1u<<30));
    g.sz[0]=g.sz[1]=g.sz[2]=g.sz[3]=65535; g.zsf4=32767;
    CHECK(btn_gte_avsz(&g,true)==65535 && (g.flag&(1u<<18)) && (g.flag&(1u<<16)));
    g.zsf4=-1; CHECK(btn_gte_avsz(&g,true)==0 && (g.flag&(1u<<18)));
}
static void animations(BtnGraphics *g,BtnDrawList *list)
{
    uint8_t data[102]={0}; le32(data,14); le16(data+4,2); le16(data+6,1); le16(data+8,1); le16(data+10,1); le32(data+12,2);
    const unsigned offsets[]={36,52,56,68,72}; for(unsigned i=0;i<5;++i) le32(data+16+i*4,offsets[i]);
    le16(data+36,896); le16(data+38,0); le16(data+40,2); le16(data+42,2);
    le16(data+44,896); le16(data+46,255); le16(data+48,16); le16(data+50,1);
    le16(data+52,1); le16(data+58,1); le16(data+68,2); data[78]=4; data[88]=2; data[89]=255;
    BtnAnimation a; CHECK(btn_animation_parse(&a,data,sizeof(data)));
    BtnAnimationCursor c={0}; CHECK(btn_animation_tick(&a,&c) && c.frame==0 && c.ticks==1);
    CHECK(btn_animation_tick(&a,&c) && c.frame==1); CHECK(btn_animation_tick(&a,&c) && c.frame==1 && c.ended);
    c=(BtnAnimationCursor){0}; data[79]=0x81;
    CHECK(btn_animation_parse(&a,data,sizeof(data))); CHECK(btn_animation_tick(&a,&c));
    CHECK(btn_animation_tick(&a,&c) && c.frame==1 && c.jumped);
    btn_draw_list_clear(list); CHECK(btn_animation_draw(&a,&c,list,0,10,10));
    CHECK(list->count==1 && list->nodes[0].primitive.w==8);
    CHECK(btn_draw_list_render(g,list)); c.range=1; CHECK(!btn_animation_tick(&a,&c));
    data[79]=0x84; CHECK(!btn_animation_parse(&a,data,sizeof(data))); data[79]=0;
    le16(data+56,2); CHECK(!btn_animation_parse(&a,data,sizeof(data))); le16(data+56,0);
    for(size_t n=0;n<92;++n) CHECK(!btn_animation_parse(&a,data,n));
    const uint16_t text[]={0,0xFFD,1,0xFFFE,2,0xFFFF}; size_t consumed=0;
    btn_draw_list_clear(list); CHECK(btn_text_draw(list,0,text,6,8,8,&consumed));
    CHECK(consumed==6 && list->count==3);
    CHECK(list->nodes[0].primitive.material.clut==(uint16_t)((254u<<6)|56u));
    CHECK(list->nodes[0].primitive.v[0].u==0 && list->nodes[1].primitive.v[0].x==24);
    CHECK(list->nodes[2].primitive.v[0].y==20);
    CHECK(!btn_text_draw(list,0,text,5,8,8,NULL));
    const uint16_t unsupported[]={0x400A,0xFFFF}; CHECK(!btn_text_draw(list,0,unsupported,2,8,8,NULL));
}
int main(void)
{
    BtnGraphics *g=calloc(1,sizeof(*g)); BtnDrawList *list=calloc(1,sizeof(*list)); CHECK(g&&list);
    btn_graphics_init(g); decoder(g); transfers(g); pixels(g); ordering(g,list); geometry(); animations(g,list);
    btn_graphics_init(g); btn_graphics_test_texture(g); uint32_t flags;
    CHECK(btn_graphics_test_scene(g,list,0,&flags) && !flags);
    uint32_t *pixels_out=malloc(BTN_WIDTH*BTN_HEIGHT*sizeof(*pixels_out)); CHECK(pixels_out);
    CHECK(btn_framebuffer(g,(BtnRect){0,0,BTN_WIDTH,BTN_HEIGHT},pixels_out,BTN_WIDTH*BTN_HEIGHT));
    CHECK(!btn_framebuffer(g,(BtnRect){0,0,BTN_WIDTH,BTN_HEIGHT},pixels_out,1));
    free(pixels_out); free(list); free(g); puts("native-graphics: passed"); return 0;
}
