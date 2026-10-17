#include "archive.h"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_stdinc.h>

bool btn_archive_parse(BtnArchiveIndex *index, const uint8_t *hdt,
    size_t hdt_size, uint64_t bin_size)
{
    BtnArchiveIndex parsed = {0};
    if (!index || !hdt || hdt_size < BTN_ARCHIVE_HDT_BYTES)
        return SDL_SetError("HDT is shorter than the 33 required offsets");
    if (bin_size > UINT32_MAX)
        return SDL_SetError("BIN exceeds the 32-bit archive address space");
    for (unsigned i = 0; i < BTN_ARCHIVE_OFFSETS; ++i) {
        uint32_t offset = btn_read_le32(hdt + i * 4);
        if (i && offset < parsed.offsets[i - 1])
            return SDL_SetError("HDT offsets decrease at index %u", i);
        if (offset > bin_size)
            return SDL_SetError("HDT offset %u exceeds BIN size", i);
        parsed.offsets[i] = offset;
    }
    parsed.bin_size = (uint32_t)bin_size;
    parsed.hdt_extra_bytes = hdt_size - BTN_ARCHIVE_HDT_BYTES;
    *index = parsed;
    return true;
}

bool btn_archive_range(const BtnArchiveIndex *index, unsigned member,
    uint32_t offset, uint32_t length, uint32_t *absolute)
{
    if (!index || !absolute || member >= BTN_ARCHIVE_MEMBERS)
        return SDL_SetError("Archive member index must be 0..31");
    uint32_t start = index->offsets[member];
    uint32_t end = index->offsets[member + 1];
    if (end < start || end > index->bin_size || !btn_span_valid(end - start, offset, length))
        return SDL_SetError("Requested range exceeds member %u span", member);
    *absolute = start + offset;
    return true;
}

bool btn_archive_open(BtnArchive *archive, const char *hdt_path, const char *bin_path)
{
    BtnArchive opened = {0};
    uint8_t hdt[BTN_ARCHIVE_HDT_BYTES];
    char error[256];
    if (!archive || archive->bin)
        return SDL_SetError("Archive must be closed before opening");
    SDL_IOStream *table = SDL_IOFromFile(hdt_path, "rb");
    if (!table) return false;
    Sint64 hdt_size = SDL_GetIOSize(table);
    if (hdt_size < BTN_ARCHIVE_HDT_BYTES || SDL_ReadIO(table, hdt, sizeof(hdt)) != sizeof(hdt)) {
        SDL_CloseIO(table);
        return SDL_SetError("HDT is shorter than the 33 required offsets or could not be read");
    }
    if (!SDL_CloseIO(table)) return false;
    opened.bin = SDL_IOFromFile(bin_path, "rb");
    if (!opened.bin) return false;
    Sint64 bin_size = SDL_GetIOSize(opened.bin);
    if (bin_size < 0 || !btn_archive_parse(&opened.index, hdt, sizeof(hdt), (uint64_t)bin_size)) {
        SDL_strlcpy(error, SDL_GetError(), sizeof(error));
        SDL_CloseIO(opened.bin);
        return SDL_SetError("Cannot open archive: %s", error);
    }
    opened.index.hdt_extra_bytes = (uint64_t)hdt_size - sizeof(hdt);
    *archive = opened;
    return true;
}

void btn_archive_close(BtnArchive *archive)
{
    if (!archive) return;
    if (archive->bin) SDL_CloseIO(archive->bin);
    *archive = (BtnArchive){0};
}

bool btn_archive_read(BtnArchive *archive, unsigned member, uint32_t offset,
    uint32_t length, void *destination, size_t capacity)
{
    uint32_t absolute;
    if (!archive || !archive->bin) return SDL_SetError("Archive is not open");
    if (!btn_archive_range(&archive->index, member, offset, length, &absolute)) return false;
    if (capacity < length || (length && !destination))
        return SDL_SetError("Destination buffer is smaller than the requested range");
    if (!length) return true;
    if (SDL_SeekIO(archive->bin, absolute, SDL_IO_SEEK_SET) != (Sint64)absolute) return false;
    size_t done = 0;
    while (done < length) {
        size_t count = SDL_ReadIO(archive->bin, (uint8_t *)destination + done, length - done);
        if (!count) return SDL_SetError("BIN read failed before requested range was complete");
        done += count;
    }
    return true;
}

bool btn_archive_read_members(BtnArchive *archive, unsigned first, unsigned last,
    void *destination, size_t capacity)
{
    if (!archive || !archive->bin || first >= last || last > BTN_ARCHIVE_MEMBERS)
        return SDL_SetError("Invalid consecutive archive member range");
    uint32_t start = archive->index.offsets[first], end = archive->index.offsets[last];
    if (end < start || end > archive->index.bin_size || capacity < end - start
        || (end != start && !destination))
        return SDL_SetError("Consecutive member range exceeds destination or BIN");
    size_t done = 0;
    for (unsigned member = first; member < last; ++member) {
        uint32_t length = archive->index.offsets[member + 1] - archive->index.offsets[member];
        uint8_t *target = destination ? (uint8_t *)destination + done : NULL;
        if (!btn_archive_read(archive, member, 0, length, target, capacity - done)) return false;
        done += length;
    }
    return true;
}
