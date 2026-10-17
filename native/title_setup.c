#include "title_setup.h"

static uint16_t *current_name(BtnTitleSetup *s)
{
    return s->stage==BTN_SETUP_PLAYER ? s->game.player
        : s->stage==BTN_SETUP_FARM ? s->game.farm : s->game.dog;
}

const uint16_t *btn_setup_name(const BtnTitleSetup *s)
{
    return s->stage==BTN_SETUP_PLAYER ? s->game.player
        : s->stage==BTN_SETUP_FARM ? s->game.farm : s->game.dog;
}

size_t btn_setup_summary(const BtnNewGame *game, const uint16_t *format, size_t count,
    uint16_t out[BTN_SETUP_SUMMARY_WORDS])
{
    if (!game || !format || !out || !count || count>BTN_SETUP_SUMMARY_FORMAT_WORDS) return 0;
    size_t n=0;
    for (size_t i=0; i<count; ++i) {
        uint16_t w=format[i];
        if (w==0xFFFF) { out[n++]=w; return n; }
        const uint16_t *name=w==0x400A ? game->player : w==0x400C ? game->dog : w==0x400D ? game->farm : NULL;
        if (name) {
            unsigned j=0;
            for (; j<9 && name[j]!=0xFFFF; ++j) {
                if (j==8 || (name[j]>255 && name[j]!=0xFFD) || n>=BTN_SETUP_SUMMARY_WORDS-1) return 0;
                out[n++]=name[j];
            }
            if (j==9) return 0;
        } else {
            if ((w>255 && w!=0xFFD && w!=0xFFFE) || n>=BTN_SETUP_SUMMARY_WORDS-1) return 0;
            out[n++]=w;
        }
    }
    return 0;
}

static void enter_name(BtnTitleSetup *s, BtnSetupStage stage)
{
    s->stage=stage; s->cell=s->shift=s->length=s->choice=0;
    s->dirty=s->confirming=false;
    uint16_t *name=current_name(s);
    for (unsigned i=0; i<9; ++i) name[i]=0xFFFF;
}

void btn_setup_reset(BtnTitleSetup *s)
{
    *s=(BtnTitleSetup){0};
    for (unsigned i=0; i<9; ++i) s->game.player[i]=s->game.farm[i]=s->game.dog[i]=0xFFFF;
    enter_name(s,BTN_SETUP_PLAYER);
}

bool btn_name_layout_valid(const BtnNameLayout *layout)
{
    if (!layout) return false;
    for (unsigned kind=0; kind<2; ++kind) {
        for (unsigned row=0; row<6; ++row) if (layout->row_last[kind][row]>13) return false;
        for (unsigned col=0; col<14; ++col) if (layout->column_last[kind][col]>5) return false;
    }
    for (unsigned i=0; i<28; ++i) if (layout->symbols[i]>255) return false;
    return true;
}

unsigned btn_name_move(const BtnNameLayout *layout, unsigned cell, unsigned shift, uint32_t direction)
{
    if (!layout || cell>=84 || shift>1) return cell;
    int col=(int)(cell%14), row=(int)(cell/14);
    bool forward=direction==BTN_BIT(BTN_RIGHT) || direction==BTN_BIT(BTN_DOWN);
    if (direction==BTN_BIT(BTN_LEFT) || direction==BTN_BIT(BTN_RIGHT)) {
        int limit=layout->row_last[shift][row];
        col+=forward ? 1 : -1;
        if (col<0) col=limit;
        if (col>limit) col=0;
        if (!shift && row==5) {
            if (col>=1 && col<=4) col=forward ? 5 : 0;
            else if (col>=6 && col<=12) col=forward ? 13 : 5;
        } else if (shift) {
            if (row<2 && col>=10 && col<=12) col=forward ? 13 : 9;
            if (row==5) col=13;
        }
    } else if (direction==BTN_BIT(BTN_UP) || direction==BTN_BIT(BTN_DOWN)) {
        int limit=layout->column_last[shift][col];
        row+=forward ? 1 : -1;
        if (row<0) row=limit;
        if (row>limit) row=0;
        if (col==13 && row>=2 && row<=4) row=forward ? 5 : 1;
    }
    return (unsigned)(row*14+col);
}

uint16_t btn_name_character(const BtnNameLayout *layout, unsigned cell, unsigned shift)
{
    if (!layout || cell>=84 || shift>1) return 0xFFFF;
    unsigned col=cell%14, row=cell/14;
    if (cell==27) return 0xFFD;
    if (cell==13 || cell==83) return 0xFFFF;
    if (col>layout->row_last[shift][row] || row>layout->column_last[shift][col]) return 0xFFFF;
    if (shift) return row*10+col<28 ? layout->symbols[row*10+col] : 0xFFFF;
    if (row==5 && col!=0 && col!=5) return 0xFFFF;
    return (uint16_t)(col<5 ? row*5+col : col<10 ? 0x20+row*5+col-5 : 0x40+row*3+col-10);
}

static uint32_t pad_repeat(BtnTitleSetup *s, const BtnInput *input)
{
    uint32_t result=0;
    /* pad_read_record: delay 20/2, rate 7/2; first repeat follows the
     * delay-state change and three further held ticks, not an OS key repeat. */
    if (input->held!=s->last_held || input->pressed) {
        s->repeat_ticks=0; s->repeating=false; result=input->pressed;
    } else if (!s->repeating) {
        if (++s->repeat_ticks>10) { s->repeat_ticks=0; s->repeating=true; }
    } else if (++s->repeat_ticks%3==0) result=input->held;
    s->last_held=input->held;
    return result;
}

void btn_setup_observe_input(BtnTitleSetup *s, const BtnInput *input)
{
    (void)pad_repeat(s,input);
}

