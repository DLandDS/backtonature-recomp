#include "graphics.h"
#include <SDL3/SDL_error.h>
#include <stdlib.h>
#include <string.h>

static int minimum(int a, int b) { return a < b ? a : b; }
static int maximum(int a, int b) { return a > b ? a : b; }
static int clamp(int x, int low, int high) { return maximum(low, minimum(x, high)); }
static bool valid_rect(BtnRect r)
{
    return r.x >= 0 && r.y >= 0 && r.w > 0 && r.h > 0
        && r.x < BTN_VRAM_W && r.y < BTN_VRAM_H
        && r.w <= BTN_VRAM_W - r.x && r.h <= BTN_VRAM_H - r.y;
}

void btn_graphics_init(BtnGraphics *g)
{
    memset(g, 0, sizeof(*g));
    g->clip = (BtnRect){0, 0, BTN_WIDTH, BTN_HEIGHT};
}

bool btn_vram_upload(BtnGraphics *g, BtnRect r, const uint16_t *words, size_t count)
{
    if (!g || !words || !valid_rect(r) || count < (size_t)r.w * (size_t)r.h)
        return SDL_SetError("Invalid VRAM upload rectangle/capacity");
    for (int y = 0; y < r.h; ++y)
        memcpy(g->words + (r.y + y) * BTN_VRAM_W + r.x, words + y * r.w, (size_t)r.w * 2);
    return true;
}

bool btn_vram_read(const BtnGraphics *g, BtnRect r, uint16_t *words, size_t count)
{
    if (!g || !words || !valid_rect(r) || count < (size_t)r.w * (size_t)r.h)
        return SDL_SetError("Invalid VRAM read rectangle/capacity");
    for (int y = 0; y < r.h; ++y)
        memcpy(words + y * r.w, g->words + (r.y + y) * BTN_VRAM_W + r.x, (size_t)r.w * 2);
    return true;
}

bool btn_vram_copy(BtnGraphics *g, BtnRect source, int x, int y)
{
    BtnRect dest = {x, y, source.w, source.h};
    if (!g || !valid_rect(source) || !valid_rect(dest)) return SDL_SetError("Invalid VRAM copy rectangle");
    size_t count = (size_t)source.w * (size_t)source.h;
    uint16_t *snapshot = malloc(count * sizeof(*snapshot));
    if (!snapshot) return SDL_SetError("Cannot allocate VRAM copy snapshot");
    bool ok = btn_vram_read(g, source, snapshot, count) && btn_vram_upload(g, dest, snapshot, count);
    free(snapshot);
    return ok;
}

uint16_t btn_texture_sample(const BtnGraphics *g, const BtnMaterial *m, int u, int v)
{
    unsigned depth = (m->page >> 7) & 3u;
    unsigned tu = ((unsigned)u & ~(unsigned)(m->mask_x * 8u))
        | (unsigned)((m->offset_x & m->mask_x) * 8u);
    unsigned tv = ((unsigned)v & ~(unsigned)(m->mask_y * 8u))
        | (unsigned)((m->offset_y & m->mask_y) * 8u);
    tu &= 255u; tv &= 255u;
    unsigned shift = depth == 0 ? 2u : depth == 1 ? 1u : 0u;
    unsigned x = (((m->page & 15u) * 64u) + (tu >> shift)) & 1023u;
    unsigned y = (((m->page & 16u) * 16u) + tv) & 511u;
    uint16_t word = g->words[y * BTN_VRAM_W + x];
    if (depth == 2) return word;
    if (depth == 3) return 0; /* Public drawing API rejects this reserved mode. */
    unsigned index = depth == 0 ? ((unsigned)word >> ((tu & 3u) * 4u)) & 15u
        : ((unsigned)word >> ((tu & 1u) * 8u)) & 255u;
    x = (((m->clut & 63u) * 16u) + index) & 1023u;
    y = (m->clut >> 6) & 511u;
    return g->words[y * BTN_VRAM_W + x];
}

uint32_t btn_color_xrgb(uint16_t color)
{
    unsigned r = color & 31u, g = (color >> 5) & 31u, b = (color >> 10) & 31u;
    return ((r * 255u / 31u) << 16) | ((g * 255u / 31u) << 8) | (b * 255u / 31u);
}

