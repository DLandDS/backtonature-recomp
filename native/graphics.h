#ifndef BTN_NATIVE_GRAPHICS_H
#define BTN_NATIVE_GRAPHICS_H

#include "runtime.h"

enum { BTN_VRAM_W = 1024, BTN_VRAM_H = 512, BTN_OT_BUCKETS = 1024,
       BTN_DRAW_CAPACITY = 4096 };
#define BTN_DRAW_END UINT32_MAX

typedef struct { int x, y, w, h; } BtnRect;
typedef struct { int x, y, u, v; uint8_t r, g, b; } BtnVertex;
typedef struct {
    uint16_t page, clut;
    uint8_t mask_x, mask_y, offset_x, offset_y;
    bool textured, raw, semi, dither, flip_x, flip_y;
} BtnMaterial;
typedef enum { BTN_RECTANGLE, BTN_LINE, BTN_TRIANGLE, BTN_QUAD } BtnPrimitiveType;
typedef struct {
    BtnPrimitiveType type;
    BtnVertex v[4];
    BtnMaterial material;
    int w, h;
} BtnPrimitive;
typedef struct { BtnPrimitive primitive; uint32_t next; } BtnDrawNode;
typedef struct {
    uint32_t heads[BTN_OT_BUCKETS], count;
    BtnDrawNode nodes[BTN_DRAW_CAPACITY];
} BtnDrawList;
typedef struct {
    uint16_t words[BTN_VRAM_W * BTN_VRAM_H];
    BtnRect clip;
    int offset_x, offset_y;
    bool set_mask, check_mask;
} BtnGraphics;

/* Host coordinates and indices, never PS1 tags or truncated host pointers.
 * Allocate these large contexts on the heap. Drawing uses 15-bit VRAM itself.
 * Rectangles are half-open; transfer operations reject out-of-range rectangles.
 */
void btn_graphics_init(BtnGraphics *graphics);
bool btn_vram_upload(BtnGraphics *graphics, BtnRect rect, const uint16_t *words, size_t count);
bool btn_vram_read(const BtnGraphics *graphics, BtnRect rect, uint16_t *words, size_t count);
bool btn_vram_copy(BtnGraphics *graphics, BtnRect source, int x, int y);
uint16_t btn_texture_sample(const BtnGraphics *graphics, const BtnMaterial *material, int u, int v);
uint32_t btn_color_xrgb(uint16_t color);
bool btn_framebuffer(const BtnGraphics *graphics, BtnRect rect, uint32_t *pixels, size_t count);
void btn_draw_list_clear(BtnDrawList *list);
bool btn_draw_list_add(BtnDrawList *list, unsigned bucket, const BtnPrimitive *primitive);
bool btn_draw_list_render(BtnGraphics *graphics, const BtnDrawList *list);
bool btn_draw(BtnGraphics *graphics, const BtnPrimitive *primitive);

#endif
