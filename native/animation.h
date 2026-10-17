#ifndef BTN_NATIVE_ANIMATION_H
#define BTN_NATIVE_ANIMATION_H
#include "archive.h"
#include "graphics.h"

typedef struct {
    const uint8_t *data;
    size_t size, consumed;
    uint32_t section[5], frames;
    unsigned images, lists, parts, ranges;
} BtnAnimation;
typedef struct { unsigned range, frame, ticks; bool ended, jumped, paused; } BtnAnimationCursor;

/* 0E tables are borrowed bytes, validated without patching serialized offsets.
 * The 0D image bank is selected/loaded separately; no guessed bank association.
 */
bool btn_animation_parse(BtnAnimation *animation, const uint8_t *data, size_t size);
bool btn_animation_read(BtnArchive *archive, unsigned member, uint32_t offset,
    uint8_t **owned_data, BtnAnimation *animation);
bool btn_animation_tick(const BtnAnimation *animation, BtnAnimationCursor *cursor);
bool btn_animation_draw(const BtnAnimation *animation, const BtnAnimationCursor *cursor,
    BtnDrawList *list, unsigned bucket, int x, int y);
/* gf.bin prefix block 9, English glyphs: 8x12, page E, CLUT (896,254).
 * Supports glyphs 00..FF, space, newline and END. Other controls fail visibly.
 */
bool btn_text_draw(BtnDrawList *list, unsigned bucket, const uint16_t *words,
    size_t count, int x, int y, size_t *consumed);

#endif
