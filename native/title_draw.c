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
    /* src/ui/window_frame.c: submission is reversed by the ordering table. */
    const int pixels[6][3]={{w-2,0,0xF8DBAF},{w-2,1,0xDEBB84},{w-1,1,0xC89F60},
        {1,h-2,0xE2C08B},{1,h-1,0xC89F60},{0,h-2,0xF8DBAF}};
    const int lines[8][5]={{1,1,w-3,1,0xFDECD2},{1,1,1,h-2,0xFDECD2},
        {w-2,1,w-2,h-2,0xBA8B47},{1,h-2,w-3,h-2,0xBA8B47},
        {1,0,w-2,0,0xFDECD2},{0,1,0,h-2,0xFDECD2},
        {w-1,1,w-1,h-2,0xBA8B47},{1,h-1,w-2,h-1,0xBA8B47}};
    for (unsigned i=0; i<6; ++i) {
        unsigned color=(unsigned)pixels[i][2];
        if (!rectangle(t,2,x+pixels[i][0],y+pixels[i][1],1,1,
            (uint8_t)(color>>16),(uint8_t)(color>>8),(uint8_t)color)) return false;
    }
    for (unsigned i=0; i<8; ++i) {
        unsigned color=(unsigned)lines[i][4];
        BtnPrimitive p={0}; p.type=BTN_LINE;
        p.v[0]=(BtnVertex){x+lines[i][0],y+lines[i][1],0,0,
            (uint8_t)(color>>16),(uint8_t)(color>>8),(uint8_t)color};
        p.v[1]=p.v[0]; p.v[1].x=x+lines[i][2]; p.v[1].y=y+lines[i][3];
        if (!btn_draw_list_add(t->list,2,&p)) return false;
    }
    BtnPrimitive fill={0}; fill.type=BTN_QUAD;
    fill.v[0]=(BtnVertex){x+2,y+2,0,0,244,223,189};
    fill.v[1]=fill.v[0]; fill.v[1].x=x+w-2;
    fill.v[2]=(BtnVertex){x+2,y+h-2,0,0,249,207,143};
    fill.v[3]=fill.v[2]; fill.v[3].x=x+w-2;
    return btn_draw_list_add(t->list,2,&fill);
}

static bool draw_words(BtnTitle *t, const uint16_t *value, size_t count, int x, int y)
{
    const BtnTextStyle style={t->text_clut_y,0,0};
    return btn_text_draw_styled(t->list,1,value,count,x,y,&style,NULL);
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
    return draw_words(t,words,n,x,y);
}

static bool number(BtnTitle *t, int x, int y, unsigned value)
{
    if (value>99) return SDL_SetError("Title UI number exceeds two digits");
    char digits[3]={(char)('0'+value/10),(char)('0'+value%10),0};
    return text(t,x,y,digits);
}

static bool icon(BtnTitle *t, unsigned range, unsigned frame, int x, int y)
{
    BtnAnimationCursor cursor={0}; cursor.range=range; cursor.frame=frame;
    /* name_sound_draw uses OT +0x60 for the hand, +0x64 for season icons/text. */
    return btn_animation_draw(&t->ui_animation,&cursor,t->list,range==11 ? 0u : 1u,x,y);
}

static int cell_x(unsigned cell)
{
    unsigned col=cell%14, gap=(col>=5 ? 1u : 0u)+(col>=10 ? 1u : 0u)+(col>=13 ? 2u : 0u);
    return 50+(int)(col+gap)*9;
}

static bool name_grid(BtnTitle *t)
{
    BtnTitleSetup *s=&t->setup;
    if (!panel(t,171,32,133,18)) return false;
    if (s->stage==BTN_SETUP_FARM) {
        if (!draw_words(t,btn_setup_name(s),9,173,35) || !text(t,245,35,"Farm")) return false;
    } else if (!text(t,173,35,"Name") || !draw_words(t,btn_setup_name(s),9,213,35)) return false;
    if (s->confirming) return true;
    if (!panel(t,45,64,230,86)) return false;
    const BtnTextStyle style={t->text_clut_y,1,2};
    if (!btn_text_draw_styled(t->list,1,t->name_captions[s->shift],128,47,66,&style,NULL)) return false;
    return icon(t,11,1,cell_x(s->cell),72+(int)(s->cell/14)*14)
        && panel(t,26,164,267,62) && text(t,30,168,"Arrows: Move cursor")
        && text(t,30,182,"Z: Input   X: Erase / Back")
        && text(t,30,196,"Enter / OK: Confirm name")
        && text(t,30,210,"Mark / alphabet: Switch keys");
}

static bool yes_no(BtnTitle *t, const uint16_t *prompt, unsigned choice)
{
    /* name_prompt_draw: choice window, text and cursor use distinct OT slots. */
    return panel(t,26,164,267,62) && panel(t,145,76,40,30)
        && draw_words(t,t->yes_no,16,147,78) && draw_words(t,prompt,64,30,168)
        && icon(t,11,1,153,84+(int)choice*12);
}

