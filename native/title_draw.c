#include "title.h"
#include <SDL3/SDL.h>

static bool sprite(BtnTitle *t, unsigned range, unsigned bucket, int x, int y)
{
    BtnAnimationCursor cursor={0}; cursor.range=range;
    return btn_animation_draw(&t->animation,&cursor,t->list,bucket,x,y);
}

static bool rectangle(BtnTitle *t, unsigned bucket, int x, int y, int w, int h,
    uint8_t r, uint8_t g, uint8_t b)
{
    BtnPrimitive p={0}; p.type=BTN_RECTANGLE; p.w=w; p.h=h;
    p.v[0]=(BtnVertex){x,y,0,0,r,g,b};
    return btn_draw_list_add(t->list,bucket,&p);
}

static bool panel(BtnTitle *t, int x, int y, int w, int h)
{
    /* Text is bucket 0; the inner fill precedes the outer border in a
     * head-inserted bucket so the border cannot paint over the panel. */
    return rectangle(t,1,x+2,y+2,w-4,h-4,24,40,72)
        && rectangle(t,1,x,y,w,h,200,216,232);
}

static uint16_t glyph(char ch)
{
    if (ch>='A' && ch<='Z') return (uint16_t)(ch-'A');
    if (ch>='a' && ch<='z') return (uint16_t)(ch-'a'+0x20);
    if (ch>='0' && ch<='9') return (uint16_t)(ch-'0'+0x40);
    switch (ch) {
    case ' ': return 0xFFD;
    case ':': return 0x66;
    case '-': return 0x69;
    case '?': return 0x62;
    case '.': return 0x6C;
    case '/': return 0x71;
    default: return 0xFFFF;
    }
}

static bool text(BtnTitle *t, int x, int y, const char *value)
{
    uint16_t words[64]; unsigned n=0;
    while (*value && n<63) {
        words[n]=glyph(*value++);
        if (words[n++]==0xFFFF) return SDL_SetError("Unsupported title UI character");
    }
    if (*value) return SDL_SetError("Title UI label exceeds its bound");
    words[n++]=0xFFFF;
    return btn_text_draw(t->list,0,words,n,x,y,NULL);
}

static bool number(BtnTitle *t, int x, int y, unsigned value)
{
    if (value>99) return SDL_SetError("Title UI number exceeds two digits");
    char digits[3]={(char)('0'+value/10),(char)('0'+value%10),0};
    return text(t,x,y,digits);
}

static bool mark(BtnTitle *t, int x, int y, int w, int h)
{
    return rectangle(t,0,x-1,y-1,w+2,1,255,216,64)
        && rectangle(t,0,x-1,y+h,w+2,1,255,216,64)
        && rectangle(t,0,x-1,y,1,h,255,216,64)
        && rectangle(t,0,x+w,y,1,h,255,216,64);
}

static int cell_x(unsigned cell)
{
    unsigned col=cell%14, gap=(col>=5 ? 1u : 0u)+(col>=10 ? 1u : 0u)+(col>=13 ? 2u : 0u);
    return 50+(int)(col+gap)*9;
}

static bool name_grid(BtnTitle *t)
{
    BtnTitleSetup *s=&t->setup;
    const char *label=s->stage==BTN_SETUP_PLAYER ? "Your name" : s->stage==BTN_SETUP_FARM ? "Farm name" : "Dog name";
    if (!panel(t,20,18,280,28) || !panel(t,42,54,238,98)
        || !text(t,28,26,label)
        || !btn_text_draw(t->list,0,btn_setup_name(s),9,176,26,NULL)) return false;
    for (unsigned cell=0; cell<84; ++cell) {
        int x=cell_x(cell), y=66+(int)(cell/14)*14;
        if (cell==13) { if (!text(t,x,y,"Shift")) return false; }
        else if (cell==27) { if (!text(t,x,y,"Space")) return false; }
        else if (cell==83) { if (!text(t,x,y,"End")) return false; }
        else {
            uint16_t words[2]={btn_name_character(&t->name_layout,cell,s->shift),0xFFFF};
            if (words[0]!=0xFFFF && !btn_text_draw(t->list,0,words,2,x,y,NULL)) return false;
        }
    }
    unsigned width=s->cell==13 || s->cell==27 ? 40u : s->cell==83 ? 24u : 8u;
    return (s->confirming || mark(t,cell_x(s->cell),66+(int)(s->cell/14)*14,(int)width,12))
        && text(t,24,218,"Z: choose  X: erase  Enter: End");
}

