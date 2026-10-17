#include "title.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>

static unsigned le16(const uint8_t *p) { return (unsigned)p[0] | ((unsigned)p[1]<<8); }

void btn_title_reset(BtnTitleState *s)
{
    *s=(BtnTitleState){BTN_TITLE_PROMPT,1,0,0};
}

int btn_title_step(BtnTitleState *s, const BtnInput *input)
{
    ++s->ticks; ++s->page_ticks;
    if (s->page==BTN_TITLE_UNAVAILABLE) {
        if (input->pressed & (BTN_BIT(BTN_B)|BTN_BIT(BTN_START))) {
            s->page=BTN_TITLE_MENU; s->page_ticks=0;
        }
        return -1;
    }
    if (s->page==BTN_TITLE_PROMPT) {
        /* 800B8664: Start (0x800) opens the two-choice menu. */
        if (input->pressed & BTN_BIT(BTN_START)) {
            s->page=BTN_TITLE_MENU; s->selection=1; s->page_ticks=0; return 0;
        }
        return -1;
    }
    /* 800B86D0: up/down/select toggle once per pressed edge, before confirm.
     * B returning to Start is a native preview control, absent from that branch. */
    if (input->pressed & BTN_BIT(BTN_B)) { s->page=BTN_TITLE_PROMPT; s->page_ticks=0; return -1; }
    if (input->pressed & (BTN_BIT(BTN_UP)|BTN_BIT(BTN_DOWN))) { s->selection^=1; return 5; }
    if (input->pressed & (BTN_BIT(BTN_A)|BTN_BIT(BTN_START))) {
        s->page=BTN_TITLE_UNAVAILABLE; s->page_ticks=0; return 0;
    }
    /* The retail idle branch enters attract mode after 1800 ticks. Its assets
     * and random scene variants are not ported; do not pretend to run a demo. */
    return -1;
}

const char *btn_title_notice(const BtnTitleState *s)
{
    if (s->page!=BTN_TITLE_UNAVAILABLE) return "PARTIAL TITLE / intro and scenery pending";
    return s->selection ? "Continue unavailable: save/load requires phase 07"
        : "New Game unavailable: name entry/gameplay pending";
}

/* Read fixed US table addresses as bounded file offsets, never host pointers.
 * Full input hashes belong to the independent audit; these structural guards
 * reject incompatible layouts without executing any executable/overlay bytes. */
static bool exe_read(SDL_IOStream *file, uint32_t address, void *out, size_t size)
{
    uint8_t h[32];
    if (SDL_SeekIO(file,0,SDL_IO_SEEK_SET)!=0 || SDL_ReadIO(file,h,sizeof(h))!=sizeof(h)
        || memcmp(h,"PS-X EXE",8)) return SDL_SetError("Expected US PS-X EXE tables");
    uint32_t load=btn_read_le32(h+24), length=btn_read_le32(h+28);
    Sint64 total=SDL_GetIOSize(file);
    if (total<2048 || length>(uint64_t)(total-2048) || address<load
        || size>UINT32_MAX || !btn_span_valid(length,address-load,(uint32_t)size))
        return SDL_SetError("Title executable table exceeds loaded image");
    Sint64 offset=2048+(Sint64)(address-load);
    return (SDL_SeekIO(file,offset,SDL_IO_SEEK_SET)==offset && SDL_ReadIO(file,out,size)==size)
        ? true : SDL_SetError("Cannot read title executable table");
}

static bool read_sound(BtnArchive *archive, SDL_IOStream *exe, unsigned index,
    bool sequence, uint8_t **owned, size_t *size)
{
    uint8_t table[12];
    if (!exe_read(exe,UINT32_C(0x80049370)+index*8,table,sizeof(table))) return false;
    uint32_t start=btn_read_le32(table), body=btn_read_le32(table+4), end=btn_read_le32(table+8), absolute;
    if (end<=start || end-start>16u*1024u*1024u || (sequence ? body!=1 : body<=start || body>=end)
        || !btn_archive_range(&archive->index,9,start,end-start,&absolute))
        return SDL_SetError("Invalid title sound resource %u",index);
    uint8_t *data=malloc(end-start);
    if (!data) return SDL_SetError("Cannot allocate title sound resource");
    if (!btn_archive_read(archive,9,start,end-start,data,end-start)) { free(data); return false; }
    *size=end-start;
    if (!sequence) {
        if (*size<32 || memcmp(data,"pBAV",4)) { free(data); return SDL_SetError("Missing title VAB header"); }
        unsigned programs=le16(data+18);
        size_t header=0x820u+programs*512u+512u;
        if (!programs || programs>128 || header>body-start) {
            free(data); return SDL_SetError("Title VAB header overlaps its body");
        }
        memmove(data+header,data+(body-start),end-body);
        *size=header+(end-body);
    }
    *owned=data; return true;
}

