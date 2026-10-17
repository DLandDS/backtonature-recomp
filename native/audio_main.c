#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"

typedef struct {
    const char *bank,*sequence,*xa,*output;
    unsigned sample,seconds,volume,fade_ms,sector_size,file,channel;
    bool headless,loop,test,has_sample,has_file,has_channel,has_sector;
} Options;
typedef struct {
    BtnAudioBank bank;
    BtnAudio audio;
    BtnSequence sequence;
    uint8_t *bank_bytes,*seq_bytes;
    SDL_IOStream *xa;
    BtnXaDecoder decoder;
    unsigned rate;
    size_t xa_cursor,xa_frames;
    int16_t xa_pcm[BTN_XA_MAX_FRAMES*2];
    uint64_t selected_sectors;
    bool eof;
} Player;

static void usage(void)
{
    puts("Usage: btn-audio --audio-test --bank FILE.vab [--sample N | --sequence FILE.seq]\n"
         "       btn-audio --audio-test --xa FILE --sector-size 2352|2336 --file N --channel N\n"
         "Common: [--headless] [--seconds 1..600] [--loop] [--volume 0..100]\n"
         "        [--fade-ms N] [--output NEW.wav]\n"
         "VAB files must join logical VH+VB (use tools/prepare_native_audio.py). Sample numbers start at 1.\n"
         "Headless writes PCM without opening a device. Desktop: Space stop/restart, left/right sample,\n"
         "up/down volume, L loop, Escape exit. Default limit 10 seconds.\n"
         "Preview fidelity: dry mix, approximate ADSR, nearest-neighbor pitch; no PS1 parity claim.\n"
         "XA supports intact raw Mode 2 sectors, 4-bit mono/stereo, 37800/18900 Hz; no RIFF/2048-byte input.");
}
static bool number(const char *text,unsigned *out)
{
    char *end; errno=0;
    if (*text<'0'||*text>'9') return false;
    unsigned long long n=strtoull(text,&end,10);
    if (errno||*end||n>UINT32_MAX) return false;
    *out=(unsigned)n; return true;
}
static int parse(Options *o,int argc,char **argv)
{
    for (int i=1;i<argc;++i) {
        const char *arg=argv[i];
        if (!strcmp(arg,"--help")) { usage(); return 0; }
        if (!strcmp(arg,"--audio-test")) { if (o->test) return -1; o->test=true; continue; }
        if (!strcmp(arg,"--headless")) { o->headless=true; continue; }
        if (!strcmp(arg,"--loop")) { o->loop=true; continue; }
        if (++i==argc||!*argv[i]) return -1;
        const char *value=argv[i];
        if (!strcmp(arg,"--bank")) { if (o->bank) return -1; o->bank=value; continue; }
        if (!strcmp(arg,"--sequence")) { if (o->sequence) return -1; o->sequence=value; continue; }
        if (!strcmp(arg,"--xa")) { if (o->xa) return -1; o->xa=value; continue; }
        if (!strcmp(arg,"--output")) { if (o->output) return -1; o->output=value; continue; }
        unsigned n;
        if (!number(value,&n)) return -1;
        if (!strcmp(arg,"--sample")) { if (!n||n>255) return -1; o->sample=n; o->has_sample=true; }
        else if (!strcmp(arg,"--seconds")) { if (!n||n>600) return -1; o->seconds=n; }
        else if (!strcmp(arg,"--volume")) { if (n>100) return -1; o->volume=n; }
        else if (!strcmp(arg,"--fade-ms")) o->fade_ms=n;
        else if (!strcmp(arg,"--sector-size")) { if (n!=2352&&n!=2336) return -1; o->sector_size=n; o->has_sector=true; }
        else if (!strcmp(arg,"--file")) { if (n>255) return -1; o->file=n; o->has_file=true; }
        else if (!strcmp(arg,"--channel")) { if (n>31) return -1; o->channel=n; o->has_channel=true; }
        else return -1;
    }
    if (!o->test||(!o->bank&&!o->xa)||(o->bank&&o->xa)||(o->sequence&&(!o->bank||o->has_sample))
        ||(o->xa&&(o->has_sample||!o->has_sector||!o->has_file||!o->has_channel))
        ||(!o->xa&&(o->has_sector||o->has_file||o->has_channel))
        ||o->fade_ms>o->seconds*1000u) return -1;
    return 1;
}
static uint8_t *load(const char *path,size_t *size)
{
    SDL_IOStream *file=SDL_IOFromFile(path,"rb");
    if (!file) return NULL;
    Sint64 length=SDL_GetIOSize(file);
    uint8_t *bytes=NULL;
    if (length<1||length>16*1024*1024) SDL_SetError("Audio input size must be 1..16 MiB");
    else {
        bytes=SDL_malloc((size_t)length);
        if (bytes&&SDL_ReadIO(file,bytes,(size_t)length)!=(size_t)length) {
            SDL_free(bytes); bytes=NULL; SDL_SetError("Short audio input read");
        }
    }
    bool closed=SDL_CloseIO(file);
    if (!closed) { SDL_free(bytes); return NULL; }
    if (bytes) *size=(size_t)length;
    return bytes;
}
static bool restart(Player *p,const Options *o)
{
    btn_audio_init(&p->audio);
    if (!btn_audio_volume(&p->audio,(float)o->volume/100,0)) return false;
    if (o->sequence) return btn_sequence_open(&p->sequence,p->seq_bytes,p->sequence.size,o->loop);
    if (o->xa) {
        p->decoder=(BtnXaDecoder){0}; p->xa_cursor=p->xa_frames=0; p->eof=false;
        return SDL_SeekIO(p->xa,0,SDL_IO_SEEK_SET)==0;
    }
    return btn_audio_sample_play(&p->audio,&p->bank,o->sample,o->loop);
}
static bool xa_next(Player *p,const Options *o)
{
    uint8_t bytes[2352];
    for (;;) {
        size_t count=SDL_ReadIO(p->xa,bytes,o->sector_size);
        if (!count) {
            if (SDL_GetIOStatus(p->xa)!=SDL_IO_STATUS_EOF) return SDL_SetError("XA stream read failed");
            p->eof=true; p->xa_cursor=p->xa_frames=0; return true;
        }
        if (count!=o->sector_size) return SDL_SetError("Truncated XA sector");
        BtnXaSector sector;
        if (!btn_xa_sector(&sector,bytes,count)) return false;
        if ((sector.submode&0x24u)!=0x24u||sector.file!=o->file||sector.channel!=o->channel) continue;
        if (!btn_xa_decode(&p->decoder,&sector,p->xa_pcm,BTN_XA_MAX_FRAMES,&p->xa_frames)) return false;
        p->rate=sector.rate; p->xa_cursor=0; ++p->selected_sectors; return true;
    }
}
static bool render(Player *p,const Options *o,int16_t *out,size_t frames)
{
    if (!o->xa) {
        if (o->sequence) return btn_sequence_render(&p->sequence,&p->audio,&p->bank,out,frames);
        for (size_t n=0;n<frames;++n) {
            bool active=false;
            for (unsigned i=0;i<BTN_AUDIO_VOICES;++i) active|=p->audio.voices[i].active;
            if (!active&&o->loop&&!btn_audio_sample_play(&p->audio,&p->bank,o->sample,true)) return false;
            if (!btn_audio_render(&p->audio,out+n*2,1)) return false;
        }
        return true;
    }
    for (size_t n=0;n<frames;++n) {
        if (p->xa_cursor==p->xa_frames&&!p->eof&&!xa_next(p,o)) return false;
        if (p->eof&&o->loop) {
            /* Empty selections are rejected at open, so one restart suffices. */
            unsigned rate=p->rate;
            p->decoder=(BtnXaDecoder){0}; p->eof=false;
            if (SDL_SeekIO(p->xa,0,SDL_IO_SEEK_SET)!=0||!xa_next(p,o)) return false;
            if (p->eof) return SDL_SetError("No selected XA audio sectors after restart");
            if (rate!=p->rate) return SDL_SetError("XA rate changed at loop");
        }
        float gain=p->audio.gain;
        if (p->audio.fade_left) {
            gain=p->audio.fade_start*(float)p->audio.fade_left/(float)p->audio.fade_total;
            if (!--p->audio.fade_left) p->audio.gain=0;
        }
        for (unsigned c=0;c<2;++c) out[n*2+c]=p->eof?0:(int16_t)((float)p->xa_pcm[p->xa_cursor*2+c]*gain);
        if (!p->eof) ++p->xa_cursor;
    }
    return true;
}
static void word(uint8_t *bytes,uint32_t value,unsigned count)
{
    for (unsigned i=0;i<count;++i) bytes[i]=(uint8_t)(value>>(i*8));
}
static FILE *wav_open(const char *path,unsigned rate,uint32_t frames)
{
#ifdef _WIN32
    wchar_t *wide=(wchar_t *)SDL_iconv_string("UTF-16LE","UTF-8",path,strlen(path)+1);
    if (!wide) return NULL;
    FILE *file=NULL; _wfopen_s(&file,wide,L"wbx"); SDL_free(wide);
#else
    FILE *file=fopen(path,"wbx");
#endif
    if (!file) { SDL_SetError("Cannot create new WAV (output must not exist)"); return NULL; }
    uint8_t header[44]={0};
    memcpy(header,"RIFF",4); word(header+4,frames*4+36,4); memcpy(header+8,"WAVEfmt ",8);
    word(header+16,16,4); word(header+20,1,2); word(header+22,2,2); word(header+24,rate,4);
    word(header+28,rate*4,4); word(header+32,4,2); word(header+34,16,2);
    memcpy(header+36,"data",4); word(header+40,frames*4,4);
    if (fwrite(header,1,44,file)!=44) { fclose(file); SDL_SetError("Cannot write WAV header"); return NULL; }
    return file;
}
static bool wav_write(FILE *file,const int16_t *pcm,size_t frames)
{
    uint8_t bytes[4096];
    if (frames>1024) return SDL_SetError("WAV chunk too large");
    for (size_t i=0;i<frames*2;++i) word(bytes+i*2,(uint16_t)pcm[i],2);
    if (fwrite(bytes,4,frames,file)!=frames) return SDL_SetError("Cannot write PCM to WAV");
    return true;
}
int main(int argc,char **argv)
{
    Options o={0}; o.sample=1; o.seconds=10; o.volume=50;
    int parsed=parse(&o,argc,argv);
    if (parsed<=0) { if (parsed<0) fputs("Invalid audio arguments. Use --help.\n",stderr); return parsed<0?2:0; }
    Player p={0}; p.rate=BTN_AUDIO_RATE;
    SDL_Window *window=NULL; SDL_AudioStream *stream=NULL; FILE *output=NULL;
    int result=1; unsigned underruns=0; bool started=false,paused=false;
    size_t length;
    if (o.bank) {
        p.bank_bytes=load(o.bank,&length);
        if (!p.bank_bytes||!btn_audio_bank_open(&p.bank,p.bank_bytes,length)) goto done;
        if (p.bank.reverb) fputs("Preview: bank requests reverb; dry output only.\n",stderr);
    }
    if (o.sequence) {
        p.seq_bytes=load(o.sequence,&length); p.sequence.size=length;
        if (!p.seq_bytes) goto done;
    }
    if (o.xa) {
        p.xa=SDL_IOFromFile(o.xa,"rb");
        if (!p.xa) goto done;
        Sint64 size=SDL_GetIOSize(p.xa);
        if (size<=0||size%(Sint64)o.sector_size) { SDL_SetError("XA size is not a whole number of intact sectors"); goto done; }
    }
    if (!restart(&p,&o)) goto done;
    if (o.xa&&(!xa_next(&p,&o)||p.eof)) { if (p.eof) SDL_SetError("No selected XA audio sectors"); goto done; }
    uint32_t total=o.seconds*p.rate, produced=0, fade=(uint32_t)((uint64_t)o.fade_ms*p.rate/1000);
    if (o.output) { output=wav_open(o.output,p.rate,total); if (!output) goto done; }
    if (!o.headless) {
        if (!SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO)) goto done;
        window=SDL_CreateWindow("BTN audio test: Space stop/restart, arrows sample/volume, L loop",760,100,0);
        if (!window) goto done;
        SDL_AudioSpec spec={SDL_AUDIO_S16,2,(int)p.rate};
        stream=SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,&spec,NULL,NULL);
        if (!stream) goto done;
    }
    while (produced<total) {
        SDL_Event e;
        while (window&&SDL_PollEvent(&e)) {
            if (e.type==SDL_EVENT_QUIT||(e.type==SDL_EVENT_KEY_DOWN&&e.key.key==SDLK_ESCAPE)) goto finished;
            if (e.type!=SDL_EVENT_KEY_DOWN||e.key.repeat) continue;
            bool reset=false;
            if (e.key.key==SDLK_SPACE) { paused=!paused; reset=!paused; }
            if (e.key.key==SDLK_L) { o.loop=!o.loop; p.sequence.repeat=o.loop; }
            if (!o.sequence&&!o.xa&&(e.key.key==SDLK_LEFT||e.key.key==SDLK_RIGHT)) {
                o.sample=e.key.key==SDLK_RIGHT?o.sample%p.bank.samples+1:(o.sample+p.bank.samples-2)%p.bank.samples+1;
                reset=true;
            }
            if (e.key.key==SDLK_UP||e.key.key==SDLK_DOWN) {
                o.volume=e.key.key==SDLK_UP?(o.volume>90?100:o.volume+10):(o.volume<10?0:o.volume-10);
                if (!btn_audio_volume(&p.audio,(float)o.volume/100,0)) goto done;
            }
            if (reset&&!restart(&p,&o)) goto done;
            if ((paused||reset)&&!SDL_ClearAudioStream(stream)) goto done;
            if (paused) btn_audio_stop(&p.audio);
            char title[180]; SDL_snprintf(title,sizeof(title),"BTN sample %u | volume %u%% | loop %s | %s",o.sample,o.volume,o.loop?"on":"off",paused?"stopped":"playing");
            SDL_SetWindowTitle(window,title);
        }
        if (paused) { SDL_Delay(10); continue; }
        if (stream) {
            int queued=SDL_GetAudioStreamQueued(stream);
            if (queued<0) goto done;
            if (started&&!queued) ++underruns;
            if (queued>=4096*3) { SDL_Delay(2); continue; }
        }
        if (fade&&produced>=total-fade&&!p.audio.fade_total)
            if (!btn_audio_volume(&p.audio,p.audio.gain,total-produced)) goto done;
        size_t count=total-produced;
        if (count>1024) count=1024;
        if (fade&&produced<total-fade&&count>total-fade-produced) count=total-fade-produced;
        int16_t pcm[2048];
        if (!render(&p,&o,pcm,count)||(output&&!wav_write(output,pcm,count))
            ||(stream&&!SDL_PutAudioStreamData(stream,pcm,(int)(count*4)))) goto done;
        produced+=(uint32_t)count;
        if (stream&&!started) {
            if (SDL_GetAudioStreamQueued(stream)>=8192||produced==total) {
                if (!SDL_ResumeAudioStreamDevice(stream)) goto done;
                started=true;
            }
        }
    }
    if (stream) {
        if (!SDL_FlushAudioStream(stream)) goto done;
        uint64_t deadline=SDL_GetTicks()+3000;
        while (SDL_GetAudioStreamQueued(stream)>0&&SDL_GetTicks()<deadline) SDL_Delay(5);
        SDL_Delay(80);
    }
