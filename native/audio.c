#include "audio.h"
#include <SDL3/SDL.h>
#include <string.h>

static unsigned le16(const uint8_t *p) { return (unsigned)p[0] | (unsigned)p[1]<<8; }
static int clamp16(int n) { return n < -32768 ? -32768 : n > 32767 ? 32767 : n; }
/* Explicit arithmetic shift, including negative values, on all C11 hosts. */
static int shift_right(int n, unsigned bits)
{
    int divisor=1<<bits;
    return n>=0 ? n/divisor : -((-n+divisor-1)/divisor);
}
static int predict(BtnAdpcm *s,int sample,unsigned filter)
{
    static const int positive[5]={0,60,115,98,122}, negative[5]={0,0,-52,-55,-60};
    int n=clamp16(sample+shift_right(s->old*positive[filter]+s->older*negative[filter]+32,6));
    s->older=s->old; s->old=n; return n;
}
bool btn_adpcm_block(BtnAdpcm *state,const uint8_t block[16],int16_t pcm[28])
{
    if (!state||!block||!pcm||block[0]>>4>4||block[1]>7)
        return SDL_SetError("Unsupported SPU ADPCM header");
    unsigned shift=block[0]&15u, filter=block[0]>>4;
    if (shift>12) shift=9;
    for (unsigned i=0;i<28;++i) {
        int n=(block[2+i/2]>>((i%2)*4))&15;
        if (n>=8) n-=16;
        pcm[i]=(int16_t)predict(state,shift_right(n*4096,shift),filter);
    }
    return true;
}
bool btn_audio_bank_open(BtnAudioBank *bank,const void *bytes,size_t size)
{
    const uint8_t *b=bytes;
    if (!bank||!b||size<0x820||memcmp(b,"pBAV",4)||btn_read_le32(b+4)!=7)
        return SDL_SetError("Expected a version 7 VAB");
    BtnAudioBank parsed={0};
    parsed.data=b; parsed.size=size; parsed.programs=le16(b+18); parsed.samples=le16(b+22);
    if (!parsed.programs||parsed.programs>128||!parsed.samples||parsed.samples>255||b[24]>127||b[25]>127)
        return SDL_SetError("Invalid VAB counts or master volume/pan");
    parsed.body=0x820+parsed.programs*512u+512u;
    if (parsed.body>size) return SDL_SetError("Truncated VAB tables");
    unsigned active=0, tones=0;
    for (unsigned p=0;p<128;++p) {
        const uint8_t *prog=b+32+p*16;
        parsed.tone_slot[p]=-1;
        if (!prog[0]) continue;
        if (prog[0]>16||prog[1]>127||prog[4]>127||active>=parsed.programs)
            return SDL_SetError("Invalid VAB program");
        parsed.tone_slot[p]=(int)active;
        for (unsigned t=0;t<prog[0];++t) {
            const uint8_t *tone=b+0x820+active*512+t*32;
            if (tone[2]>127||tone[3]>127||tone[4]>127||tone[5]>127||tone[6]>tone[7]
                ||tone[7]>127||le16(tone+22)>parsed.samples||!le16(tone+22))
                return SDL_SetError("Invalid VAB tone");
            parsed.reverb|=(tone[1]&4u)!=0;
        }
        tones+=prog[0]; ++active;
    }
    if (active!=parsed.programs||tones!=le16(b+20)) return SDL_SetError("VAB program/tone count mismatch");
    size_t position=parsed.body, table=parsed.body-512;
    if (le16(b+table)) return SDL_SetError("VAB sample zero must be reserved");
    for (unsigned i=1;i<=parsed.samples;++i) {
        unsigned length=le16(b+table+i*2)*8u;
        if (!length||length%16||length>size-position||position>UINT32_MAX)
            return SDL_SetError("Invalid VAB sample span");
        parsed.sample_offset[i]=(uint32_t)position; parsed.sample_size[i]=length;
        bool end=false;
        for (unsigned j=0;j<length;j+=16) {
            const uint8_t *block=b+position+j;
            if (block[0]>>4>4||block[1]>7) return SDL_SetError("Invalid VAB ADPCM block");
            if (block[1]&1u) { end=true; break; }
        }
        if (!end) return SDL_SetError("VAB sample has no end block");
        position+=length;
    }
    for (unsigned i=parsed.samples+1;i<256;++i)
        if (le16(b+table+i*2)) return SDL_SetError("Unexpected VAB sample table tail");
    if (position!=btn_read_le32(b+12)) return SDL_SetError("VAB declared size disagrees with sample tables");
    *bank=parsed; return true;
}
void btn_audio_init(BtnAudio *audio)
{
    *audio=(BtnAudio){0}; audio->gain=1;
    for (unsigned c=0;c<16;++c) { audio->channels[c].volume=127; audio->channels[c].pan=64; }
}
static BtnAudioVoice *allocate_voice(BtnAudio *a,unsigned priority)
{
    BtnAudioVoice *choice=NULL;
    unsigned count=0;
    for (unsigned i=0;i<BTN_AUDIO_VOICES;++i) {
        BtnAudioVoice *v=&a->voices[i];
        if (!v->active) { if (!choice||choice->active) choice=v; continue; }
        ++count;
        if ((!choice||choice->active)&&(v->priority<=priority)
            &&(!choice||v->priority<choice->priority||(v->priority==choice->priority&&v->stamp<choice->stamp))) choice=v;
    }
    if (!choice) return NULL;
    if (choice->active) ++a->stolen; else ++count;
    if (count>a->peak_voices) a->peak_voices=count;
    *choice=(BtnAudioVoice){0}; choice->active=true; choice->cursor=28;
    choice->priority=priority; choice->stamp=++a->stamp; choice->bend=INT32_MAX;
    return choice;
}
bool btn_audio_sample_play(BtnAudio *a,const BtnAudioBank *b,unsigned sample,bool loop)
{
    if (!a||!b||sample<1||sample>b->samples) return SDL_SetError("VAB sample index out of range");
    BtnAudioVoice *v=allocate_voice(a,127);
    if (!v) return SDL_SetError("No audio voice available");
    v->data=b->data+b->sample_offset[sample]; v->size=b->sample_size[sample];
    v->step=4096; v->pitch=4096; v->level=32767; v->left=v->right=1;
    v->repeating=loop; v->raw=true; return true;
}
bool btn_audio_note_on(BtnAudio *a,const BtnAudioBank *b,unsigned channel,unsigned note,unsigned velocity)
{
    if (!a||!b||channel>15||note>127||velocity>127) return SDL_SetError("Invalid note parameters");
    if (!velocity) { btn_audio_note_off(a,channel,note); return true; }
    unsigned p=a->channels[channel].program;
    if (p>127||b->tone_slot[p]<0) return SDL_SetError("Sequence program %u absent from selected bank",p);
    const uint8_t *prog=b->data+32+p*16;
    for (unsigned t=0;t<prog[0];++t) {
        const uint8_t *tone=b->data+0x820+(size_t)b->tone_slot[p]*512+t*32;
        if (note<tone[6]||note>tone[7]) continue;
        BtnAudioVoice *v=allocate_voice(a,tone[0]);
        if (!v) continue; /* A higher-priority voice legitimately wins. */
        unsigned sample=le16(tone+22);
        v->data=b->data+b->sample_offset[sample]; v->size=b->sample_size[sample];
        v->channel=channel; v->note=note; v->repeating=true;
        v->adsr1=(uint16_t)le16(tone+16); v->adsr2=(uint16_t)le16(tone+18);
        v->pitch=4096.0*SDL_pow(2.0,((double)note-tone[4]-(double)tone[5]/128.0)/12.0);
        float volume=(float)velocity/127*(float)b->data[24]/127*(float)prog[1]/127*(float)tone[2]/127;
        int pan=(int)tone[3]+prog[4]+b->data[25]-128;
        if (pan<0) pan=0;
        if (pan>127) pan=127;
        v->left=volume*(pan>64?(float)(127-pan)/63:1);
        v->right=volume*(pan<64?(float)pan/64:1);
    }
    return true;
}
void btn_audio_note_off(BtnAudio *a,unsigned channel,unsigned note)
{
    for (unsigned i=0;i<BTN_AUDIO_VOICES;++i) {
        BtnAudioVoice *v=&a->voices[i];
        if (v->active&&v->channel==channel&&v->note==note) { v->stage=3; v->counter=0; }
    }
}
void btn_audio_stop(BtnAudio *a)
{
    for (unsigned i=0;i<BTN_AUDIO_VOICES;++i) a->voices[i].active=false;
}
bool btn_audio_volume(BtnAudio *a,float gain,uint32_t fade_frames)
{
    if (!a||!(gain>=0&&gain<=1)) return SDL_SetError("Volume must be 0..1");
    /* Fade is a stop operation; independent volume changes are immediate. */
    a->gain=gain; a->fade_start=gain; a->fade_left=a->fade_total=fade_frames; return true;
}
static void envelope(BtnAudioVoice *v)
{
    if (v->raw) return;
    unsigned shift, step;
    bool down=false, exponential=false;
    switch (v->stage) {
    case 0: shift=(v->adsr1>>10)&31u; step=(v->adsr1>>8)&3u; exponential=(v->adsr1&0x8000u)!=0; break;
    case 1: shift=(v->adsr1>>4)&15u; step=0; down=exponential=true; break;
    case 2: shift=(v->adsr2>>8)&31u; step=(v->adsr2>>6)&3u; down=(v->adsr2&0x4000u)!=0; exponential=(v->adsr2&0x8000u)!=0; break;
    default: shift=v->adsr2&31u; step=0; down=true; exponential=(v->adsr2&32u)!=0; break;
    }
    unsigned increment=32768u>>(shift>11?shift-11:0);
    int delta=down?(-8+(int)step):(7-(int)step);
    delta*=1<<(shift<11?11-shift:0);
    if (exponential&&down) delta=shift_right(delta*(int)v->level,15);
    if (exponential&&!down&&v->level>0x6000) {
        if (shift<10) delta=shift_right(delta,2);
        else if (shift>=11) increment/=4;
        else { delta=shift_right(delta,1); increment/=2; }
    }
    bool frozen=(v->stage==0||v->stage==2)?(shift==31&&step==3):(shift==31);
    if (!increment&&!frozen) increment=1;
    v->counter+=increment;
    if (v->counter<32768) return;
    v->counter-=32768;
    int next=(int)v->level+delta;
    v->level=(unsigned)(next<0?0:next>32767?32767:next);
    if (v->stage==0&&v->level==32767) { v->stage=1; v->counter=0; }
    else if (v->stage==1&&v->level<=((v->adsr1&15u)+1)*2048u) { v->stage=2; v->counter=0; }
    else if (v->stage==3&&!v->level) v->active=false;
}
static bool next_sample(BtnAudioVoice *v)
{
    if (v->cursor<28) return true;
    if (v->ended) {
        if (!v->repeating) { v->active=false; return true; }
        v->block=v->loop;
    }
    if (v->block>v->size||v->size-v->block<16) return SDL_SetError("Voice read exceeds ADPCM sample");
    const uint8_t *block=v->data+v->block;
    if (block[1]&4u) v->loop=v->block;
    v->ended=(block[1]&1u)!=0;
    if (v->ended&&!(block[1]&2u)) v->repeating=false;
    if (!btn_adpcm_block(&v->decoder,block,v->pcm)) return false;
    v->block+=16; v->cursor=0; return true;
}
bool btn_audio_render(BtnAudio *a,int16_t *stereo,size_t frames)
{
    if (!a||(!stereo&&frames)) return SDL_SetError("Invalid audio output buffer");
    for (size_t f=0;f<frames;++f) {
        float left=0,right=0;
        for (unsigned i=0;i<BTN_AUDIO_VOICES;++i) {
            BtnAudioVoice *v=&a->voices[i];
            if (!v->active) continue;
            if (!next_sample(v)) return false;
            if (!v->active) continue;
            envelope(v);
            const BtnAudioChannel *c=&a->channels[v->channel];
            float level=(float)v->level/32767;
            unsigned step=v->step;
            if (!v->raw) {
                if (v->bend!=c->bend) {
                    double pitch=v->pitch*SDL_pow(2.0,(double)c->bend/(8192.0*6.0));
                    v->step=(unsigned)(pitch<1?1:pitch>16383?16383:pitch); v->bend=c->bend;
                }
                step=v->step;
                level*=(float)c->volume/127;
            }
            float l=v->left,r=v->right;
            if (!v->raw) {
                if (c->pan>64) l*=(float)(127-c->pan)/63;
                if (c->pan<64) r*=(float)c->pan/64;
            }
            left+=v->pcm[v->cursor]*level*l; right+=v->pcm[v->cursor]*level*r;
            v->phase+=step;
            while (v->phase>=4096&&v->active) {
                v->phase-=4096; ++v->cursor;
                if (!next_sample(v)) return false;
            }
        }
        float gain=a->gain;
        if (a->fade_left) {
            gain=a->fade_start*(float)a->fade_left/(float)a->fade_total;
            if (!--a->fade_left) { btn_audio_stop(a); a->gain=0; }
        }
        float l=left*gain,r=right*gain;
        stereo[f*2]=(int16_t)(l<-32768?-32768:l>32767?32767:l);
        stereo[f*2+1]=(int16_t)(r<-32768?-32768:r>32767?32767:r);
    }
    return true;
}

