#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "animation.h"
#include "graphics_scene.h"
#include "image_bank.h"

typedef struct {
    const char *directory,*output;
    unsigned member,image,palette,depth,mode,frames,tick,animation_member,range,frame;
    uint32_t offset,animation_offset;
    bool viewer,test,headless,has_animation,has_image,has_palette,has_frame,has_depth;
} Options;
typedef struct {
    BtnGraphics *graphics,*assets;
    BtnDrawList *list;
    uint32_t *pixels;
    BtnArchive archive;
    BtnImageBank bank;
    BtnAnimation animation;
    uint8_t *bank_data,*font_data,*animation_data;
    bool font;
} Scene;

static void usage(void)
{
    puts("Usage: btn-graphics --graphics-test [--data-dir PATH] [--tick N]\n"
        "       btn-graphics --asset-viewer --data-dir PATH [--member 0..31] [--offset N]\n"
        "         [--image BLOCK] [--palette BLOCK] [--depth 4|8|15] [--mode 0|1|2]\n"
        "         [--animation-member 0..31] [--animation-offset N] [--range N] [--frame N]\n"
        "Common: [--headless] [--frames N] [--output NEW.ppm]\n"
        "Left/right: image; up/down: pan; Space: pause; Escape: exit; F11: fullscreen.\n"
        "Headless defaults to one frame. Output is a new P6 PPM without diagnostic overlays.\n"
        "0D bank and 0E animation selection are explicit, not an inferred association.\n"
        "Synthetic scene and selected resources only; PS1 image parity is not established.");
}
static bool number(const char *text,uint32_t *out)
{
    char *end; errno=0;
    if (*text<'0' || *text>'9') return false;
    unsigned long long n=strtoull(text,&end,10);
    if (errno || *end || n>UINT32_MAX) return false;
    *out=(uint32_t)n; return true;
}
static int parse(Options *o,int argc,char **argv)
{
    for (int i=1; i<argc; ++i) {
        const char *arg=argv[i];
        if (!strcmp(arg,"--help")) { usage(); return 0; }
        if (!strcmp(arg,"--asset-viewer")) { if (o->viewer||o->test) return -1; o->viewer=true; continue; }
        if (!strcmp(arg,"--graphics-test")) { if (o->viewer||o->test) return -1; o->test=true; continue; }
        if (!strcmp(arg,"--headless")) { o->headless=true; continue; }
        if (i+1>=argc) return -1;
        const char *value=argv[++i];
        if (!strcmp(arg,"--data-dir")) { if (!*value || o->directory) return -1; o->directory=value; continue; }
        if (!strcmp(arg,"--output")) { if (!*value || o->output) return -1; o->output=value; continue; }
        uint32_t n;
        if (!number(value,&n)) return -1;
        if (!strcmp(arg,"--member")) { if (n>=32) return -1; o->member=n; }
        else if (!strcmp(arg,"--offset")) o->offset=n;
        else if (!strcmp(arg,"--image")) { o->image=n; o->has_image=true; }
        else if (!strcmp(arg,"--palette")) { o->palette=n; o->has_palette=true; }
        else if (!strcmp(arg,"--depth")) { if (n!=4&&n!=8&&n!=15) return -1; o->depth=n; o->has_depth=true; }
        else if (!strcmp(arg,"--mode")) { if (n>2) return -1; o->mode=n; }
        else if (!strcmp(arg,"--frames")) { if (!n || n>1000000) return -1; o->frames=n; }
        else if (!strcmp(arg,"--tick")) { if (n>1000000) return -1; o->tick=n; }
        else if (!strcmp(arg,"--animation-member")) { if (n>=32) return -1; o->animation_member=n; o->has_animation=true; }
        else if (!strcmp(arg,"--animation-offset")) o->animation_offset=n;
        else if (!strcmp(arg,"--range")) o->range=n;
        else if (!strcmp(arg,"--frame")) { o->frame=n; o->has_frame=true; }
        else return -1;
    }
    if ((!o->viewer&&!o->test) || (o->viewer&&!o->directory)
        || (o->has_animation&&!o->viewer) || (!o->has_animation&&(o->animation_offset||o->range||o->has_frame))) return -1;
    if (o->test&&(o->offset||o->member||o->has_image||o->has_palette||o->has_depth||o->mode)) return -1;
    if (o->headless&&!o->frames) o->frames=1;
    return 1;
}
static bool load(Scene *s,const Options *o)
{
    s->graphics=calloc(1,sizeof(*s->graphics)); s->assets=calloc(1,sizeof(*s->assets));
    s->list=calloc(1,sizeof(*s->list)); s->pixels=calloc(BTN_WIDTH*BTN_HEIGHT,sizeof(*s->pixels));
    if (!s->graphics||!s->assets||!s->list||!s->pixels) return SDL_SetError("Cannot allocate graphics scene");
    btn_graphics_init(s->assets);
    if (o->directory) {
        char *hdt=NULL,*bin=NULL;
        bool ok=SDL_asprintf(&hdt,"%s/A_FILE.HDT",o->directory)>=0
            && SDL_asprintf(&bin,"%s/A_FILE.BIN",o->directory)>=0 && btn_archive_open(&s->archive,hdt,bin);
        SDL_free(hdt); SDL_free(bin);
        if (!ok) return false;
        if (o->viewer) {
            if (!btn_image_bank_read(&s->archive,o->member,o->offset,&s->bank_data,&s->bank)
                || !btn_image_bank_upload(s->assets,&s->bank,0,0,0,0,o->mode)) return false;
            printf("resource: member=%u offset=%" PRIu32 " bytes=%zu blocks=%u mode=%u\n",
                o->member,o->offset,s->bank.consumed,s->bank.count,o->mode);
        } else {
            BtnImageBank font;
            if (!btn_image_bank_read(&s->archive,0,0,&s->font_data,&font)
                || !btn_image_bank_upload(s->assets,&font,0,0,0,0,0)) return false;
            s->font=true;
        }
        if (o->has_animation && !btn_animation_read(&s->archive,o->animation_member,
            o->animation_offset,&s->animation_data,&s->animation)) return false;
    }
    if (o->test) btn_graphics_test_texture(s->assets);
    return true;
}
static bool draw_image(Scene *s,const Options *o,int pan)
{
    BtnImageBlock image,palette;
    if (!btn_image_bank_block(&s->bank,o->image,&image)) return false;
    unsigned depth=o->has_depth ? o->depth : ((image.flags&3u)==2u ? 15u : (image.flags&1u) ? 8u : 4u);
    unsigned factor=depth==4 ? 4u : depth==8 ? 2u : 1u;
    BtnPrimitive p={0}; p.type=BTN_RECTANGLE; p.w=image.rect.w*(int)factor; p.h=image.rect.h;
    p.material.textured=true; p.material.raw=true;
    p.material.page=(uint16_t)((unsigned)image.rect.x/64u|((unsigned)image.rect.y/256u<<4)
        |((depth==4 ? 0u : depth==8 ? 1u : 2u)<<7));
    if (depth!=15) {
        if (!btn_image_bank_block(&s->bank,o->palette,&palette)) return false;
        if ((palette.flags&3u)!=2u || palette.rect.w<(depth==4 ? 16 : 256) || palette.rect.x%16)
            return SDL_SetError("Selected palette block cannot supply this image's CLUT");
        p.material.clut=(uint16_t)((unsigned)palette.rect.x/16u|((unsigned)palette.rect.y<<6));
    }
    p.v[0]=(BtnVertex){8,40-pan,(image.rect.x&63)*(int)factor,image.rect.y&255,128,128,128};
    return btn_draw(s->graphics,&p);
}
static bool render(Scene *s,const Options *o,BtnAnimationCursor *cursor,unsigned tick,int pan,uint32_t *flags)
{
    memcpy(s->graphics,s->assets,sizeof(*s->graphics));
    if (o->test) {
        if (!btn_graphics_test_scene(s->graphics,s->list,tick,flags)) return false;
        if (s->font) {
            const uint16_t text[]={1,0x13,0x0D,0xFFD,6,0x31,0x20,0x2F,0x27,0x28,0x22,0x32,0xFFFE,
                0,0xFFD,1,0xFFD,2,0xFFD,0x40,0x41,0x42,0xFFFF};
            btn_draw_list_clear(s->list);
            if (!btn_text_draw(s->list,0,text,sizeof(text)/sizeof(text[0]),8,8,NULL)
                || !btn_draw_list_render(s->graphics,s->list)) return false;
        }
    } else {
        BtnPrimitive background={0}; background.type=BTN_RECTANGLE;
        background.w=BTN_WIDTH; background.h=BTN_HEIGHT; background.v[0]=(BtnVertex){0,0,0,0,24,32,48};
        if (!btn_draw(s->graphics,&background)) return false;
        if (o->has_animation) {
            btn_draw_list_clear(s->list);
            if (!btn_animation_draw(&s->animation,cursor,s->list,0,160,120-pan)
                || !btn_draw_list_render(s->graphics,s->list)) return false;
        } else if (!draw_image(s,o,pan)) return false;
    }
    return btn_framebuffer(s->graphics,(BtnRect){0,0,BTN_WIDTH,BTN_HEIGHT},s->pixels,BTN_WIDTH*BTN_HEIGHT);
}
static bool ppm(const char *path,const uint32_t *pixels)
{
    FILE *file=NULL;
#ifdef _WIN32
    wchar_t *wide=(wchar_t *)SDL_iconv_string("UTF-16LE","UTF-8",path,strlen(path)+1);
    if (wide) { _wfopen_s(&file,wide,L"wbx"); SDL_free(wide); }
#else
    file=fopen(path,"wbx");
#endif
    if (!file) return SDL_SetError("Cannot create new image (output must not exist): %s",path);
    bool ok=fprintf(file,"P6\n%d %d\n255\n",BTN_WIDTH,BTN_HEIGHT)>0;
    for (size_t i=0; ok&&i<BTN_WIDTH*BTN_HEIGHT; ++i) {
        uint8_t rgb[3]={(uint8_t)(pixels[i]>>16),(uint8_t)(pixels[i]>>8),(uint8_t)pixels[i]};
        ok=fwrite(rgb,1,3,file)==3;
    }
    if (fclose(file)) ok=false;
    return ok ? true : SDL_SetError("Cannot write complete output image");
}

