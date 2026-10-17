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
    setup_checks();

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
