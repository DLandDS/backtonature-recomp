#include "title.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr,"line %d: %s\n",__LINE__,#condition); exit(1); \
} } while (0)

static int press(BtnTitleState *state, unsigned action)
{
    BtnInput input={0}; input.pressed=BTN_BIT(action); input.held=input.pressed;
    return btn_title_step(state,&input);
}

static void advance(BtnTitleState *state, unsigned ticks)
{
    BtnInput input={0};
    for (unsigned i=0; i<ticks; ++i) (void)btn_title_step(state,&input);
}

static BtnNameLayout layout(void)
{
    BtnNameLayout l={
        {{13,13,12,10,9,13},{13,13,7,0,0,13}},
        {{5,4,4,4,4,5,4,4,4,4,3,2,2,5},{2,2,2,2,2,2,2,2,1,1,0,0,0,5}}, {0}};
    for (unsigned i=0; i<28; ++i) l.symbols[i]=(uint16_t)(0x60+i);
    return l;
}

static int setup_press(BtnTitleSetup *s, const BtnNameLayout *l, unsigned action)
{
    BtnInput input={0}; input.pressed=input.held=BTN_BIT(action);
    return btn_setup_step(s,l,&input);
}

static void speech_checks(void)
{
    const uint16_t words[]={0xF87,0xFF7,0xFF4,0,1,0xFFC,0xFFB,0xFFFE,0xF87,0xFF7,0xFF4,2,0xFFFF};
    BtnSpeechMessage m;
    CHECK(btn_speech_message(&m,words,sizeof(words)/sizeof(*words),104));
    uint16_t broken[sizeof(words)/sizeof(*words)];
    for (unsigned i=0; i<4; ++i) {
        memcpy(broken,words,sizeof(words));
        broken[i==0 ? 0 : i==1 ? 3 : i==2 ? 6 : 12]=i==0 ? 0xF88 : i==1 ? 0x400A : 0;
        CHECK(!btn_speech_message(&m,broken,sizeof(broken)/sizeof(*broken),104));
    }
    CHECK(!btn_speech_message(&m,words,3,104));
    CHECK(!btn_speech_message(&m,words,BTN_SPEECH_WORDS+1,104));
    uint16_t wide[37]={0xF87,0xFF7,0xFF4}; wide[36]=0xFFFF;
    CHECK(!btn_speech_message(&m,wide,37,104));
    const uint16_t tall[]={0xF87,0xFF7,0xFF4,0xFFFE,0xFFFE,0xFFFE,0xFFFE,0xFFFF};
    CHECK(!btn_speech_message(&m,tall,8,104));
    CHECK(btn_speech_message(&m,words,sizeof(words)/sizeof(*words),104));
    BtnTitleSpeech s; BtnInput none={0},held={0},tap={0};
    held.held=BTN_BIT(BTN_A); tap=held; tap.pressed=tap.held;
    btn_speech_begin(&s,0,&m);
    CHECK(s.active && !s.waiting && !s.shown);
    for (unsigned i=0; i<3; ++i) (void)btn_speech_step(&s,&m,&none,3);
    CHECK(s.shown==3 && btn_speech_face(&s,&m));
    (void)btn_speech_step(&s,&m,&none,3); CHECK(s.shown==3);
    CHECK(btn_speech_step(&s,&m,&none,3)==4 && s.shown==4);
    (void)btn_speech_step(&s,&m,&held,3); CHECK(s.shown==5);
    (void)btn_speech_step(&s,&m,&held,3); CHECK(s.waiting && !s.closing);
    (void)btn_speech_step(&s,&m,&held,3); CHECK(!s.start && s.waiting);
    uint16_t visible[BTN_SPEECH_WORDS];
    CHECK(btn_speech_visible(&s,&m,visible)==3 && visible[0]==0 && visible[1]==1 && visible[2]==0xFFFF);
    (void)btn_speech_step(&s,&m,&tap,3);
    CHECK(s.start==8 && !s.waiting && !s.shown && btn_speech_face(&s,&m));
    for (unsigned i=0; i<5; ++i) (void)btn_speech_step(&s,&m,&held,3);
    CHECK(s.waiting && !s.closing);
    (void)btn_speech_step(&s,&m,&held,3); CHECK(!s.closing);
    tap.pressed=tap.held=BTN_BIT(BTN_B);
    (void)btn_speech_step(&s,&m,&tap,3); CHECK(s.closing);
    CHECK(btn_speech_visible(&s,&m,visible)==2 && visible[0]==2 && visible[1]==0xFFFF);
    btn_speech_begin(&s,1,&m); CHECK(s.active && !s.closing && !s.start && !s.blip);
}

