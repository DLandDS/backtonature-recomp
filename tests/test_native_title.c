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

int main(void)
{
    BtnTitleState state; btn_title_reset(&state);
    CHECK(state.page==BTN_TITLE_PROMPT && state.selection==1);
    CHECK(press(&state,BTN_A)==-1 && state.page==BTN_TITLE_PROMPT);
    CHECK(press(&state,BTN_START)==0 && state.page==BTN_TITLE_MENU && state.selection==1);
    CHECK(press(&state,BTN_UP)==5 && state.selection==0);
    BtnInput held={0}; held.held=BTN_BIT(BTN_UP);
    for (unsigned i=0; i<60; ++i) CHECK(btn_title_step(&state,&held)==-1 && !state.selection);
    CHECK(press(&state,BTN_DOWN)==5 && state.selection==1);
    CHECK(press(&state,BTN_A)==0 && state.page==BTN_TITLE_UNAVAILABLE);
    CHECK(strstr(btn_title_notice(&state),"phase 07"));
    CHECK(press(&state,BTN_B)==-1 && state.page==BTN_TITLE_MENU && state.selection==1);
    CHECK(press(&state,BTN_UP)==5 && state.selection==0);
    CHECK(press(&state,BTN_START)==0 && state.page==BTN_TITLE_UNAVAILABLE);
    CHECK(strstr(btn_title_notice(&state),"name entry"));
    CHECK(press(&state,BTN_START)==-1 && state.page==BTN_TITLE_MENU && state.selection==0);
    CHECK(press(&state,BTN_B)==-1 && state.page==BTN_TITLE_PROMPT);
    CHECK(press(&state,BTN_START)==0 && state.page==BTN_TITLE_MENU && state.selection==1);
    BtnInput simultaneous={0}; simultaneous.pressed=BTN_BIT(BTN_A)|BTN_BIT(BTN_UP)|BTN_BIT(BTN_DOWN);
    CHECK(btn_title_step(&state,&simultaneous)==5 && state.selection==0 && state.page==BTN_TITLE_MENU);
    btn_title_reset(&state);
    CHECK(!state.ticks && !state.page_ticks && state.page==BTN_TITLE_PROMPT && state.selection==1);

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
    puts("native-title: menu edges, precedence, unavailable destinations, reset and nominal 30 Hz clock passed");
    return 0;
}