static void name_accepted(BtnTitleSetup *s)
{
    s->confirming=false; s->choice=s->value=0;
    if (s->stage==BTN_SETUP_PLAYER) s->stage=BTN_SETUP_SEASON;
    else if (s->stage==BTN_SETUP_FARM) enter_name(s,BTN_SETUP_DOG);
    else s->stage=BTN_SETUP_REVIEW;
}

static int name_step(BtnTitleSetup *s, const BtnNameLayout *layout, uint32_t press, uint32_t direction)
{
    if (s->confirming) {
        if (press==BTN_BIT(BTN_A)) {
            if (!s->choice) { name_accepted(s); return 0; }
            s->confirming=false; return 1;
        }
        if (press==BTN_BIT(BTN_B)) { s->confirming=false; return 1; }
        if (direction & (BTN_BIT(BTN_UP)|BTN_BIT(BTN_DOWN))) { s->choice^=1; return 5; }
        return -1;
    }
    if (press==BTN_BIT(BTN_B)) {
        if (s->length) { current_name(s)[--s->length]=0xFFFF; return 1; }
        if (s->stage==BTN_SETUP_PLAYER) return 2;
        if (s->stage==BTN_SETUP_FARM) { s->stage=BTN_SETUP_SEASON; s->value=0; }
        else enter_name(s,BTN_SETUP_FARM);
        return 1;
    }
    if (press==BTN_BIT(BTN_START) || (press==BTN_BIT(BTN_A) && s->cell==83)) {
        if (!s->length || !s->dirty) return 2;
        s->confirming=true; s->choice=0; return 0;
    }
    if (press==BTN_BIT(BTN_A)) {
        if (s->cell==13) { s->shift^=1; return 5; }
        uint16_t ch=btn_name_character(layout,s->cell,s->shift);
        if (ch==0xFFFF) return 2;
        if (s->length<8) {
            uint16_t *name=current_name(s);
            name[s->length++]=ch; name[s->length]=0xFFFF;
            if (ch!=0xFFD) s->dirty=true;
        }
        return 0;
    }
    unsigned before=s->cell;
    s->cell=btn_name_move(layout,s->cell,s->shift,direction);
    return before!=s->cell ? 5 : -1;
}

int btn_setup_step(BtnTitleSetup *s, const BtnNameLayout *layout, const BtnInput *input)
{
    const uint32_t directions=BTN_BIT(BTN_UP)|BTN_BIT(BTN_DOWN)|BTN_BIT(BTN_LEFT)|BTN_BIT(BTN_RIGHT);
    uint32_t repeat=pad_repeat(s,input), press=input->pressed;
    uint32_t direction=repeat & ~directions ? 0 : repeat;
    if (s->stage==BTN_SETUP_PLAYER || s->stage==BTN_SETUP_FARM || s->stage==BTN_SETUP_DOG)
        return name_step(s,layout,press,direction);
    if (s->stage==BTN_SETUP_DONE) return -1;
    if (s->stage==BTN_SETUP_REVIEW) {
        /* text_choice_menu: direction takes precedence; only A confirms. */
        if (repeat & (BTN_BIT(BTN_UP)|BTN_BIT(BTN_DOWN))) { s->choice^=1; return 5; }
        if (press & BTN_BIT(BTN_A)) {
            if (s->choice) enter_name(s,BTN_SETUP_PLAYER);
            else { s->stage=BTN_SETUP_DONE; s->game.flags=0x2000; }
            return 0;
        }
        return -1;
    }
    if (s->stage==BTN_SETUP_BIRTHDAY_CONFIRM) {
        if (press==BTN_BIT(BTN_A) && !s->choice) {
            enter_name(s,BTN_SETUP_FARM);
            return 0;
        }
        if (press==BTN_BIT(BTN_B) || press==BTN_BIT(BTN_A)) {
            s->stage=BTN_SETUP_DAY; s->value=s->game.day;
            s->choice=0; return 1;
        }
        if (direction & (BTN_BIT(BTN_UP)|BTN_BIT(BTN_DOWN))) { s->choice^=1; return 5; }
        return -1;
    }
    if (press==BTN_BIT(BTN_A)) {
        if (s->stage==BTN_SETUP_SEASON) { s->game.season=(uint16_t)s->value; s->stage=BTN_SETUP_DAY; }
        else { s->game.day=(uint16_t)s->value; s->stage=BTN_SETUP_BIRTHDAY_CONFIRM; }
        s->value=s->choice=0; return 0;
    }
    if (press==BTN_BIT(BTN_B)) {
        if (s->stage==BTN_SETUP_SEASON) enter_name(s,BTN_SETUP_PLAYER);
        else { s->stage=BTN_SETUP_SEASON; s->value=s->game.season; }
        return 1;
    }
    unsigned before=s->value;
    if (s->stage==BTN_SETUP_SEASON) {
        if (direction==BTN_BIT(BTN_RIGHT)) s->value=(s->value+1)%4;
        if (direction==BTN_BIT(BTN_LEFT)) s->value=(s->value+3)%4;
    } else {
        int day=(int)s->value;
        if (direction==BTN_BIT(BTN_UP)) { day-=7; if (day<0) { day+=28; if (day<23) day+=7; } }
        if (direction==BTN_BIT(BTN_DOWN)) { day+=7; if (day>=30) { day-=28; if (day>=7) day-=7; } }
        if (direction==BTN_BIT(BTN_LEFT)) day=(day+29)%30;
        if (direction==BTN_BIT(BTN_RIGHT)) day=(day+1)%30;
        s->value=(unsigned)day;
    }
    return before!=s->value ? 5 : -1;
}