bool btn_title_open(BtnTitle *t, BtnArchive *archive, const char *directory)
{
    if (!t || t->assets || !archive || !archive->bin || !directory)
        return SDL_SetError("Title requires a closed context and an open archive");
    uint8_t entry[20], tables[32];
    /* GF_TITLE header and native entry identify the audited scene layout. */
    if (!btn_archive_read(archive,27,0,4,entry,sizeof(entry)) || btn_read_le32(entry)!=12
        || !btn_archive_read(archive,27,0xCC,20,entry,sizeof(entry))
        || btn_read_le32(entry)!=UINT32_C(0x27BDFFE8)
        || btn_read_le32(entry+16)!=UINT32_C(0x248449F8))
        return SDL_SetError("Title overlay does not match the audited US entry");
    t->assets=malloc(sizeof(*t->assets)); t->graphics=malloc(sizeof(*t->graphics));
    t->list=malloc(sizeof(*t->list));
    if (!t->assets || !t->graphics || !t->list) return SDL_SetError("Cannot allocate title scene");
    btn_graphics_init(t->assets);
    BtnImageBank images;
    if (!btn_animation_read(archive,0,8192,&t->animation_data,&t->animation)) return false;
    if (t->animation.consumed!=3356 || t->animation.ranges!=38)
        return SDL_SetError("Title animation layout differs from the audited input");
    if (!btn_image_bank_read(archive,0,11548,&t->image_data,&images)
        || !btn_image_bank_upload(t->assets,&images,0,0,0,0,0)) return false;
    char *path=NULL;
    if (SDL_asprintf(&path,"%s/SLUS_011.15",directory)<0) return false;
    SDL_IOStream *exe=SDL_IOFromFile(path,"rb"); SDL_free(path);
    if (!exe) return false;
    bool ok=exe_read(exe,UINT32_C(0x80049296)+52,tables,2) && tables[0]==53 && tables[1]==54;
    size_t size=0;
    if (ok) ok=read_sound(archive,exe,53,false,&t->music_data,&size)
        && btn_audio_bank_open(&t->music_bank,t->music_data,size)
        && read_sound(archive,exe,54,true,&t->sequence_data,&size)
        && btn_sequence_open(&t->sequence,t->sequence_data,size,true)
        && read_sound(archive,exe,0,false,&t->effects_data,&size)
        && btn_audio_bank_open(&t->effects_bank,t->effects_data,size);
    for (unsigned i=0; ok && i<2; ++i) {
        ok=exe_read(exe,UINT32_C(0x800495CC)+(i ? 5u : 0u)*14,tables,14);
        for (unsigned n=0; ok && n<7; ++n) t->effect_definitions[i][n]=(uint16_t)le16(tables+n*2);
        if (ok && (t->effect_definitions[i][0]!=0 || t->effect_definitions[i][2]>127
            || t->effect_definitions[i][4]>127 || t->effect_definitions[i][5]>127
            || t->effect_definitions[i][6]>127)) ok=false;
    }
    if (!SDL_CloseIO(exe)) return false;
    if (!ok) return SDL_SetError("Title sound tables or resources are incompatible");
    btn_audio_init(&t->music); btn_audio_init(&t->effects);
    btn_title_reset(&t->state);
    return true;
}

void btn_title_close(BtnTitle *t)
{
    if (!t) return;
    free(t->animation_data); free(t->image_data); free(t->music_data);
    free(t->sequence_data); free(t->effects_data);
    free(t->list); free(t->assets); free(t->graphics);
    *t=(BtnTitle){0};
}

bool btn_title_tick(BtnTitle *t, const BtnInput *input)
{
    int sound=btn_title_step(&t->state,input);
    if (sound>=0) {
        const uint16_t *d=t->effect_definitions[sound==5 ? 1 : 0];
        if (!btn_audio_tone_on(&t->effects,&t->effects_bank,d[2],d[3],d[4],d[5],d[6])) return false;
    }
    return true;
}

static bool sprite(BtnTitle *t, unsigned range, int x, int y)
{
    BtnAnimationCursor cursor={0}; cursor.range=range;
    return btn_animation_draw(&t->animation,&cursor,t->list,0,x,y);
}

bool btn_title_draw(BtnTitle *t, uint32_t *pixels, size_t count)
{
    if (!t || !t->assets || !pixels || count<BTN_WIDTH*BTN_HEIGHT)
        return SDL_SetError("Title is not loaded or framebuffer is too small");
    *t->graphics=*t->assets;
    BtnPrimitive back={0}; back.type=BTN_RECTANGLE; back.w=BTN_WIDTH; back.h=BTN_HEIGHT;
    back.v[0]=(BtnVertex){0,0,0,0,112,176,232};
    if (!btn_draw(t->graphics,&back)) return false;
    btn_draw_list_clear(t->list);
    if (t->state.page==BTN_TITLE_PROMPT) {
        if (!sprite(t,33,0,0)) return false;
    } else {
        unsigned x=t->state.ticks%30<15 ? 100u : 104u;
        if (!sprite(t,36,(int)x,t->state.selection ? 179 : 155)
            || !sprite(t,35,0,0) || !sprite(t,34,0,0)) return false;
    }
    /* Descending OT/head insertion: queue foreground before logo/copyright. */
    return sprite(t,32,0,0) && sprite(t,31,0,0)
        && btn_draw_list_render(t->graphics,t->list)
        && btn_framebuffer(t->graphics,(BtnRect){0,0,BTN_WIDTH,BTN_HEIGHT},pixels,count);
}

bool btn_title_audio(BtnTitle *t, int16_t *stereo, size_t frames)
{
    int16_t effects[512*2];
    if (!t || !stereo) return SDL_SetError("Missing title audio destination");
    while (frames) {
        size_t n=frames>512 ? 512 : frames;
        if (!btn_sequence_render(&t->sequence,&t->music,&t->music_bank,stereo,n)
            || !btn_audio_render(&t->effects,effects,n)) return false;
        for (size_t i=0; i<n*2; ++i) {
            int sample=stereo[i]+effects[i];
            stereo[i]=(int16_t)(sample>32767 ? 32767 : sample< -32768 ? -32768 : sample);
        }
        stereo+=n*2; frames-=n;
    }
    return true;
}
