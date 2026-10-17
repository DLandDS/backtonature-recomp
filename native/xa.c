#include "audio.h"
#include <SDL3/SDL.h>
#include <string.h>

bool btn_xa_sector(BtnXaSector *out,const void *bytes,size_t size)
{
    const uint8_t *b=bytes;
    static const uint8_t sync[12]={0,255,255,255,255,255,255,255,255,255,255,0};
    if (!out||!b||(size!=2352&&size!=2336)) return SDL_SetError("XA requires complete 2352/2336-byte sectors, not 2048-byte data");
    if (size==2352) {
        if (memcmp(b,sync,12)||b[15]!=2) return SDL_SetError("Invalid XA Mode 2 sector header");
        b+=16;
    }
    if (memcmp(b,b+4,4)) return SDL_SetError("XA subheader copies differ");
    if (b[1]>31) return SDL_SetError("Invalid XA channel");
    *out=(BtnXaSector){b[0],b[1],b[2],b[3],(b[3]&4u)?18900u:37800u,(b[3]&1u)?2u:1u,b+8};
    return true;
}
bool btn_xa_decode(BtnXaDecoder *decoder,const BtnXaSector *s,int16_t *out,size_t capacity,size_t *frames)
{
    if (!decoder||!s||!out||!frames||!s->payload||(s->channels!=1&&s->channels!=2))
        return SDL_SetError("Invalid XA decode arguments");
    if ((s->submode&0x24u)!=0x24u) return SDL_SetError("Sector is not XA Form 2 audio");
    if (s->coding&~5u) return SDL_SetError("Unsupported XA coding (requires 4-bit mono/stereo, no emphasis)");
    size_t needed=s->channels==2?2016:4032;
    if (capacity<needed) return SDL_SetError("XA PCM buffer is too small");
    if (decoder->rate&&(decoder->rate!=s->rate||decoder->channel_count!=s->channels))
        return SDL_SetError("XA coding changed within selected stream");
    /* Validate all group headers before changing predictor history/output. */
    for (unsigned g=0;g<18;++g) {
        const uint8_t *p=s->payload+g*128;
        if (memcmp(p,p+4,4)||memcmp(p+8,p+12,4)) return SDL_SetError("XA ADPCM header copies differ");
        for (unsigned u=0;u<8;++u) if (p[4+u]&0xC0u) return SDL_SetError("Invalid XA ADPCM filter");
    }
    static const int positive[4]={0,60,115,98},negative[4]={0,0,-52,-55};
    decoder->rate=s->rate; decoder->channel_count=s->channels;
    for (unsigned g=0;g<18;++g) {
        const uint8_t *p=s->payload+g*128;
        for (unsigned u=0;u<8;++u) {
            unsigned channel=s->channels==2?u%2:0,shift=p[4+u]&15u,filter=p[4+u]>>4;
            BtnAdpcm *history=&decoder->channels[channel];
            if (shift>12) shift=9;
            for (unsigned j=0;j<28;++j) {
                int n=(p[16+u/2+j*4]>>((u%2)*4))&15;
                if (n>=8) n-=16;
                int scaled=n*4096, divisor=1<<(int)shift;
                scaled=scaled>=0?scaled/divisor:-((-scaled+divisor-1)/divisor);
                int prediction=history->old*positive[filter]+history->older*negative[filter]+32;
                prediction=prediction>=0?prediction/64:-((-prediction+63)/64);
                int pcm=scaled+prediction;
                if (pcm<-32768) pcm=-32768;
                if (pcm>32767) pcm=32767;
                history->older=history->old; history->old=pcm;
                size_t frame=s->channels==2?g*112u+(u/2)*28u+j:g*224u+u*28u+j;
                out[frame*2+channel]=(int16_t)pcm;
                if (s->channels==1) out[frame*2+1]=(int16_t)pcm;
            }
        }
    }
    *frames=needed; return true;
}