static bool vlq(BtnSequence *s,uint32_t *result)
{
    *result=0;
    for (unsigned n=0;n<4;++n) {
        if (s->cursor>=s->size) return SDL_SetError("Truncated SEQ delta");
        uint8_t b=s->data[s->cursor++]; *result=(*result<<7)|(b&127u);
        if (!(b&128u)) return true;
    }
    return SDL_SetError("SEQ delta exceeds four bytes");
}
static bool schedule(BtnSequence *s)
{
    uint32_t delta;
    if (!vlq(s,&delta)) return false;
    /* Divide before multiplying by rate to avoid overflow at the VLQ limit. */
    uint64_t micros=(uint64_t)delta*s->tempo, denominator=(uint64_t)s->resolution*1000000u;
    uint64_t whole=micros/denominator, part=(micros%denominator)*BTN_AUDIO_RATE+s->fraction;
    s->wait=whole*BTN_AUDIO_RATE+part/denominator; s->fraction=part%denominator; return true;
}
bool btn_sequence_open(BtnSequence *s,const void *bytes,size_t size,bool repeat)
{
    const uint8_t *b=bytes;
    if (!s||!b||size<16||memcmp(b,"pQES\0\0\0\1",8)) return SDL_SetError("Expected SEQ version 1");
    BtnSequence parsed={0}; parsed.data=b; parsed.size=size; parsed.cursor=15;
    parsed.resolution=(unsigned)b[8]*256+b[9];
    parsed.tempo=(uint32_t)b[10]*65536u+(uint32_t)b[11]*256u+b[12];
    parsed.playing=true; parsed.repeat=repeat;
    if (!parsed.resolution||parsed.resolution>32767||!parsed.tempo) return SDL_SetError("Invalid SEQ resolution/tempo");
    if (!schedule(&parsed)) return false;
    *s=parsed; return true;
}
static bool event(BtnSequence *s,BtnAudio *a,const BtnAudioBank *bank)
{
    if (s->cursor>=s->size) return SDL_SetError("SEQ has no end event");
    unsigned status=s->data[s->cursor];
    if (status>=128) { ++s->cursor; if (status<240) s->running=status; }
    else status=s->running;
    if (status==255) {
        if (s->cursor>=s->size) return SDL_SetError("Truncated SEQ meta event");
        unsigned type=s->data[s->cursor++];
        if (type==47) {
            if (s->cursor>=s->size||s->data[s->cursor++]!=0) return SDL_SetError("Invalid SEQ end event");
            s->playing=false;
            for (unsigned c=0;c<16;++c) for (unsigned n=0;n<128;++n) btn_audio_note_off(a,c,n);
            return true;
        }
        if (type!=81||s->size-s->cursor<3) return SDL_SetError("Unsupported/truncated SEQ meta event %u",type);
        /* Sony SEQ tempo is three bytes without SMF's length byte. */
        const uint8_t *b=s->data+s->cursor; s->cursor+=3;
        s->tempo=(uint32_t)b[0]*65536u+(uint32_t)b[1]*256u+b[2];
        if (!s->tempo) return SDL_SetError("Zero SEQ tempo");
        return schedule(s);
    }
    unsigned kind=status>>4, channel=status&15u, count=kind==12?1:2;
    if (status<128||status>=240||!(kind==8||kind==9||kind==11||kind==12||kind==14)||s->size-s->cursor<count)
        return SDL_SetError("Unsupported/truncated SEQ status %02X",status);
    const uint8_t *b=s->data+s->cursor;
    if (b[0]>127||(count==2&&b[1]>127)) return SDL_SetError("Invalid SEQ data byte");
    s->cursor+=count;
    BtnAudioChannel *c=&a->channels[channel];
    if (kind==8) btn_audio_note_off(a,channel,b[0]);
    else if (kind==9) { if (!btn_audio_note_on(a,bank,channel,b[0],b[1])) return false; }
    else if (kind==12) c->program=b[0];
    else if (kind==14) c->bend=(int)b[0]+(int)b[1]*128-8192;
    else switch (b[0]) {
    case 7: c->volume=b[1]; break;
    case 10: c->pan=b[1]; break;
    case 99:
        if (b[1]!=20&&b[1]!=30) return SDL_SetError("Unsupported SEQ NRPN %u",b[1]);
        c->nrpn=b[1];
        if (b[1]==30&&s->repeat&&s->loop_cursor&&s->repeats) {
            s->cursor=s->loop_cursor; s->running=s->loop_running;
            if (s->repeats!=127) --s->repeats;
        }
        break;
    case 6:
        if (c->nrpn!=20) return SDL_SetError("Unsupported SEQ data entry");
        s->loop_cursor=s->cursor; s->loop_running=s->running; s->repeats=b[1]; break;
    default: return SDL_SetError("Unsupported SEQ controller %u",b[0]);
    }
    return schedule(s);
}
bool btn_sequence_render(BtnSequence *s,BtnAudio *a,const BtnAudioBank *bank,int16_t *out,size_t frames)
{
    if (!s||!a||!bank||(!out&&frames)) return SDL_SetError("Invalid sequence render arguments");
    for (size_t f=0;f<frames;++f) {
        unsigned budget=4096;
        while (s->playing&&!s->wait) {
            if (!budget--) return SDL_SetError("SEQ zero-time event limit exceeded");
            if (!event(s,a,bank)) return false;
        }
        if (!btn_audio_render(a,out+f*2,1)) return false;
        if (s->wait) --s->wait;
    }
    return true;
}
