#include "title_speech.h"
#include <string.h>

bool btn_speech_message(BtnSpeechMessage *out, const uint16_t *w, size_t count, unsigned portrait)
{
    if (!out || !w || count<4 || count>BTN_SPEECH_WORDS || portrait>0x1B7) return false;
    unsigned column=0, lines=1, pages=1;
    bool prefix=true;
    for (size_t i=0; i<count; ++i) {
        if (prefix) {
            if (count-i<4 || w[i]!=0xFEF-portrait || w[i+1]!=0xFF7 || w[i+2]!=0xFF4) return false;
            i+=2; prefix=false; continue;
        }
        if (w[i]==0xFFFF) {
            BtnSpeechMessage result={0}; result.count=(unsigned)i+1;
            memcpy(result.words,w,result.count*sizeof(*w)); *out=result; return true;
        }
        if (w[i]==0xFFC) {
            if (++pages>2 || count-i<7 || w[i+1]!=0xFFB || w[i+2]!=0xFFFE) return false;
            i+=2; prefix=true; column=0; lines=1; continue;
        }
        if (w[i]==0xFFFE) { column=0; if (++lines>4) return false; }
        else if (w[i]<=255 || w[i]==0xFFD) { if (++column>32) return false; }
        else return false;
    }
    return false;
}

static void page_begin(BtnTitleSpeech *s, const BtnSpeechMessage *text)
{
    s->shown=s->elapsed=0; s->waiting=s->started=false;
    s->end=s->start;
    while (s->end<text->count && text->words[s->end]!=0xFFC && text->words[s->end]!=0xFFFF) ++s->end;
}

void btn_speech_begin(BtnTitleSpeech *s, unsigned message, const BtnSpeechMessage *text)
{
    *s=(BtnTitleSpeech){0}; s->active=true; s->message=message; page_begin(s,text);
}

int btn_speech_step(BtnTitleSpeech *s, const BtnSpeechMessage *text, const BtnInput *input, unsigned interval)
{
    if (!s->active || s->closing) return -1;
    uint32_t accept=BTN_BIT(BTN_A)|BTN_BIT(BTN_B);
    if (s->waiting) {
        if (input->pressed&accept) {
            if (text->words[s->end]==0xFFFF) s->closing=true;
            else { s->start=s->end+3; page_begin(s,text); }
        }
        return -1;
    }
    if (s->started) s->elapsed+=2;
    else s->started=true;
    unsigned speed=(input->held&accept) ? 0u : interval;
    bool due=s->elapsed>=speed;
    int sound=-1;
    if (due || text->words[s->start+s->shown]>0xE00) {
        if (due) {
            s->elapsed=0;
            if (s->blip) sound=4;
            s->blip=!s->blip;
        }
        ++s->shown;
        s->waiting=s->start+s->shown>s->end;
    }
    return sound;
}

bool btn_speech_face(const BtnTitleSpeech *s, const BtnSpeechMessage *text)
{
    return (s->start || s->shown>=3) && text->words[s->start+2]==0xFF4;
}

size_t btn_speech_visible(const BtnTitleSpeech *s, const BtnSpeechMessage *text, uint16_t out[BTN_SPEECH_WORDS])
{
    size_t n=0;
    for (unsigned i=s->start; i<s->start+s->shown && i<s->end; ++i)
        if (text->words[i]<=255 || text->words[i]==0xFFD || text->words[i]==0xFFFE) out[n++]=text->words[i];
    out[n++]=0xFFFF; return n;
}
