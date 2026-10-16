#ifndef BTNX_GAME_SHARED_TYPES_12_H
#define BTNX_GAME_SHARED_TYPES_12_H
#include "common.h"
#include "psyq_gte.h"
#include "psyq_libgpu.h"

enum Utensil {
    UTENSIL_KNIFE = 0,
    UTENSIL_FRYING_PAN = 1,
    UTENSIL_POT = 2,
    UTENSIL_MIXER = 3,
    UTENSIL_WHISK = 4,
    UTENSIL_ROLLING_PIN = 5,
    UTENSIL_OVEN = 6,
    UTENSIL_SEASONING_SET = 7,
};

enum UtensilFlag {
    UTENSIL_FLAG_KNIFE = (1 << UTENSIL_KNIFE),
    UTENSIL_FLAG_FRYING_PAN = (1 << UTENSIL_FRYING_PAN),
    UTENSIL_FLAG_POT = (1 << UTENSIL_POT),
    UTENSIL_FLAG_MIXER = (1 << UTENSIL_MIXER),
    UTENSIL_FLAG_WHISK = (1 << UTENSIL_WHISK),
    UTENSIL_FLAG_ROLLING_PIN = (1 << UTENSIL_ROLLING_PIN),
    UTENSIL_FLAG_OVEN = (1 << UTENSIL_OVEN),
    UTENSIL_FLAG_SEASONING_SET = (1 << UTENSIL_SEASONING_SET),
};

typedef struct ShopMenuHeader {
    u16 *w0;
    u16 *w4;
    s16 h8;
    s16 ha;
} ShopMenuHeader;

#endif