bool btn_framebuffer(const BtnGraphics *g, BtnRect r, uint32_t *pixels, size_t count)
{
    if (!g || !pixels || !valid_rect(r) || count < (size_t)r.w * (size_t)r.h)
        return SDL_SetError("Invalid framebuffer rectangle/capacity");
    for (int y = 0; y < r.h; ++y)
        for (int x = 0; x < r.w; ++x)
            pixels[y * r.w + x] = btn_color_xrgb(g->words[(r.y + y) * BTN_VRAM_W + r.x + x]);
    return true;
}

static void pixel(BtnGraphics *g, int x, int y, const BtnMaterial *m,
    int u, int v, int r, int green, int b, bool polygon)
{
    static const int dither[4][4] = {{-4,0,-3,1},{2,-2,3,-1},{-3,1,-4,0},{3,-1,2,-2}};
    if (x < g->clip.x || y < g->clip.y || x >= g->clip.x + g->clip.w || y >= g->clip.y + g->clip.h)
        return;
    uint16_t *dest = &g->words[y * BTN_VRAM_W + x];
    if (g->check_mask && (*dest & 0x8000u)) return;
    uint16_t texel = 0;
    if (m->textured) {
        texel = btn_texture_sample(g, m, u, v);
        if (!texel) return;
        if (m->raw) {
            r = (texel & 31) * 8; green = ((texel >> 5) & 31) * 8; b = ((texel >> 10) & 31) * 8;
        } else {
            r = ((texel & 31) * r) / 16;
            green = (((texel >> 5) & 31) * green) / 16;
            b = (((texel >> 10) & 31) * b) / 16;
        }
    }
    int d = polygon && m->dither && !(m->textured && m->raw) ? dither[y & 3][x & 3] : 0;
    int channels[3] = {clamp(r + d, 0, 255) / 8, clamp(green + d, 0, 255) / 8, clamp(b + d, 0, 255) / 8};
    uint16_t result = (uint16_t)((g->set_mask ? 0x8000u : 0u) | (texel & 0x8000u));
    for (unsigned c = 0; c < 3; ++c) {
        int front = channels[c];
        if (m->semi && (!m->textured || (texel & 0x8000u))) {
            int back = (*dest >> (c * 5)) & 31;
            switch ((m->page >> 5) & 3u) {
            case 0: front = back / 2 + front / 2; break;
            case 1: front = back + front; break;
            case 2: front = back - front; break;
            case 3: front = back + front / 4; break;
            }
        }
        result |= (uint16_t)((unsigned)clamp(front, 0, 31) << (c * 5));
    }
    *dest = result;
}

static int64_t edge(BtnVertex a, BtnVertex b, int x, int y)
{
    return (int64_t)(b.x - a.x) * (y - a.y) - (int64_t)(b.y - a.y) * (x - a.x);
}
static bool top_left(BtnVertex a, BtnVertex b)
{
    return b.y < a.y || (b.y == a.y && b.x > a.x);
}
static void triangle(BtnGraphics *g, BtnVertex a, BtnVertex b, BtnVertex c, const BtnMaterial *m)
{
    int64_t area = edge(a, b, c.x, c.y);
    if (!area) return;
    if (area < 0) { BtnVertex temp = b; b = c; c = temp; area = -area; }
    int x0 = maximum(g->clip.x, minimum(a.x, minimum(b.x, c.x)));
    int y0 = maximum(g->clip.y, minimum(a.y, minimum(b.y, c.y)));
    int x1 = minimum(g->clip.x + g->clip.w - 1, maximum(a.x, maximum(b.x, c.x)));
    int y1 = minimum(g->clip.y + g->clip.h - 1, maximum(a.y, maximum(b.y, c.y)));
    for (int y = y0; y <= y1; ++y) for (int x = x0; x <= x1; ++x) {
        int64_t wa = edge(b, c, x, y), wb = edge(c, a, x, y), wc = edge(a, b, x, y);
        if (wa < 0 || wb < 0 || wc < 0 || (!wa && !top_left(b,c))
            || (!wb && !top_left(c,a)) || (!wc && !top_left(a,b))) continue;
#define INTERPOLATE(field) ((int)((wa * a.field + wb * b.field + wc * c.field) / area))
        pixel(g, x, y, m, INTERPOLATE(u), INTERPOLATE(v), INTERPOLATE(r), INTERPOLATE(g), INTERPOLATE(b), true);
#undef INTERPOLATE
    }
}

