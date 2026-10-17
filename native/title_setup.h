#ifndef BTN_NATIVE_TITLE_SETUP_H
#define BTN_NATIVE_TITLE_SETUP_H

#include "runtime.h"

typedef enum {
    BTN_SETUP_PLAYER, BTN_SETUP_SEASON, BTN_SETUP_DAY, BTN_SETUP_BIRTHDAY_CONFIRM,
    BTN_SETUP_FARM, BTN_SETUP_DOG, BTN_SETUP_REVIEW, BTN_SETUP_DONE
} BtnSetupStage;

typedef struct {
    uint16_t player[9], farm[9], dog[9];
    uint16_t season, day; /* Both zero-based, as in FarmBirthday. */
    uint16_t seeds[4];
    uint32_t flags;
} BtnNewGame;

typedef struct {
    uint8_t row_last[2][6], column_last[2][14];
    uint16_t symbols[28];
} BtnNameLayout;

typedef struct {
    BtnNewGame game;
    BtnSetupStage stage;
    unsigned cell, shift, length, choice, value, repeat_ticks;
    uint32_t last_held;
    bool dirty, confirming, repeating;
} BtnTitleSetup;

void btn_setup_reset(BtnTitleSetup *setup);
bool btn_name_layout_valid(const BtnNameLayout *layout);
unsigned btn_name_move(const BtnNameLayout *layout, unsigned cell, unsigned shift, uint32_t direction);
uint16_t btn_name_character(const BtnNameLayout *layout, unsigned cell, unsigned shift);
/* Return a retail UI sound ID (0/1/2/5), or -1 when nothing happens.
 * Based on src/ui/name_entry*.c and the title overlay's setup ordering.
 * No file I/O, PS1 pointers, save format or gameplay state is fabricated here. */
int btn_setup_step(BtnTitleSetup *setup, const BtnNameLayout *layout, const BtnInput *input);
const uint16_t *btn_setup_name(const BtnTitleSetup *setup);

#endif
