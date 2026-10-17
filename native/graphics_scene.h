#ifndef BTN_NATIVE_GRAPHICS_SCENE_H
#define BTN_NATIVE_GRAPHICS_SCENE_H
#include "graphics.h"
#include "gte.h"

void btn_graphics_test_texture(BtnGraphics *graphics);
bool btn_graphics_test_scene(BtnGraphics *graphics, BtnDrawList *list, unsigned tick, uint32_t *gte_flags);

#endif
