#include "archive_load.h"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_stdinc.h>

static bool queue(BtnArchiveLoad *load, BtnArchive *archive, unsigned first, unsigned last,
    uint32_t offset, uint32_t length, void *destination, size_t capacity, bool consecutive)
{
    if (!load || !archive || !archive->bin || first >= last || last > BTN_ARCHIVE_MEMBERS)
        return SDL_SetError("Invalid archive loading request");
    uint32_t start = archive->index.offsets[first], end = archive->index.offsets[last];
    if (end < start || end > archive->index.bin_size || !btn_span_valid(end - start, offset, length)
        || capacity < length || (length && !destination))
        return SDL_SetError("Loading request exceeds member span or destination capacity");
    *load = (BtnArchiveLoad){0};
    load->archive = archive;
    load->first = first;
    load->last = last;
    load->offset = offset;
    load->length = length;
    load->destination = destination;
    load->capacity = capacity;
    load->consecutive = consecutive;
    load->state = BTN_LOAD_PENDING;
    return true;
}

static int poll(BtnArchiveLoad *load)
{
    bool ok = load->consecutive
        ? btn_archive_read_members(load->archive, load->first, load->last,
            load->destination, load->capacity)
        : btn_archive_read(load->archive, load->first, load->offset, load->length,
            load->destination, load->capacity);
    if (!ok) {
        SDL_strlcpy(load->error, SDL_GetError(), sizeof(load->error));
        load->state = BTN_LOAD_FAILED;
        return -1;
    }
    load->state = BTN_LOAD_READY;
    return 1;
}

int btn_archive_read_async(BtnArchiveLoad *load, BtnArchive *archive, unsigned member,
    uint32_t offset, uint32_t length, void *destination, size_t capacity, int mode)
{
    if (!load || member >= BTN_ARCHIVE_MEMBERS || (mode != 0 && mode != 1)) {
        SDL_SetError("Invalid asynchronous archive arguments");
        return -1;
    }
    if (mode == 1 || load->state == BTN_LOAD_IDLE)
        return queue(load, archive, member, member + 1, offset, length,
            destination, capacity, false) ? 0 : -1;
    if (load->state == BTN_LOAD_FAILED) {
        SDL_SetError("Archive loading failed: %s", load->error);
        return -1;
    }
    if (load->state != BTN_LOAD_PENDING || load->consecutive || load->archive != archive
        || load->first != member || load->offset != offset || load->length != length
        || load->destination != destination || load->capacity != capacity) {
        SDL_SetError("Poll arguments differ from the active archive request");
        return -1;
    }
    int status = poll(load);
    if (status == 1) load->state = BTN_LOAD_IDLE;
    return status;
}

int btn_archive_load_entry_range(BtnArchiveLoad *load, BtnArchive *archive,
    unsigned member, uint32_t offset, uint32_t length, uint8_t *buffer,
    size_t capacity, int copy, int mode, uint8_t **result)
{
    uint32_t absolute;
    if (result) *result = NULL;
    if (!result || !archive || !buffer || (copy != 0 && copy != 1)) {
        SDL_SetError("Invalid entry-range loading arguments");
        return -1;
    }
    if (!btn_archive_range(&archive->index, member, offset, length, &absolute)) return -1;
    size_t prefix = copy ? 0 : (absolute & 2047u);
    if (prefix > capacity || length > capacity - prefix) {
        SDL_SetError("Entry-range destination cannot hold prefix and payload");
        return -1;
    }
    uint8_t *destination = buffer + prefix;
    int status = btn_archive_read_async(load, archive, member, offset, length,
        destination, capacity - prefix, mode);
    if (status == 1) *result = destination;
    return status;
}

int btn_archive_start_range(BtnArchiveLoad *load, BtnArchive *archive,
    unsigned first, unsigned last, void *destination, size_t capacity)
{
    if (!load || !archive || first >= last || last > BTN_ARCHIVE_MEMBERS) {
        SDL_SetError("Invalid consecutive loading range");
        return -1;
    }
    if (load->state == BTN_LOAD_PENDING) return 1;
    uint32_t start = archive->index.offsets[first], end = archive->index.offsets[last];
    if (end < start) {
        SDL_SetError("Consecutive loading offsets decrease");
        return -1;
    }
    return queue(load, archive, first, last, 0, end - start,
        destination, capacity, true) ? 0 : -1;
}

int btn_archive_poll_range(BtnArchiveLoad *load)
{
    if (!load || load->state == BTN_LOAD_IDLE || load->state == BTN_LOAD_READY) return -1;
    if (!load->consecutive) {
        SDL_SetError("Active request uses asynchronous member polling");
        return -2;
    }
    if (load->state == BTN_LOAD_FAILED) {
        SDL_SetError("Archive loading failed: %s", load->error);
        return -2;
    }
    return poll(load) == 1 ? 1 : -2;
}

bool btn_archive_range_done(const BtnArchiveLoad *load)
{
    return load && load->consecutive && load->state == BTN_LOAD_READY;
}

bool btn_archive_read_idle(const BtnArchiveLoad *load)
{
    return !load || load->state != BTN_LOAD_PENDING;
}

void btn_archive_load_cancel(BtnArchiveLoad *load)
{
    if (load) *load = (BtnArchiveLoad){0};
}
