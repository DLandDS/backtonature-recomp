#include "image_bank.h"
#include <SDL3/SDL_error.h>
#include <stdlib.h>
#include <string.h>

static unsigned le16(const uint8_t *p) { return (unsigned)p[0] | ((unsigned)p[1] << 8); }
static bool block_at(const uint8_t *data, size_t size, size_t cursor, BtnImageBlock *block, size_t *next)
{
    if (cursor > size || size - cursor < 12) return SDL_SetError("Truncated 0D block header");
    uint32_t word = btn_read_le32(data + cursor), length = word & 0xFFFFFFu;
    size_t aligned = ((size_t)length + 3u) & ~(size_t)3;
    if (aligned > size - cursor - 12) return SDL_SetError("0D block payload exceeds resource span");
    *block = (BtnImageBlock){{(int)le16(data+cursor+4),(int)le16(data+cursor+6),
        (int)le16(data+cursor+8),(int)le16(data+cursor+10)}, (uint8_t)(word >> 24), length, cursor+12};
    *next = cursor + 12 + aligned;
    return true;
}

bool btn_image_bank_parse(BtnImageBank *bank, const uint8_t *data, size_t size)
{
    if (!bank || !data || size < 4 || data[0] != 13) return SDL_SetError("Expected container 0D header");
    unsigned count = le16(data+2);
    size_t cursor = 4;
    BtnImageBlock block;
    for (unsigned i = 0; i < count; ++i) if (!block_at(data,size,cursor,&block,&cursor)) return false;
    *bank = (BtnImageBank){data,size,cursor,count};
    return true;
}
bool btn_image_bank_block(const BtnImageBank *bank, unsigned index, BtnImageBlock *block)
{
    if (!bank || !block || index >= bank->count) return SDL_SetError("0D block index out of range");
    size_t cursor = 4;
    for (unsigned i = 0; i <= index; ++i)
        if (!block_at(bank->data,bank->size,cursor,block,&cursor)) return false;
    return true;
}

static bool decode(const BtnImageBank *bank, const BtnImageBlock *block, uint8_t *out, size_t bytes)
{
    const uint8_t *p = bank->data + block->payload;
    size_t read = 0;
    unsigned run = 0;
    bool repeat = false;
    uint8_t value = 0;
    for (size_t i = 0; i < bytes; ++i) {
        if ((block->flags & 12u) == 4u) {
            if (read >= block->length) return SDL_SetError("Truncated raw 0D pixels");
            value = p[read++];
        } else {
            if (!run) {
                if (read >= block->length) return SDL_SetError("Truncated 0D RLE control");
                unsigned control = p[read++];
                repeat = control < 128u;
                run = repeat ? (control ? control : 1u) : 256u-control;
                if (repeat) {
                    if (read >= block->length) return SDL_SetError("Truncated 0D RLE repeat");
                    value = p[read++];
                }
            }
            if (!repeat) {
                if (read >= block->length) return SDL_SetError("Truncated 0D RLE literal");
                value = p[read++];
            }
            --run;
        }
        size_t dest = i;
        if (block->flags & 8u) {
            /* Retail walks word columns, low byte down the column then high. */
            size_t height = (size_t)block->rect.h;
            size_t column = i / (height * 2), byte = (i / height) & 1u, row = i % height;
            dest = (row * (size_t)block->rect.w + column) * 2 + byte;
        }
        out[dest] = value;
    }
    return true;
}

bool btn_image_bank_upload(BtnGraphics *g, const BtnImageBank *bank,
    int x, int y, int clut_x, int clut_y, unsigned mode)
{
    if (!g || !bank || mode > 2 || x < -1024 || x > 1024 || y < -512 || y > 512
        || clut_x < -1024 || clut_x > 1024 || clut_y < -512 || clut_y > 512)
        return SDL_SetError("Invalid 0D upload mode/offsets");
    BtnGraphics *staging = malloc(sizeof(*staging));
    if (!staging) return SDL_SetError("Cannot allocate transactional VRAM upload");
    memcpy(staging,g,sizeof(*g));
    bool ok = false;
    size_t cursor = 4;
    for (unsigned i = 0; i < bank->count; ++i) {
        BtnImageBlock block;
        if (!block_at(bank->data,bank->size,cursor,&block,&cursor)) goto done;
        BtnRect r = block.rect;
        if ((block.flags & 3u) != 2u) { r.x += x; r.y += y; }
        else if (r.w < 256 || mode != 2) { r.x += clut_x; r.y += clut_y; }
        if (r.x < 0 || r.y < 0 || r.w <= 0 || r.h <= 0 || r.x >= BTN_VRAM_W || r.y >= BTN_VRAM_H
            || r.w > BTN_VRAM_W-r.x || r.h > BTN_VRAM_H-r.y) {
            SDL_SetError("0D block %u exceeds VRAM rectangle",i); goto done;
        }
        size_t count = (size_t)r.w * (size_t)r.h;
        uint8_t *decoded = malloc(count * 2);
        uint16_t *words = malloc(count * sizeof(*words));
        if (!decoded || !words) {
            free(decoded); free(words); SDL_SetError("Cannot allocate decoded 0D pixels"); goto done;
        }
        bool decoded_ok = decode(bank,&block,decoded,count*2);
        if (decoded_ok) {
            for (size_t n = 0; n < count; ++n) {
                words[n] = (uint16_t)le16(decoded+n*2);
                /* Retail skips word zero of each row, and the whole column path. */
                if (mode == 1 && (block.flags & 2u) && !(block.flags & 8u) && n % (size_t)r.w)
                    words[n] |= 0x8000u;
            }
            decoded_ok = btn_vram_upload(staging,r,words,count);
        }
        free(decoded); free(words);
        if (!decoded_ok) goto done;
    }
    memcpy(g->words,staging->words,sizeof(g->words));
    ok = true;
done:
    free(staging);
    return ok;
}

bool btn_image_bank_read(BtnArchive *archive, unsigned member, uint32_t offset,
    uint8_t **owned_data, BtnImageBank *bank)
{
    uint8_t header[12];
    uint32_t absolute;
    if (!archive || !owned_data || !bank || !btn_archive_range(&archive->index,member,offset,4,&absolute)
        || !btn_archive_read(archive,member,offset,4,header,sizeof(header))) return false;
    if (header[0] != 13) return SDL_SetError("Selected member/offset is not container 0D");
    unsigned count = le16(header+2);
    uint32_t cursor = offset+4;
    for (unsigned i = 0; i < count; ++i) {
        if (!btn_archive_read(archive,member,cursor,12,header,sizeof(header))) return false;
        uint32_t length = btn_read_le32(header) & 0xFFFFFFu;
        uint32_t step = 12u + ((length+3u) & ~3u);
        if (!btn_archive_range(&archive->index,member,cursor,step,&absolute)) return false;
        cursor += step;
    }
    size_t size = cursor-offset;
    /* A 1 MiB VRAM needs no arbitrary archive-sized allocation. Encoded streams
     * can be larger than decoded pixels; 16 MiB is this viewer's explicit cap. */
    if (size > 16u*1024u*1024u) return SDL_SetError("0D container exceeds viewer's 16 MiB limit");
    uint8_t *data = malloc(size);
    if (!data) return SDL_SetError("Cannot allocate selected 0D container");
    BtnImageBank parsed;
    if (!btn_archive_read(archive,member,offset,(uint32_t)size,data,size)
        || !btn_image_bank_parse(&parsed,data,size)) { free(data); return false; }
    *owned_data = data; *bank = parsed;
    return true;
}
