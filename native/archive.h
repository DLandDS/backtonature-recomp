#ifndef BTN_NATIVE_ARCHIVE_H
#define BTN_NATIVE_ARCHIVE_H

#include <SDL3/SDL_iostream.h>
#include "runtime.h"

/* afile_init consumes 33 offsets. Extra bytes and resource payload formats
 * are classified separately from container span validation.
 */
enum { BTN_ARCHIVE_MEMBERS = 32, BTN_ARCHIVE_OFFSETS = 33,
       BTN_ARCHIVE_HDT_BYTES = BTN_ARCHIVE_OFFSETS * 4 };

typedef struct {
    uint32_t offsets[BTN_ARCHIVE_OFFSETS];
    uint32_t bin_size;
    uint64_t hdt_extra_bytes;
} BtnArchiveIndex;

typedef struct {
    BtnArchiveIndex index;
    SDL_IOStream *bin;
} BtnArchive;

/* All failures return false and set SDL_GetError(). Output index/range values
 * remain unchanged on failure. Extra HDT bytes and BIN prefix/tail are accepted
 * but unclassified; member sizes describe allocated spans, not payload sizes.
 */
bool btn_archive_parse(BtnArchiveIndex *index, const uint8_t *hdt,
    size_t hdt_size, uint64_t bin_size);
bool btn_archive_range(const BtnArchiveIndex *index, unsigned member,
    uint32_t offset, uint32_t length, uint32_t *absolute);

/* Zero-initialize before opening; close before reopening. The archive owns its
 * read-only BIN stream. No SDL_Init/video device is required. One thread at a
 * time may use a given archive; the caller owns every destination buffer.
 */
bool btn_archive_open(BtnArchive *archive, const char *hdt_path, const char *bin_path);
void btn_archive_close(BtnArchive *archive);

/* Synchronous byte read: destination[0] is the requested byte (no sector skip),
 * and success means exactly length bytes have been copied. Zero-length reads
 * at a member's end are valid, including empty members. Capacity must cover
 * length. On I/O failure the destination may be partially written; ignore it.
 * Polling/mode/copy adapters are provided separately in archive_load.h.
 */
bool btn_archive_read(BtnArchive *archive, unsigned member, uint32_t offset,
    uint32_t length, void *destination, size_t capacity);
/* Load consecutive members [first, last), directly into host memory. */
bool btn_archive_read_members(BtnArchive *archive, unsigned first, unsigned last,
    void *destination, size_t capacity);

#endif