static bool yes_no(BtnTitle *t, const char *label, unsigned choice)
{
    return panel(t,62,160,196,52) && text(t,70,166,label)
        && text(t,94,192,"Yes") && text(t,182,192,"No")
        && mark(t,choice ? 182 : 94,192,choice ? 16 : 24,12);
}

static bool calendar(BtnTitle *t)
{
    const char *seasons[4]={"Spring","Summer","Fall","Winter"};
    BtnTitleSetup *s=&t->setup;
    if (!panel(t,20,46,280,110) || !text(t,48,26,"Your birthday")) return false;
    for (unsigned i=0; i<4; ++i) if (!text(t,28+(int)i*64,54,seasons[i])) return false;
    for (unsigned i=0; i<30; ++i) if (!number(t,32+(int)(i%7)*38,78+(int)(i/7)*14,i+1)) return false;
    if (s->stage==BTN_SETUP_SEASON)
        return mark(t,28+(int)s->value*64,54,(int)SDL_strlen(seasons[s->value])*8,12);
    if (s->stage==BTN_SETUP_DAY) return mark(t,32+(int)(s->value%7)*38,78+(int)(s->value/7)*14,16,12);
    return mark(t,28+(int)s->game.season*64,54,(int)SDL_strlen(seasons[s->game.season])*8,12)
        && mark(t,32+(int)(s->game.day%7)*38,78+(int)(s->game.day/7)*14,16,12);
}

static bool review(BtnTitle *t)
{
    const char *seasons[4]={"Spring","Summer","Fall","Winter"};
    const BtnNewGame *g=&t->setup.game;
    return panel(t,28,44,264,112) && text(t,40,52,"Player")
        && btn_text_draw(t->list,0,g->player,9,152,52,NULL)
        && text(t,40,76,"Birthday") && text(t,152,76,seasons[g->season])
        && number(t,216,76,g->day+1)
        && text(t,40,100,"Farm") && btn_text_draw(t->list,0,g->farm,9,152,100,NULL)
        && text(t,40,124,"Dog") && btn_text_draw(t->list,0,g->dog,9,152,124,NULL);
}

static bool setup_draw(BtnTitle *t)
{
    BtnTitleSetup *s=&t->setup;
    if (s->stage==BTN_SETUP_PLAYER || s->stage==BTN_SETUP_FARM || s->stage==BTN_SETUP_DOG)
        return name_grid(t) && (!s->confirming || yes_no(t,"Is this name OK?",s->choice));
    if (s->stage==BTN_SETUP_SEASON || s->stage==BTN_SETUP_DAY || s->stage==BTN_SETUP_BIRTHDAY_CONFIRM)
        return calendar(t) && (s->stage!=BTN_SETUP_BIRTHDAY_CONFIRM || yes_no(t,"Birthday OK?",s->choice));
    return review(t) && (s->stage==BTN_SETUP_DONE || yes_no(t,"Begin this game?",s->choice));
}