static void dialog_checks(void)
{
    BtnTitleDialog d; btn_dialog_reset(&d);
    btn_dialog_tick(&d,BTN_DIALOG_NONE);
    CHECK(d.phase==BTN_DIALOG_HIDDEN && !d.visible);
    for (unsigned tick=0; tick<18; ++tick) {
        btn_dialog_tick(&d,BTN_DIALOG_NAME_HELP);
        if (!tick) CHECK(!d.visible && d.phase==BTN_DIALOG_OPENING);
        if (tick==1) CHECK(d.box.x==137 && d.box.y==195 && d.box.w==44 && d.box.h==4);
        if (tick==5) CHECK(d.box.x==48 && d.box.w==222 && d.box.h==4);
        if (tick==6) CHECK(d.box.x==26 && d.box.y==193 && d.box.w==267 && d.box.h==4 && d.frame==8);
        if (tick<17) CHECK(!btn_dialog_ready(&d,BTN_DIALOG_NAME_HELP));
    }
    CHECK(btn_dialog_ready(&d,BTN_DIALOG_NAME_HELP));
    CHECK(d.box.x==26 && d.box.y==164 && d.box.w==267 && d.box.h==62);
    btn_dialog_tick(&d,BTN_DIALOG_NAME_CONFIRM);
    CHECK(d.phase==BTN_DIALOG_CLOSING && d.box.y==165 && d.box.h==60);
    for (unsigned i=1; i<12; ++i) btn_dialog_tick(&d,BTN_DIALOG_NAME_CONFIRM);
    CHECK(d.phase==BTN_DIALOG_HIDDEN && !d.visible);
    for (unsigned i=0; i<18; ++i) btn_dialog_tick(&d,BTN_DIALOG_NAME_CONFIRM);
    CHECK(btn_dialog_ready(&d,BTN_DIALOG_NAME_CONFIRM));

    BtnNameLayout l=layout(); BtnTitleSetup s; btn_setup_reset(&s); btn_dialog_reset(&d);
    BtnInput a={0}; a.pressed=a.held=BTN_BIT(BTN_A);
    for (unsigned i=0; i<18; ++i)
        CHECK(btn_dialog_setup_step(&d,&s,&l,&a)==-1 && !s.length && !s.confirming);
    CHECK(btn_dialog_ready(&d,BTN_DIALOG_NAME_HELP));
    BtnInput held=a; held.pressed=0;
    CHECK(btn_dialog_setup_step(&d,&s,&l,&held)==-1 && !s.length);
    CHECK(btn_dialog_setup_step(&d,&s,&l,&a)==0 && s.length==1);
    BtnInput start={0}; start.pressed=start.held=BTN_BIT(BTN_START);
    CHECK(btn_dialog_setup_step(&d,&s,&l,&start)==0 && s.confirming && d.phase==BTN_DIALOG_CLOSING);
    for (unsigned i=0; i<29; ++i)
        CHECK(btn_dialog_setup_step(&d,&s,&l,&a)==-1 && s.confirming && s.stage==BTN_SETUP_PLAYER);
    CHECK(btn_dialog_ready(&d,BTN_DIALOG_NAME_CONFIRM));
    CHECK(btn_dialog_setup_step(&d,&s,&l,&held)==-1 && s.confirming);
    BtnInput down={0}; down.pressed=down.held=BTN_BIT(BTN_DOWN);
    CHECK(btn_dialog_setup_step(&d,&s,&l,&down)==5 && s.choice==1);
    CHECK(btn_dialog_setup_step(&d,&s,&l,&a)==1 && !s.confirming && s.length==1);
    BtnInput none={0};
    for (unsigned i=0; i<29; ++i) (void)btn_dialog_setup_step(&d,&s,&l,&none);
    CHECK(btn_dialog_ready(&d,BTN_DIALOG_NAME_HELP));
    CHECK(btn_dialog_setup_step(&d,&s,&l,&start)==0 && s.confirming);
    for (unsigned i=0; i<29; ++i) (void)btn_dialog_setup_step(&d,&s,&l,&none);
    CHECK(btn_dialog_setup_step(&d,&s,&l,&a)==0 && s.stage==BTN_SETUP_SEASON);
    for (unsigned i=0; i<29; ++i) (void)btn_dialog_setup_step(&d,&s,&l,&none);
    CHECK(btn_dialog_ready(&d,BTN_DIALOG_BIRTHDAY_HELP));
    CHECK(btn_dialog_setup_step(&d,&s,&l,&a)==0 && s.stage==BTN_SETUP_DAY && d.phase==BTN_DIALOG_READY);
    CHECK(btn_dialog_setup_step(&d,&s,&l,&a)==0 && s.stage==BTN_SETUP_BIRTHDAY_CONFIRM);
    for (unsigned i=0; i<29; ++i) (void)btn_dialog_setup_step(&d,&s,&l,&none);
    CHECK(btn_dialog_ready(&d,BTN_DIALOG_BIRTHDAY_CONFIRM));
    CHECK(btn_dialog_setup_step(&d,&s,&l,&a)==0 && s.stage==BTN_SETUP_FARM);

    btn_setup_reset(&s); s.stage=BTN_SETUP_REVIEW; btn_dialog_reset(&d);
    for (unsigned i=0; i<18; ++i) (void)btn_dialog_setup_step(&d,&s,&l,&none);
    CHECK(btn_dialog_setup_step(&d,&s,&l,&a)==0 && s.stage==BTN_SETUP_DONE && d.phase==BTN_DIALOG_CLOSING);
    for (unsigned i=0; i<10; ++i) {
        (void)btn_dialog_setup_step(&d,&s,&l,&none);
        CHECK(d.phase==BTN_DIALOG_CLOSING);
    }
    (void)btn_dialog_setup_step(&d,&s,&l,&none);
    CHECK(d.phase==BTN_DIALOG_HIDDEN);
    BtnTitle title={0}; title.setup=s; title.state.page=BTN_TITLE_READY; title.dialog=d;
    CHECK(btn_title_new_game(&title));
    title.dialog.phase=BTN_DIALOG_CLOSING; CHECK(!btn_title_new_game(&title));
    btn_dialog_reset(&title.dialog); CHECK(!title.dialog.visible && !title.dialog.frame);
    title.speech.active=true; CHECK(!btn_title_new_game(&title));

    btn_setup_reset(&s); btn_dialog_reset(&d);
    BtnInput right={0}; right.held=BTN_BIT(BTN_RIGHT);
    for (unsigned i=0; i<18; ++i) {
        right.pressed=i ? 0 : right.held;
        CHECK(btn_dialog_setup_step(&d,&s,&l,&right)==-1 && !s.cell);
    }
    right.pressed=0;
    CHECK(btn_dialog_setup_step(&d,&s,&l,&right)==-1 && !s.cell);
    CHECK(btn_dialog_setup_step(&d,&s,&l,&right)==-1 && !s.cell);
    CHECK(btn_dialog_setup_step(&d,&s,&l,&right)==5 && s.cell==1);
}