int main(int argc,char **argv)
{
    Options o={0}; o.image=1;
    int parsed=parse(&o,argc,argv);
    if (parsed<=0) { if (parsed<0) fputs("Invalid graphics arguments. Use --help.\n",stderr); return parsed<0 ? 2 : 0; }
    Scene s={0}; SDL_Window *window=NULL; SDL_Renderer *renderer=NULL; SDL_Texture *texture=NULL;
    int result=1,pan=0; unsigned frames=0,tick=o.tick,last_tick=tick; uint32_t flags=0;
    BtnAnimationCursor cursor={0}; cursor.range=o.range; cursor.frame=o.frame;
    if (!load(&s,&o)) goto done;
    if (o.has_animation&&!o.has_frame)
        for (unsigned i=0; i<o.tick; ++i) if (!btn_animation_tick(&s.animation,&cursor)) goto done;
    if (!o.headless) {
        if (!SDL_Init(SDL_INIT_VIDEO)
            || !SDL_CreateWindowAndRenderer("BTN - graphics / asset viewer",960,720,SDL_WINDOW_RESIZABLE,&window,&renderer)
            || !SDL_SetRenderLogicalPresentation(renderer,BTN_WIDTH,BTN_HEIGHT,SDL_LOGICAL_PRESENTATION_LETTERBOX)) goto done;
        texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_XRGB8888,SDL_TEXTUREACCESS_STREAMING,BTN_WIDTH,BTN_HEIGHT);
        if (!texture||!SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST)) goto done;
    }
    bool running=true,paused=o.has_frame,active=true;
    BtnClock clock={0}; uint64_t previous=SDL_GetTicksNS();
    while (running&&(!o.frames||frames<o.frames)) {
        uint64_t start=SDL_GetTicksNS(); SDL_Event event;
        while (!o.headless&&SDL_PollEvent(&event)) {
            if (event.type==SDL_EVENT_QUIT || event.type==SDL_EVENT_WINDOW_CLOSE_REQUESTED) running=false;
            if (event.type==SDL_EVENT_WINDOW_FOCUS_LOST || event.type==SDL_EVENT_WINDOW_MINIMIZED) { active=false; clock.phase=0; }
            if (event.type==SDL_EVENT_WINDOW_FOCUS_GAINED || event.type==SDL_EVENT_WINDOW_RESTORED) { active=true; previous=start; }
            if (event.type==SDL_EVENT_KEY_DOWN&&!event.key.repeat) {
                switch (event.key.scancode) {
                case SDL_SCANCODE_ESCAPE: running=false; break;
                case SDL_SCANCODE_SPACE: paused=!paused; break;
                case SDL_SCANCODE_F11:
                    if (!SDL_SetWindowFullscreen(window,(SDL_GetWindowFlags(window)&SDL_WINDOW_FULLSCREEN)==0)) goto done;
                    break;
                case SDL_SCANCODE_RIGHT: if (o.viewer&&!o.has_animation&&s.bank.count) o.image=(o.image+1)%s.bank.count; break;
                case SDL_SCANCODE_LEFT: if (o.viewer&&!o.has_animation&&s.bank.count) o.image=(o.image+s.bank.count-1)%s.bank.count; break;
                case SDL_SCANCODE_UP: if (pan>-512) pan-=8; break;
                case SDL_SCANCODE_DOWN: if (pan<512) pan+=8; break;
                default: break;
                }
            }
        }
        if (!running) break;
        if (!render(&s,&o,&cursor,tick,pan,&flags)) goto done;
        last_tick=tick;
        if (!o.headless) {
            if (!SDL_UpdateTexture(texture,NULL,s.pixels,BTN_WIDTH*(int)sizeof(*s.pixels))
                || !SDL_SetRenderDrawColor(renderer,0,0,0,255) || !SDL_RenderClear(renderer)
                || !SDL_RenderTexture(renderer,texture,NULL,NULL) || !SDL_SetRenderDrawColor(renderer,255,255,255,255)
                || !SDL_RenderDebugTextFormat(renderer,8,224,"%s  image %u  tick %u",o.test ? "SYNTHETIC" : "ASSET",o.image,tick)
                ) goto done;
            if (o.viewer) {
                if (!SDL_RenderDebugTextFormat(renderer,8,8,"Member %u / offset %" PRIu32,o.member,o.offset)) goto done;
                if (o.has_animation) {
                    if (!SDL_RenderDebugTextFormat(renderer,8,20,"Animation %u / range %u / frame %u",
                        o.animation_member,cursor.range,cursor.frame)) goto done;
                } else {
                    BtnImageBlock block;
                    if (!btn_image_bank_block(&s.bank,o.image,&block)
                        || !SDL_RenderDebugTextFormat(renderer,8,20,"VRAM %d,%d %dx%d flags %02X pal %u",
                            block.rect.x,block.rect.y,block.rect.w,block.rect.h,(unsigned)block.flags,o.palette)) goto done;
                }
            }
            if (!SDL_RenderPresent(renderer)) goto done;
        }
        ++frames;
        unsigned ticks=o.headless ? 1u : btn_clock_advance(&clock,start-previous,active&&!paused);
        previous=start;
        if (!paused) for (unsigned i=0; i<ticks; ++i) {
            ++tick;
            if (o.has_animation&&!btn_animation_tick(&s.animation,&cursor)) goto done;
        }
        if (!o.headless) { uint64_t elapsed=SDL_GetTicksNS()-start; if (elapsed<16666667) SDL_DelayNS(16666667-elapsed); }
    }
    if (o.output) {
        if (!frames) { SDL_SetError("No frame rendered for requested output"); goto done; }
        if (!ppm(o.output,s.pixels)) goto done;
    }
    printf("graphics: frames=%u last_tick=%u gte_flags=0x%08" PRIX32 " SDL=%d pointer_bits=%zu\n",
        frames,last_tick,flags,SDL_GetVersion(),sizeof(void*)*8);
    result=0;
done:
    if (result) fprintf(stderr,"graphics failed: %s\n",SDL_GetError());
    SDL_DestroyTexture(texture); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
    btn_archive_close(&s.archive);
    free(s.bank_data); free(s.font_data); free(s.animation_data);
    free(s.pixels); free(s.list); free(s.assets); free(s.graphics);
    return result;
}
