/* Included by src/ui.c; shares its private declarations. */
/* A6 candidate: retain separate outer-index and outer-offset induction
 * variables through the indexed affine access. */
s32 shop_has_item(s16 *bets, s16 id)
{
    u32 i;
    u32 j;

    i = 0;
    bets++;
    for (; i < 3; i++) {
        for (j = 0; j < 6; j++) {
            if (*(s16 *)((u8 *)bets + i * 0x54 + j * 0xE) == id) {
                return 1;
            }
        }
    }
    return 0;
}
/* Round 33e bounded: 84-byte blocks with 14-byte rows reach 180 differences, 192 versus 200 words;
 * hoisted table bases and special-state scheduling remain different. Candidate round33e-2206Cb.txt.
 */
/* Ported from hmbtn-decomp (CC0), shop_restock. */
typedef struct HorseView {
    u8 pad[0x92B2];
    s16 affection;
    u8 pad2[0x9348 - 0x92B4];
    s16 training;
} HorseView;
extern u16 shopRestockValues[80];
extern u16 shopRestockRanges[24];
void shop_restock(void)
{
    FarmRaceBets *bets = (FarmRaceBets *)(gameState + 0xE514);
    s16 mode;
    u32 group;
    u32 row;
    s16 id;
    u32 highest;

    mode = bets->race;
    memset(bets, 0, sizeof(FarmRaceBets));
    for (group = 0; group < 3; group++) {
        for (row = 0; row < 6; row++) {
            bets->rows[group][row].runner = -1;
        }
    }
    for (group = 0; group < 3; group++) {
        for (row = 0; row < 6; row++) {
            do {
                id = rand_range(40);
            } while (shop_has_item((s16 *)bets, id) != 0);
            bets->rows[group][row].runner = id;
            bets->rows[group][row].chance = shopRestockValues[id * 2];
            bets->rows[group][row].rivalChance = shopRestockValues[id * 2 + 1];
            bets->rows[group][row].training =
                shopRestockRanges[row * 4] +
                rand_range((u16)(shopRestockRanges[row * 4 + 1] - shopRestockRanges[row * 4]));
            bets->rows[group][row].affection =
                shopRestockRanges[row * 4 + 2] +
                rand_range((u16)(shopRestockRanges[row * 4 + 3] - shopRestockRanges[row * 4 + 2]));
        }
    }
    if (mode == 3) {
        bets->rows[2][2].runner = -1;
        bets->rows[2][2].chance = 0;
        bets->rows[2][2].rivalChance = 0;
        bets->rows[2][2].training = ((HorseView *)gameState)->training;
        bets->rows[2][2].affection = ((HorseView *)gameState)->affection;
    }
    for (group = 0; group < 3; group++) {
        highest = 0;
        for (row = 0; row < 6; row++) {
            if (highest < bets->rows[group][row].training) {
                highest = bets->rows[group][row].training;
            }
        }
        for (row = 0; row < 6; row++) {
            bets->rows[group][row].odds =
                rand_range(4) + ((highest - bets->rows[group][row].training) *
                                     (highest - bets->rows[group][row].training) / 1000 +
                                 1);
        }
    }
}

typedef void (*UiCallback)(s32, s32, s32, s32, s32);
void bg_tile_screen(s32, UiCallback);
void bg_tile_draw_sprite(s32, s32, s32, s32, s32);
void bg_tile_draw_cursor(s32, s32, s32, s32, s32);
void anim_bind_file(void *, void *);
void anim_draw_sprt_cursor(s32, s32, s32, s32, s32, s32);
void anim_draw_sprt(s32, s32 *, s32 *, s32, s32, u8, s16, s16);
void anim_select_range(void *, u8);
void anim_draw_sprt_advance(void *, void *, s16, s16);
void prim_add_tile(u32 *, s16, s16, s16, s16, s32, s16);
void name_sound_draw(void);
void name_grid_step(void);
/* CONFIRMED from system ASM: a0 is passed through; the four scalar arguments
 * are each read as signed halves. The first scalar is decremented internally. */
void window_frame_draw(s32 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4);