static void enter_one_name(BtnTitleSetup *s, const BtnNameLayout *l)
{
    CHECK(setup_press(s,l,BTN_A)==0);
    CHECK(setup_press(s,l,BTN_START)==0 && s->confirming);
    CHECK(setup_press(s,l,BTN_A)==0 && !s->confirming);
}

static void setup_checks(void)
{
    BtnNameLayout l=layout(); CHECK(btn_name_layout_valid(&l));
    CHECK(btn_name_character(&l,0,0)==0 && btn_name_character(&l,5,0)==0x20);
    CHECK(btn_name_character(&l,70,0)==25 && btn_name_character(&l,75,0)==0x39);
    CHECK(btn_name_character(&l,27,0)==0xFFD && btn_name_character(&l,83,0)==0xFFFF);
    CHECK(btn_name_character(&l,0,1)==0x60 && btn_name_character(&l,82,1)==0xFFFF);
    CHECK(btn_name_move(&l,70,0,BTN_BIT(BTN_RIGHT))==75);
    CHECK(btn_name_move(&l,75,0,BTN_BIT(BTN_RIGHT))==83);
    CHECK(btn_name_move(&l,13,0,BTN_BIT(BTN_UP))==83);
    CHECK(btn_name_move(&l,27,0,BTN_BIT(BTN_DOWN))==83);
    for (unsigned kind=0; kind<2; ++kind) for (unsigned cell=0; cell<84; ++cell) {
        if (btn_name_character(&l,cell,kind)==0xFFFF && cell!=13 && cell!=27 && cell!=83) continue;
        for (unsigned dir=BTN_UP; dir<=BTN_RIGHT; ++dir) {
            unsigned next=btn_name_move(&l,cell,kind,BTN_BIT(dir));
            CHECK(next<84 && (btn_name_character(&l,next,kind)!=0xFFFF || next==13 || next==27 || next==83));
        }
    }
    BtnTitleSetup s; btn_setup_reset(&s);
    CHECK(s.stage==BTN_SETUP_PLAYER && s.game.player[8]==0xFFFF && s.game.farm[0]==0xFFFF);
    CHECK(setup_press(&s,&l,BTN_START)==2 && !s.confirming);
    CHECK(setup_press(&s,&l,BTN_B)==2 && s.stage==BTN_SETUP_PLAYER);
    s.cell=27; CHECK(setup_press(&s,&l,BTN_A)==0 && s.length==1 && !s.dirty);
    CHECK(setup_press(&s,&l,BTN_START)==2 && !s.confirming);
    CHECK(setup_press(&s,&l,BTN_B)==1 && !s.length);
    s.cell=0;
    for (unsigned i=0; i<10; ++i) CHECK(setup_press(&s,&l,BTN_A)==0);
    CHECK(s.length==8 && s.game.player[8]==0xFFFF && s.game.farm[0]==0xFFFF);
    CHECK(setup_press(&s,&l,BTN_START)==0 && s.confirming);
    CHECK(setup_press(&s,&l,BTN_DOWN)==5 && s.choice==1);
    CHECK(setup_press(&s,&l,BTN_A)==1 && !s.confirming && s.length==8);
    CHECK(setup_press(&s,&l,BTN_START)==0);
    CHECK(setup_press(&s,&l,BTN_A)==0 && s.stage==BTN_SETUP_SEASON);
    CHECK(setup_press(&s,&l,BTN_LEFT)==5 && s.value==3);
    CHECK(setup_press(&s,&l,BTN_A)==0 && s.stage==BTN_SETUP_DAY && s.game.season==3);
    CHECK(setup_press(&s,&l,BTN_UP)==5 && s.value==28);
    CHECK(setup_press(&s,&l,BTN_DOWN)==5 && !s.value);
    CHECK(setup_press(&s,&l,BTN_LEFT)==5 && s.value==29);
    CHECK(setup_press(&s,&l,BTN_A)==0 && s.stage==BTN_SETUP_BIRTHDAY_CONFIRM && s.game.day==29);
    CHECK(setup_press(&s,&l,BTN_B)==1 && s.stage==BTN_SETUP_DAY && s.value==29);
    CHECK(setup_press(&s,&l,BTN_A)==0);
    CHECK(setup_press(&s,&l,BTN_A)==0 && s.stage==BTN_SETUP_FARM);
    CHECK(setup_press(&s,&l,BTN_B)==1 && s.stage==BTN_SETUP_SEASON);
    CHECK(setup_press(&s,&l,BTN_B)==1 && s.stage==BTN_SETUP_PLAYER && !s.length);
    enter_one_name(&s,&l); CHECK(s.stage==BTN_SETUP_SEASON);
    CHECK(setup_press(&s,&l,BTN_A)==0);
    CHECK(setup_press(&s,&l,BTN_A)==0);
    CHECK(setup_press(&s,&l,BTN_A)==0 && s.stage==BTN_SETUP_FARM);
    enter_one_name(&s,&l); CHECK(s.stage==BTN_SETUP_DOG);
    CHECK(setup_press(&s,&l,BTN_B)==1 && s.stage==BTN_SETUP_FARM && !s.length);
    enter_one_name(&s,&l); enter_one_name(&s,&l);
    CHECK(s.stage==BTN_SETUP_REVIEW && s.game.player[0]==0 && s.game.farm[0]==0 && s.game.dog[0]==0);
    CHECK(setup_press(&s,&l,BTN_A)==0 && s.stage==BTN_SETUP_DONE && s.game.flags==0x2000);
    BtnTitle title={0}; title.setup=s; title.state.page=BTN_TITLE_READY;
    CHECK(btn_title_new_game(&title)==&title.setup.game);
    title.state.pending=true; CHECK(!btn_title_new_game(&title));
    title.state.pending=false; title.state.fade_level=1; CHECK(!btn_title_new_game(&title));
    btn_setup_reset(&s);
    CHECK(setup_press(&s,&l,BTN_RIGHT)==5 && s.cell==1);
    BtnInput held={0}; held.held=BTN_BIT(BTN_RIGHT);
    for (unsigned i=1; i<14; ++i) CHECK(btn_setup_step(&s,&l,&held)==-1 && s.cell==1);
    CHECK(btn_setup_step(&s,&l,&held)==5 && s.cell==2);
    for (unsigned day=0; day<30; ++day) for (unsigned dir=BTN_UP; dir<=BTN_RIGHT; ++dir) {
        s.stage=BTN_SETUP_DAY; s.value=day;
        (void)setup_press(&s,&l,dir); CHECK(s.value<30);
    }
    l.row_last[0][0]=14; CHECK(!btn_name_layout_valid(&l));
}