static bool scenery(BtnTitle *t)
{
    static const int positions[14][2]={{282,173},{33,146},{51,182},{295,205},{258,138},{280,121},
        {63,149},{10,188},{9,214},{295,52},{294,30},{23,30},{6,109},{36,223}};
    if (!sprite(t,29,5,0,0)) return false;
    /* 800B944C: two 3x3 tiled layers. SDK ccos/csin quantization remains
     * a PS1-comparison gate; host trig preserves the audited motion geometry. */
    for (unsigned layer=0; layer<2; ++layer) {
        unsigned phase=t->background_frame[layer];
        int y=(int)(phase%240)*(layer ? 1 : -1);
        double angle=(double)t->background_frame[1]*4096.0/120.0;
        int trig=(int)SDL_round((layer ? SDL_sin(angle*6.283185307179586/4096.0)
            : SDL_cos(angle*6.283185307179586/4096.0))*4096.0);
        int horizontal=320-(int)(phase/240)*80+trig*12/4096;
        int x[3]={y-horizontal,layer ? y+horizontal-320 : y-horizontal+320,
            layer ? y+horizontal-640 : y-horizontal+640};
        for (unsigned row=0; row<3; ++row) for (unsigned col=0; col<3; ++col)
            if (!sprite(t,37,4,x[col],y+(int)row*240)) return false;
    }
    for (unsigned i=0; i<14; ++i)
        if (!btn_animation_draw(&t->animation,&t->actor_frames[i],t->list,3,positions[i][0],positions[i][1])) return false;
    return true;
}

bool btn_title_draw(BtnTitle *t, uint32_t *pixels, size_t count)
{
    if (!t || !t->assets || !pixels || count<BTN_WIDTH*BTN_HEIGHT)
        return SDL_SetError("Title is not loaded or framebuffer is too small");
    *t->graphics=*t->assets;
    BtnPrimitive back={0}; back.type=BTN_RECTANGLE; back.w=BTN_WIDTH; back.h=BTN_HEIGHT;
    if (!btn_draw(t->graphics,&back)) return false;
    btn_draw_list_clear(t->list);
    BtnTitleState *s=&t->state;
    if (!scenery(t)) return false;
    if (s->page==BTN_TITLE_SETUP || s->page==BTN_TITLE_READY) {
        if (!setup_draw(t)) return false;
        if (s->page==BTN_TITLE_READY && (!panel(t,28,164,264,52)
            || !text(t,40,170,"Setup complete") || !text(t,40,184,"Gameplay requires phase 06")
            || !text(t,40,198,"X: Return to title"))) return false;
    } else {
        int logo=s->page==BTN_TITLE_INTRO ? s->logo_y : 0;
        if (!sprite(t,31,2,0,logo)) return false;
        if (s->page!=BTN_TITLE_INTRO || s->intro_stage>=2)
            if (!sprite(t,32,2,0,s->page==BTN_TITLE_INTRO ? s->copyright_y : 0)) return false;
        if (s->page==BTN_TITLE_INTRO) {
            if (s->intro_stage>=3 && (s->intro_stage!=4 || s->intro_ticks%30<15))
                if (!sprite(t,33,0,s->start_x,0)) return false;
        } else if (s->page==BTN_TITLE_PROMPT) {
            if (!sprite(t,33,0,0,0)) return false;
        } else if (!sprite(t,36,0,s->ticks%30<15 ? 100 : 104,s->selection ? 179 : 155)
            || !sprite(t,35,0,0,0) || !sprite(t,34,0,0,0)) return false;
        if (s->page==BTN_TITLE_ATTRACT && (!panel(t,16,164,288,52)
            || !text(t,24,170,"Attract demo requires gameplay")
            || !text(t,24,190,"Enter: Return to title"))) return false;
    }
    if (!btn_draw_list_render(t->graphics,t->list)) return false;
    if (s->fade_level && s->fade_length) {
        unsigned level=(UINT32_C(0xFF000)/s->fade_length*s->fade_level)/4096;
        uint8_t value=(uint8_t)(level>255 ? 255 : level);
        back.v[0].r=back.v[0].g=back.v[0].b=value;
        back.material.semi=true; back.material.page=2u<<5;
        if (!btn_draw(t->graphics,&back)) return false;
    }
    return btn_framebuffer(t->graphics,(BtnRect){0,0,BTN_WIDTH,BTN_HEIGHT},pixels,count);
}
