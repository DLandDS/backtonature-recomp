#include "title_dialog.h"

void btn_dialog_reset(BtnTitleDialog *d) { *d=(BtnTitleDialog){0}; }

BtnDialogKind btn_dialog_for_setup(const BtnTitleSetup *s)
{
    if (s->stage==BTN_SETUP_DONE) return BTN_DIALOG_NONE;
    if (s->stage==BTN_SETUP_REVIEW) return BTN_DIALOG_REVIEW;
    if (s->stage==BTN_SETUP_BIRTHDAY_CONFIRM) return BTN_DIALOG_BIRTHDAY_CONFIRM;
    if (s->stage==BTN_SETUP_SEASON || s->stage==BTN_SETUP_DAY) return BTN_DIALOG_BIRTHDAY_HELP;
    return s->confirming ? BTN_DIALOG_NAME_CONFIRM : BTN_DIALOG_NAME_HELP;
}

bool btn_dialog_ready(const BtnTitleDialog *d, BtnDialogKind kind)
{
    return kind!=BTN_DIALOG_NONE && d->phase==BTN_DIALOG_READY && d->kind==kind;
}

void btn_dialog_tick(BtnTitleDialog *d, BtnDialogKind requested)
{
    if (d->phase==BTN_DIALOG_HIDDEN) {
        if (requested==BTN_DIALOG_NONE) return;
        d->kind=requested; d->phase=BTN_DIALOG_OPENING; d->frame=0;
    } else if (d->phase!=BTN_DIALOG_CLOSING && requested!=d->kind) {
        d->phase=BTN_DIALOG_CLOSING; d->frame=0;
    }
    if (d->phase==BTN_DIALOG_READY) return;
    bool opening=d->phase==BTN_DIALOG_OPENING;
    int duration=opening ? 12 : 6;
    /* Same integer divisions, centre and zero-height frame skip as the source.
     * A zero-width first frame has no raster output. */
    for (;;) {
        int t=opening ? (int)d->frame : duration-((int)d->frame-6);
        if (d->frame>=(unsigned)(duration+6)) {
            if (opening) { d->phase=BTN_DIALOG_READY; d->box=(BtnRect){26,164,267,62}; d->visible=true; }
            else btn_dialog_reset(d);
            return;
        }
        if (t<6) {
            int half=((267<<11)/6*t)/4096;
            d->box=(BtnRect){159-half,195,half*2,4};
        } else {
            int half=((62<<11)/duration*(t-6))/4096;
            if (half*2<3) { ++d->frame; continue; }
            d->box=(BtnRect){26,195-half,267,half*2};
        }
        d->visible=d->box.w>0;
        ++d->frame;
        return;
    }
}

int btn_dialog_setup_step(BtnTitleDialog *d, BtnTitleSetup *s,
    const BtnNameLayout *layout, const BtnInput *input)
{
    int sound=-1;
    /* Inputs on an opening/closing tick are discarded, never replayed on ready. */
    if (btn_dialog_ready(d,btn_dialog_for_setup(s))) sound=btn_setup_step(s,layout,input);
    else btn_setup_observe_input(s,input);
    btn_dialog_tick(d,btn_dialog_for_setup(s));
    return sound;
}