int main(void)
{
    BtnTitleState state; btn_title_reset(&state);
    CHECK(state.page==BTN_TITLE_INTRO && state.selection==1 && state.fade_level==9);
    advance(&state,9);
    CHECK(press(&state,BTN_A)==0 && state.pending);
    advance(&state,21); CHECK(state.page==BTN_TITLE_PROMPT && !state.fade_level);
    CHECK(press(&state,BTN_START)==0 && state.page==BTN_TITLE_MENU && state.selection==1);
    CHECK(press(&state,BTN_UP)==5 && state.selection==0);
    BtnInput held={0}; held.held=BTN_BIT(BTN_UP);
    for (unsigned i=0; i<60; ++i) CHECK(btn_title_step(&state,&held)==-1 && !state.selection);
    CHECK(press(&state,BTN_DOWN)==5 && state.selection==1);
    CHECK(press(&state,BTN_A)==0 && state.page==BTN_TITLE_UNAVAILABLE);
    CHECK(strstr(btn_title_notice(&state),"phase 07"));
    CHECK(press(&state,BTN_B)==-1 && state.page==BTN_TITLE_MENU && state.selection==1);
    CHECK(press(&state,BTN_UP)==5 && state.selection==0);
    CHECK(press(&state,BTN_START)==0 && state.pending && state.target==BTN_TITLE_SETUP);
    advance(&state,21); CHECK(state.page==BTN_TITLE_SETUP && !state.fade_level);
    CHECK(strstr(btn_title_notice(&state),"birthday"));
    state.page=BTN_TITLE_MENU; state.selection=0;
    CHECK(press(&state,BTN_B)==-1 && state.page==BTN_TITLE_PROMPT);
    CHECK(press(&state,BTN_START)==0 && state.page==BTN_TITLE_MENU && state.selection==1);
    BtnInput simultaneous={0}; simultaneous.pressed=BTN_BIT(BTN_A)|BTN_BIT(BTN_UP)|BTN_BIT(BTN_DOWN);
    CHECK(btn_title_step(&state,&simultaneous)==5 && state.selection==0 && state.page==BTN_TITLE_MENU);
    btn_title_reset(&state);
    CHECK(!state.ticks && !state.page_ticks && state.page==BTN_TITLE_INTRO && state.selection==1);
    advance(&state,500); CHECK(state.intro_stage>=3 && state.logo_y==0 && state.copyright_y==0);
    advance(&state,1300); CHECK(state.pending && state.target==BTN_TITLE_ATTRACT);
    advance(&state,21); CHECK(state.page==BTN_TITLE_ATTRACT);
    setup_checks(); dialog_checks(); speech_checks();

    /* Presentation rate must not change menu simulation; pause discards elapsed time. */
    const unsigned rates[]={30,60,144};
    for (unsigned r=0; r<3; ++r) {
        BtnClock clock={0}; unsigned ticks=0; uint64_t previous=0;
        for (unsigned frame=1; frame<=rates[r]*10; ++frame) {
            uint64_t now=(uint64_t)frame*UINT64_C(1000000000)/rates[r];
            ticks+=btn_clock_advance_hz(&clock,now-previous,true,BTN_TITLE_HZ); previous=now;
        }
        CHECK(ticks==300 && !clock.phase && !clock.dropped_ns);
        CHECK(!btn_clock_advance_hz(&clock,UINT64_C(10000000000),false,BTN_TITLE_HZ));
        CHECK(!clock.phase);
        CHECK(btn_clock_advance_hz(&clock,UINT64_C(10000000000),true,BTN_TITLE_HZ)==7);
        CHECK(clock.dropped_ns==UINT64_C(9750000000));
        CHECK(!btn_clock_advance_hz(&clock,1000,true,0) && !clock.phase);
    }
    BtnTitle title={0};
    CHECK(!btn_title_draw(&title,NULL,0));
    btn_title_close(&title); btn_title_close(&title);
    puts("native-title: intro/fade, menu, name bounds/grid/repeat, birthday, setup confirmation/handoff and 30 Hz clock passed");
    return 0;
}