static bool calendar(BtnTitle *t)
{
    const char *seasons[4]={"Spring","Summer","Fall","Winter"};
    BtnTitleSetup *s=&t->setup;
    if (!panel(t,171,32,133,18) || !text(t,173,35,"Birthday")) return false;
    if (s->stage!=BTN_SETUP_SEASON && !icon(t,43,s->game.season,253,35)) return false;
    if (s->stage==BTN_SETUP_BIRTHDAY_CONFIRM) return number(t,277,35,s->game.day+1);
    if (!panel(t,20,56,279,103) || !panel(t,26,164,267,62)
        || !text(t,30,168,"Arrows: Move cursor") || !text(t,30,182,"Z: Select   X: Back")) return false;
    for (unsigned i=0; i<4; ++i)
        if (!icon(t,43,i,30+(int)i*64,60) || !text(t,42+(int)i*64,60,seasons[i])) return false;
    for (unsigned i=0; i<30; ++i) if (!number(t,30+(int)(i%7)*39,88+(int)(i/7)*14,i+1)) return false;
    if (s->stage==BTN_SETUP_SEASON)
        return icon(t,11,1,36+(int)s->value*64,66);
    return icon(t,11,1,36+(int)(s->value%7)*39,94+(int)(s->value/7)*14);
}

static bool review(BtnTitle *t)
{
    const char *seasons[4]={"Spring","Summer","Fall","Winter"};
    const BtnNewGame *g=&t->setup.game;
    return panel(t,28,44,264,112) && text(t,40,52,"Player")
        && draw_words(t,g->player,9,152,52)
        && text(t,40,76,"Birthday") && text(t,152,76,seasons[g->season])
        && number(t,216,76,g->day+1)
        && text(t,40,100,"Farm") && draw_words(t,g->farm,9,152,100)
        && text(t,40,124,"Dog") && draw_words(t,g->dog,9,152,124);
}

static bool setup_draw(BtnTitle *t)
{
    BtnTitleSetup *s=&t->setup;
    if (s->stage==BTN_SETUP_PLAYER || s->stage==BTN_SETUP_FARM || s->stage==BTN_SETUP_DOG)
        return name_grid(t) && (!s->confirming || yes_no(t,t->setup_prompts[0],s->choice));
    if (s->stage==BTN_SETUP_SEASON || s->stage==BTN_SETUP_DAY || s->stage==BTN_SETUP_BIRTHDAY_CONFIRM)
        return calendar(t) && (s->stage!=BTN_SETUP_BIRTHDAY_CONFIRM || yes_no(t,t->setup_prompts[1],s->choice));
    if (!review(t)) return false;
    if (s->stage==BTN_SETUP_DONE) return true;
    /* The typed-handoff review is native UI, outside name_prompt_draw. */
    return panel(t,28,164,264,52) && text(t,40,170,"Begin this game?")
        && text(t,94,192,"Yes") && text(t,182,192,"No")
        && icon(t,11,1,s->choice ? 180 : 92,198);
}

static bool scenery(BtnTitle *t)
{
    static const int positions[14][2]={{282,173},{33,146},{51,182},{295,205},{258,138},{280,121},
        {63,149},{10,188},{9,214},{295,52},{294,30},{23,30},{6,109},{36,223}};
    if (!sprite(t,29,6,0,0)) return false;
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
            if (!sprite(t,37,5,x[col],y+(int)row*240)) return false;
    }
    for (unsigned i=0; i<14; ++i)
        if (!btn_animation_draw(&t->animation,&t->actor_frames[i],t->list,4,positions[i][0],positions[i][1])) return false;
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
        /* name_entry_step/name_sound_step draw a black semitransparent tile. */
        BtnPrimitive dim={0}; dim.type=BTN_RECTANGLE; dim.w=BTN_WIDTH; dim.h=BTN_HEIGHT;
        dim.material.semi=true;
        if (!btn_draw_list_add(t->list,3,&dim)) return false;
        if (!setup_draw(t)) return false;
        if (s->page==BTN_TITLE_READY && (!panel(t,28,164,264,52)
            || !text(t,40,170,"Setup complete") || !text(t,40,184,"Gameplay requires phase 06")
            || !text(t,40,198,"X: Return to title"))) return false;
    } else {
        int logo=s->page==BTN_TITLE_INTRO ? s->logo_y : 0;
        if (!sprite(t,31,3,0,logo)) return false;
        if (s->page!=BTN_TITLE_INTRO || s->intro_stage>=2)
            if (!sprite(t,32,3,0,s->page==BTN_TITLE_INTRO ? s->copyright_y : 0)) return false;
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
