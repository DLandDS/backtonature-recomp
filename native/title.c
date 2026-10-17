#include "title.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>

static unsigned le16(const uint8_t *p) { return (unsigned)p[0] | ((unsigned)p[1]<<8); }

static void reset_actors(BtnTitle *t)
{
    const unsigned ranges[6]={0,10,19,22,24,26};
    unsigned slot=0;
    for (unsigned i=0; i<6; ++i) {
        t->actors[i]=(BtnAnimationCursor){0}; t->actors[i].range=ranges[i]; t->actor_ticks[i]=0;
        unsigned copies=i<3 ? 1u : i==3 ? 3u : i==4 ? 5u : 3u;
        for (unsigned n=0; n<copies; ++n) t->actor_frames[slot++]=t->actors[i];
    }
    t->background[0]=t->background[1]=t->background_frame[0]=t->background_frame[1]=0;
}

void btn_title_reset(BtnTitleState *s)
{
    *s=(BtnTitleState){0}; s->page=BTN_TITLE_INTRO; s->selection=1;
    s->fade_mode=1; s->fade_level=9; s->fade_length=10;
    s->logo_y=-160; s->copyright_y=48; s->start_x=320;
}

static void transition(BtnTitleState *s, BtnTitlePage target)
{
    s->target=target; s->pending=true;
    s->fade_mode=2; s->fade_level=0; s->fade_length=11;
}

int btn_title_step(BtnTitleState *s, const BtnInput *input)
{
    ++s->ticks; ++s->page_ticks;
    /* screen_fade_frame runs before scene dispatch. Fade-in starts at n-1;
     * fade-out settles at (n+1)+1, matching screen_fade.c mode 1/2. */
    if (s->fade_mode==1 && s->fade_level) --s->fade_level;
    else if (s->fade_mode==2 && s->fade_level<s->fade_length+1) ++s->fade_level;
    if (s->pending) {
        if (s->fade_level>=s->fade_length+1) {
            s->page=s->target; s->page_ticks=0; s->pending=false;
            s->fade_mode=1; s->fade_level=9; s->fade_length=10;
        }
        return -1;
    }
    if (s->fade_mode==1 && s->fade_level) return -1;
    if (s->page==BTN_TITLE_READY) {
        if (input->pressed & BTN_BIT(BTN_B)) btn_title_reset(s);
        return -1;
    }
    if (s->page==BTN_TITLE_SETUP) return -1;
    if (s->page==BTN_TITLE_INTRO) {
        if (input->pressed & (BTN_BIT(BTN_A)|BTN_BIT(BTN_START))) {
            transition(s,BTN_TITLE_PROMPT); return 0;
        }
        /* Audited 800B8E5C..800B9120: wait, logo -160..0, copyright
         * +48..0, Start +320..0, blink 90 ticks, slide left, repeat Start. */
        ++s->intro_ticks;
        switch (s->intro_stage) {
        case 0: if (s->intro_ticks>=30) { s->intro_stage=1; s->intro_ticks=0; } break;
        case 1: if (++s->logo_y>=0) s->intro_stage=2; break;
        case 2: if (--s->copyright_y<=0) s->intro_stage=3; break;
        case 3: if ((s->start_x-=2)<=0) { s->intro_stage=4; s->intro_ticks=0; } break;
        case 4: if (s->intro_ticks>=90) s->intro_stage=5; break;
        case 5: if ((s->start_x-=2)<=-160) { s->intro_stage=3; s->start_x=320; } break;
        default: break;
        }
        if (s->page_ticks>=1800) transition(s,BTN_TITLE_ATTRACT);
        return -1;
    }
    if (s->page==BTN_TITLE_ATTRACT) {
        if (input->pressed & (BTN_BIT(BTN_B)|BTN_BIT(BTN_START))) transition(s,BTN_TITLE_PROMPT);
        return -1;
    }
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
        if (s->page_ticks>=1800) transition(s,BTN_TITLE_ATTRACT);
        return -1;
    }
    /* 800B86D0: up/down/select toggle once per pressed edge, before confirm.
     * B returning to Start is a native preview control, absent from that branch. */
    if (input->pressed & BTN_BIT(BTN_B)) { s->page=BTN_TITLE_PROMPT; s->page_ticks=0; return -1; }
    if (input->pressed & (BTN_BIT(BTN_UP)|BTN_BIT(BTN_DOWN))) { s->selection^=1; return 5; }
    if (input->pressed & (BTN_BIT(BTN_A)|BTN_BIT(BTN_START))) {
        if (!s->selection) transition(s,BTN_TITLE_SETUP);
        else { s->page=BTN_TITLE_UNAVAILABLE; s->page_ticks=0; }
        return 0;
    }
    return -1;
}