finished:
    /* Patch WAV length on an early interactive stop. */
    if (output) {
        long end=ftell(output); uint8_t bytes[4];
        if (end<44) { SDL_SetError("Invalid WAV output length"); goto done; }
        word(bytes,(uint32_t)end-8,4);
        if (fseek(output,4,SEEK_SET)||fwrite(bytes,1,4,output)!=4) { SDL_SetError("Cannot finalize WAV size"); goto done; }
        word(bytes,(uint32_t)end-44,4);
        if (fseek(output,40,SEEK_SET)||fwrite(bytes,1,4,output)!=4) { SDL_SetError("Cannot finalize WAV data"); goto done; }
    }
    printf("audio-test rate=%u peak_voices=%u stolen=%u underruns=%u xa_sectors=%llu\n",p.rate,p.audio.peak_voices,p.audio.stolen,underruns,(unsigned long long)p.selected_sectors);
    result=0;
done:
    if (result) fprintf(stderr,"audio failed: %s\n",SDL_GetError());
    if (output&&fclose(output)) { fputs("Cannot close WAV output\n",stderr); result=1; }
    if (stream) SDL_DestroyAudioStream(stream);
    if (window) SDL_DestroyWindow(window);
    if (p.xa) SDL_CloseIO(p.xa);
    SDL_free(p.bank_bytes); SDL_free(p.seq_bytes); SDL_Quit();
    return result;
}