bool btn_draw(BtnGraphics *g, const BtnPrimitive *p)
{
    if (!g || !p || !valid_rect(g->clip) || g->offset_x < -2048 || g->offset_x > 2047
        || g->offset_y < -2048 || g->offset_y > 2047 || p->type < BTN_RECTANGLE || p->type > BTN_QUAD
        || (p->material.textured && ((p->material.page >> 7) & 3u) == 3))
        return SDL_SetError("Unsupported primitive, clip, offset, or texture depth");
    BtnVertex v[4];
    memcpy(v, p->v, sizeof(v));
    for (unsigned i = 0; i < 4; ++i) {
        if (v[i].x < -32768 || v[i].x > 32767 || v[i].y < -32768 || v[i].y > 32767
            || v[i].u < -32768 || v[i].u > 32767 || v[i].v < -32768 || v[i].v > 32767)
            return SDL_SetError("Primitive coordinate exceeds signed 16-bit range");
        v[i].x += g->offset_x; v[i].y += g->offset_y;
    }
    if (p->type == BTN_RECTANGLE) {
        if (p->w < 0 || p->w > 1024 || p->h < 0 || p->h > 512)
            return SDL_SetError("Invalid rectangle dimensions");
        int x0 = maximum(v[0].x, g->clip.x), y0 = maximum(v[0].y, g->clip.y);
        int x1 = minimum(v[0].x + p->w, g->clip.x + g->clip.w);
        int y1 = minimum(v[0].y + p->h, g->clip.y + g->clip.h);
        for (int y = y0; y < y1; ++y) for (int x = x0; x < x1; ++x) {
            int dx = x - v[0].x, dy = y - v[0].y;
            pixel(g,x,y,&p->material,v[0].u + (p->material.flip_x ? -dx : dx),
                v[0].v + (p->material.flip_y ? -dy : dy),v[0].r,v[0].g,v[0].b,false);
        }
    } else if (p->type == BTN_LINE) {
        if (p->material.textured) return SDL_SetError("Textured lines are unsupported");
        int dx = abs(v[1].x - v[0].x), dy = abs(v[1].y - v[0].y);
        int steps = maximum(dx, dy);
        for (int i = 0; i <= steps; ++i) {
            int n = steps ? steps : 1;
#define LERP(field) ((int)(v[0].field + (int64_t)(v[1].field - v[0].field) * i / n))
            pixel(g,LERP(x),LERP(y),&p->material,0,0,LERP(r),LERP(g),LERP(b),true);
#undef LERP
        }
    } else {
        triangle(g,v[0],v[1],v[2],&p->material);
        if (p->type == BTN_QUAD) triangle(g,v[1],v[3],v[2],&p->material);
    }
    return true;
}

void btn_draw_list_clear(BtnDrawList *list)
{
    list->count = 0;
    for (unsigned i = 0; i < BTN_OT_BUCKETS; ++i) list->heads[i] = BTN_DRAW_END;
}
bool btn_draw_list_add(BtnDrawList *list, unsigned bucket, const BtnPrimitive *primitive)
{
    if (!list || !primitive || bucket >= BTN_OT_BUCKETS || list->count >= BTN_DRAW_CAPACITY)
        return SDL_SetError("Ordering table bucket/capacity exceeded");
    uint32_t index = list->count++;
    list->nodes[index] = (BtnDrawNode){*primitive, list->heads[bucket]};
    list->heads[bucket] = index;
    return true;
}
bool btn_draw_list_render(BtnGraphics *g, const BtnDrawList *list)
{
    if (!g || !list || list->count > BTN_DRAW_CAPACITY) return SDL_SetError("Invalid ordering table");
    unsigned visited = 0;
    for (unsigned bucket = BTN_OT_BUCKETS; bucket-- > 0;) {
        uint32_t index = list->heads[bucket];
        while (index != BTN_DRAW_END) {
            if (index >= list->count || ++visited > list->count)
                return SDL_SetError("Invalid/cyclic ordering table link");
            if (!btn_draw(g, &list->nodes[index].primitive)) return false;
            index = list->nodes[index].next;
        }
    }
    return true;
}