const char *btn_title_notice(const BtnTitleState *s)
{
    if (s->page==BTN_TITLE_UNAVAILABLE) return "Continue unavailable: save/load requires phase 07";
    if (s->page==BTN_TITLE_ATTRACT) return "Attract demo requires gameplay / phase 06";
    if (s->page==BTN_TITLE_READY) return "New Game setup ready; gameplay requires phase 06";
    if (s->page==BTN_TITLE_SETUP) return "New Game setup: player, birthday, farm, dog";
    return "Native title / PS1 comparison pending";
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

static bool exe_caption(SDL_IOStream *file, uint32_t address, uint16_t *out, unsigned count)
{
    uint8_t word[2];
    for (unsigned i=0; i<count; ++i) {
        if (!exe_read(file,address+i*2,word,2)) return false;
        out[i]=(uint16_t)le16(word);
        if (out[i]==0xFFFF) return true;
        if (out[i]>255 && out[i]!=0xFFD && out[i]!=0xFFFE)
            return SDL_SetError("Unsupported title keyboard caption control");
    }
    return SDL_SetError("Title keyboard caption has no bounded END");
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
    for (unsigned i=0; ok && i<6; ++i) {
        ok=exe_read(exe,UINT32_C(0x800495CC)+i*14,tables,14);
        for (unsigned n=0; ok && n<7; ++n) t->effect_definitions[i][n]=(uint16_t)le16(tables+n*2);
        if (ok && (t->effect_definitions[i][0]!=0 || t->effect_definitions[i][2]>127
            || t->effect_definitions[i][4]>127 || t->effect_definitions[i][5]>127
            || t->effect_definitions[i][6]>127)) ok=false;
    }
    if (ok) ok=exe_read(exe,UINT32_C(0x8005E364),t->name_layout.row_last[0],6)
        && exe_read(exe,UINT32_C(0x8005E36C),t->name_layout.row_last[1],6)
        && exe_read(exe,UINT32_C(0x8005599C),t->name_layout.column_last[0],14)
        && exe_read(exe,UINT32_C(0x800559AC),t->name_layout.column_last[1],14);
    for (unsigned i=0; ok && i<28; ++i) {
        ok=exe_read(exe,UINT32_C(0x800559D4)+i*2,tables,2);
        if (ok) t->name_layout.symbols[i]=(uint16_t)le16(tables);
    }
    if (ok) ok=btn_name_layout_valid(&t->name_layout)
        && exe_read(exe,UINT32_C(0x8005E290),tables,4);
    if (ok) t->random=btn_read_le32(tables);
    /* runtime/main.c selects palette 3; text_glyph_sprite maps its CLUT row. */
    if (ok) ok=exe_read(exe,UINT32_C(0x80049044)+3*2,tables,2);
    if (ok) { t->text_clut_y=le16(tables); ok=t->text_clut_y<BTN_VRAM_H; }
    if (ok) ok=exe_caption(exe,UINT32_C(0x800537B2),t->name_captions[0],128)
        && exe_caption(exe,UINT32_C(0x80053888),t->name_captions[1],128);
    if (!SDL_CloseIO(exe)) return false;
    if (!ok) return SDL_SetError("Title sound/text tables or resources are incompatible");
    BtnImageBank font;
    if (!btn_image_bank_read(archive,0,0,&t->font_data,&font)
        || !btn_image_bank_upload(t->assets,&font,0,0,0,0,0)) return false;
    btn_audio_init(&t->music); btn_audio_init(&t->effects);
    btn_title_reset(&t->state);
    btn_setup_reset(&t->setup);
    reset_actors(t);
    return true;
}

void btn_title_close(BtnTitle *t)
{
    if (!t) return;
    free(t->animation_data); free(t->image_data); free(t->music_data);
    free(t->sequence_data); free(t->effects_data); free(t->font_data);
    free(t->list); free(t->assets); free(t->graphics);
    *t=(BtnTitle){0};
}

static unsigned random_range(BtnTitle *t, unsigned range)
{
    t->random=t->random*UINT32_C(0x41C64E6D)+UINT32_C(0x3039);
    return ((t->random>>16)*(range&65535u))>>16;
}

static bool animate(BtnTitle *t)
{
    const unsigned delays[6]={90,60,60,15,20,30};
    unsigned slot=0;
    for (unsigned i=0; i<6; ++i) {
        unsigned copies=i<3 ? 1u : i==3 ? 3u : i==4 ? 5u : 3u;
        for (unsigned n=0; n<copies; ++n) {
            t->actor_frames[slot++]=t->actors[i];
            if (!btn_animation_tick(&t->animation,&t->actors[i])) return false;
        }
        if (++t->actor_ticks[i]>=delays[i]) {
            t->actor_ticks[i]=0;
            unsigned range;
            if (i<2) {
                int candidate=(int)random_range(t,i ? 9 : 10)+(i ? 10 : 0);
                if (!(candidate&1)) --candidate;
                int minimum=i ? 10 : 0;
                range=(unsigned)(candidate<minimum ? minimum : candidate);
            } else if (i==2) {
                range=19+random_range(t,3);
                if (range==19) t->actor_ticks[i]=30;
            } else range=22+(i-3)*2+random_range(t,2);
            t->actors[i]=(BtnAnimationCursor){0}; t->actors[i].range=range;
        }
    }
    t->background_frame[0]=t->background[0]; t->background_frame[1]=t->background[1];
    if (t->state.ticks && ((t->state.ticks-1)&1u)) t->background[0]=(t->background[0]+1)%960;
    t->background[1]=(t->background[1]+1)%960;
    return true;
}

const BtnNewGame *btn_title_new_game(const BtnTitle *t)
{
    return t && t->state.page==BTN_TITLE_READY && !t->state.pending
        && !t->state.fade_level && t->setup.stage==BTN_SETUP_DONE ? &t->setup.game : NULL;
}

bool btn_title_tick(BtnTitle *t, const BtnInput *input)
{
    BtnTitlePage before=t->state.page;
    (void)random_range(t,65535); /* main_run_frame consumes one random draw. */
    int sound=btn_title_step(&t->state,input);
    if (before==BTN_TITLE_READY && t->state.page==BTN_TITLE_INTRO) {
        reset_actors(t); btn_setup_reset(&t->setup); btn_audio_init(&t->music); btn_audio_stop(&t->effects);
        if (!btn_sequence_open(&t->sequence,t->sequence_data,t->sequence.size,true)) return false;
    }
    if (before!=BTN_TITLE_SETUP && t->state.page==BTN_TITLE_SETUP) btn_setup_reset(&t->setup);
    else if (t->state.page==BTN_TITLE_SETUP && !t->state.pending && !t->state.fade_level) {
        sound=btn_setup_step(&t->setup,&t->name_layout,input);
        if (t->setup.stage==BTN_SETUP_DONE) {
            for (unsigned i=0; i<4; ++i) t->setup.game.seeds[i]=(uint16_t)random_range(t,65535);
            if (!btn_audio_volume(&t->music,1,BTN_AUDIO_RATE*10/BTN_TITLE_HZ)) return false;
            transition(&t->state,BTN_TITLE_READY);
        }
    }
    if (sound>=0 && sound<6) {
        const uint16_t *d=t->effect_definitions[sound];
        if (!btn_audio_tone_on(&t->effects,&t->effects_bank,d[2],d[3],d[4],d[5],d[6])) return false;
    }
    return animate(t);
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
