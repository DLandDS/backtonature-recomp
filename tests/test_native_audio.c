#include "audio.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr,"line %d: %s (%s)\n",__LINE__,#c,SDL_GetError()); exit(1); } } while (0)
#define BANK_BYTES (0xC20+32)
static void word(uint8_t *p,unsigned v) { p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8); }
static void fixture(uint8_t *b)
{
    memset(b,0,BANK_BYTES); memcpy(b,"pBAV",4); b[4]=7;
    word(b+12,BANK_BYTES); word(b+18,1); word(b+20,1); word(b+22,1);
    b[24]=127; b[25]=64; b[32]=1; b[33]=127; b[36]=64;
    uint8_t *t=b+0x820; t[2]=127; t[3]=64; t[4]=60; t[7]=127;
    word(t+16,15); word(t+18,0x1FC0); word(t+22,1); word(b+0xA22,4);
    b[0xC20]=1; b[0xC21]=4; memset(b+0xC22,0x11,14);
    b[0xC30]=1; b[0xC31]=3; memset(b+0xC32,0x22,14);
}
static void adpcm(void)
{
    BtnAdpcm history={0}; uint8_t block[16]={0x0C,1}; int16_t pcm[28];
    memset(block+2,0x87,14);
    CHECK(btn_adpcm_block(&history,block,pcm));
    for (unsigned i=0;i<28;++i) CHECK(pcm[i]==(i%2?-8:7));
    block[0]=0x1C; memset(block+2,0,14); history=(BtnAdpcm){100,0};
    CHECK(btn_adpcm_block(&history,block,pcm)); CHECK(pcm[0]==94&&pcm[1]==88);
    history=(BtnAdpcm){32767,32767}; block[0]=0x40; memset(block+2,0x77,14);
    CHECK(btn_adpcm_block(&history,block,pcm)); CHECK(pcm[0]==32767);
    BtnAdpcm before=history; block[0]=0x50;
    CHECK(!btn_adpcm_block(&history,block,pcm)); CHECK(!memcmp(&before,&history,sizeof(history)));
    block[0]=0; block[1]=8; CHECK(!btn_adpcm_block(&history,block,pcm));
}
static void mixer(void)
{
    uint8_t bytes[BANK_BYTES]; fixture(bytes); BtnAudioBank bank={0};
    CHECK(btn_audio_bank_open(&bank,bytes,sizeof(bytes)));
    CHECK(bank.samples==1&&bank.sample_offset[1]==0xC20);
    BtnAudioBank before=bank;
    CHECK(!btn_audio_bank_open(&bank,bytes,sizeof(bytes)-1)); CHECK(!memcmp(&bank,&before,sizeof(bank)));
    word(bytes+0xA22,0xFFFF); CHECK(!btn_audio_bank_open(&bank,bytes,sizeof(bytes))); fixture(bytes);
    bytes[0x821]=4; CHECK(btn_audio_bank_open(&bank,bytes,sizeof(bytes))&&bank.reverb); fixture(bytes);
    BtnAudio a; btn_audio_init(&a); int16_t pcm[256];
    CHECK(!btn_audio_sample_play(&a,&bank,0,false));
    CHECK(btn_audio_sample_play(&a,&bank,1,true));
    CHECK(btn_audio_render(&a,pcm,84));
    CHECK(pcm[0]==2048&&pcm[54]==2048&&pcm[56]==4096&&pcm[112]==2048);
    btn_audio_stop(&a); CHECK(btn_audio_render(&a,pcm,10));
    for (unsigned i=0;i<20;++i) CHECK(pcm[i]==0);
    CHECK(btn_audio_sample_play(&a,&bank,1,true)); CHECK(btn_audio_volume(&a,1,4));
    CHECK(btn_audio_render(&a,pcm,5));
    CHECK(pcm[0]==2048&&pcm[2]==1536&&pcm[4]==1024&&pcm[6]==512&&pcm[8]==0);
    btn_audio_init(&a);
    for (unsigned i=0;i<25;++i) CHECK(btn_audio_sample_play(&a,&bank,1,true));
    CHECK(a.peak_voices==24&&a.stolen==1); CHECK(btn_audio_render(&a,pcm,1)); CHECK(pcm[0]==32767);
    btn_audio_init(&a); a.channels[0].pan=0;
    CHECK(btn_audio_note_on(&a,&bank,0,60,127)); CHECK(btn_audio_render(&a,pcm,64));
    CHECK(pcm[100]>0&&pcm[101]==0); btn_audio_note_off(&a,0,60);
    CHECK(btn_audio_render(&a,pcm,64)); CHECK(!a.voices[0].active);
    CHECK(!btn_audio_volume(&a,-1,0)); CHECK(!btn_audio_volume(&a,2,0));
    btn_audio_init(&a);
    CHECK(!btn_audio_tone_on(&a,&bank,0,1,60,0,127));
    CHECK(!btn_audio_tone_on(&a,&bank,0,0,60,128,127));
    CHECK(btn_audio_tone_on(&a,&bank,0,0,60,64,127));
    CHECK(a.peak_voices==1&&a.voices[0].active&&a.voices[0].pitch>4096.0);
    CHECK(btn_audio_render(&a,pcm,64));
    bytes[32]=2; word(bytes+20,2);
    memcpy(bytes+0x840,bytes+0x820,32); bytes[0x844]=48;
    CHECK(btn_audio_bank_open(&bank,bytes,sizeof(bytes)));
    btn_audio_init(&a);
    CHECK(btn_audio_tone_on(&a,&bank,0,1,60,0,127));
    CHECK(a.peak_voices==1&&a.voices[0].pitch==8192.0&&!a.voices[1].active);
}
static void sequence(void)
{
    uint8_t bytes[BANK_BYTES]; fixture(bytes); BtnAudioBank bank={0};
    CHECK(btn_audio_bank_open(&bank,bytes,sizeof(bytes)));
    const uint8_t seq[]={ 'p','Q','E','S',0,0,0,1,1,0xE0,7,0xA1,0x20,4,2,
        0,0xC0,0, 0,0x90,60,127, 0x83,0x60,60,0, 0,0xFF,0x2F,0 };
    BtnAudio a,b; BtnSequence s,t; btn_audio_init(&a); btn_audio_init(&b);
    CHECK(btn_sequence_open(&s,seq,sizeof(seq),false)); t=s;
    int16_t full[400],split[400];
    CHECK(btn_sequence_render(&s,&a,&bank,full,200));
    CHECK(btn_sequence_render(&t,&b,&bank,split,71));
    CHECK(btn_sequence_render(&t,&b,&bank,split+142,129));
    CHECK(!memcmp(full,split,sizeof(full))&&s.wait==21850);
    int16_t scratch[2048];
    for (unsigned i=0;i<22;++i) CHECK(btn_sequence_render(&s,&a,&bank,scratch,1024));
    CHECK(!s.playing);
    const uint8_t malformed[]={ 'p','Q','E','S',0,0,0,1,1,0,7,0xA1,0x20,4,2,0,0x90,60 };
    CHECK(btn_sequence_open(&s,malformed,sizeof(malformed),false));
    CHECK(!btn_sequence_render(&s,&a,&bank,scratch,1));
    const uint8_t tempo[]={ 'p','Q','E','S',0,0,0,1,1,0,7,0xA1,0x20,4,2,
        0,0xFF,0x51,3,0xD0,0x90, 1,0xFF,0x2F,0 };
    CHECK(btn_sequence_open(&s,tempo,sizeof(tempo),false));
    CHECK(btn_sequence_render(&s,&a,&bank,scratch,1)); CHECK(s.tempo==250000&&s.wait==42);
}
static void xa(void)
{
    uint8_t bytes[2352]={0}; bytes[0]=bytes[11]=0;
    memset(bytes+1,255,10); bytes[15]=2;
    bytes[16]=1; bytes[17]=3; bytes[18]=0x64; bytes[19]=1; memcpy(bytes+20,bytes+16,4);
    for (unsigned g=0;g<18;++g) {
        uint8_t *p=bytes+24+g*128; memset(p,0x0C,16); memset(p+16,0x21,112);
    }
    BtnXaSector sector; CHECK(btn_xa_sector(&sector,bytes,sizeof(bytes)));
    CHECK(sector.file==1&&sector.channel==3&&sector.rate==37800&&sector.channels==2);
    BtnXaDecoder d={0}; int16_t pcm[BTN_XA_MAX_FRAMES*2]; size_t frames=0;
    CHECK(btn_xa_decode(&d,&sector,pcm,BTN_XA_MAX_FRAMES,&frames)&&frames==2016);
    for (size_t i=0;i<frames;++i) CHECK(pcm[i*2]==1&&pcm[i*2+1]==2);
    CHECK(!btn_xa_decode(&d,&sector,pcm,2015,&frames));
    CHECK(btn_xa_sector(&sector,bytes+16,2336));
    CHECK(!btn_xa_sector(&sector,bytes,2048));
    bytes[20]^=1; CHECK(!btn_xa_sector(&sector,bytes,sizeof(bytes))); bytes[20]^=1;
    bytes[19]=bytes[23]=0x10; CHECK(btn_xa_sector(&sector,bytes,sizeof(bytes)));
    CHECK(!btn_xa_decode(&d,&sector,pcm,BTN_XA_MAX_FRAMES,&frames));
    bytes[19]=bytes[23]=4; CHECK(btn_xa_sector(&sector,bytes,sizeof(bytes)));
    d=(BtnXaDecoder){0}; CHECK(btn_xa_decode(&d,&sector,pcm,BTN_XA_MAX_FRAMES,&frames));
    CHECK(frames==4032&&d.rate==18900&&d.channel_count==1);
    CHECK(pcm[0]==1&&pcm[54]==1&&pcm[56]==2&&pcm[57]==2);
}
int main(void)
{
    adpcm(); mixer(); sequence(); xa(); puts("native audio checks passed"); return 0;
}
