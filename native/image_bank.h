#ifndef BTN_NATIVE_IMAGE_BANK_H
#define BTN_NATIVE_IMAGE_BANK_H

#include "archive.h"
#include "graphics.h"

typedef struct {
    BtnRect rect;
    uint8_t flags;
    uint32_t length;
    size_t payload;
} BtnImageBlock;
typedef struct { const uint8_t *data; size_t size, consumed; unsigned count; } BtnImageBank;

/* Borrowed input. Parse checks every declared block; decode also validates RLE.
 * Upload is transactional: invalid data leaves VRAM unchanged. Mode 0/1/2
 * follows DecodeContainer0D (normal/STP/keep-wide-palette).
 */
bool btn_image_bank_parse(BtnImageBank *bank, const uint8_t *data, size_t size);
bool btn_image_bank_block(const BtnImageBank *bank, unsigned index, BtnImageBlock *block);
bool btn_image_bank_upload(BtnGraphics *graphics, const BtnImageBank *bank,
    int x, int y, int clut_x, int clut_y, unsigned mode);
/* Only the chosen 0D prefix is allocated, never a whole member or archive.
 * Caller frees *owned_data. Outputs are unchanged on failure.
 */
bool btn_image_bank_read(BtnArchive *archive, unsigned member, uint32_t offset,
    uint8_t **owned_data, BtnImageBank *bank);

#endif
