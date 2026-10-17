#ifndef BTN_NATIVE_ARCHIVE_LOAD_H
#define BTN_NATIVE_ARCHIVE_LOAD_H

#include "archive.h"

enum { BTN_LOAD_IDLE, BTN_LOAD_PENDING, BTN_LOAD_READY, BTN_LOAD_FAILED };
typedef struct {
    BtnArchive *archive;
    uint8_t *destination;
    size_t capacity;
    unsigned first, last;
    uint32_t offset, length;
    int state;
    bool consecutive;
    char error[256];
} BtnArchiveLoad;

/* Zero-initialize. The archive/buffer must outlive the request. One thread at
 * a time. No worker or fake completion: the first poll performs the file read.
 * mode 1 starts/replaces a request; mode 0 starts if idle, otherwise polls.
 * Return 0 pending, 1 completed, -1 error. Errors stick until a valid restart.
 * Poll arguments must identify the queued request, including its destination.
 */
int btn_archive_read_async(BtnArchiveLoad *load, BtnArchive *archive, unsigned member,
    uint32_t offset, uint32_t length, void *destination, size_t capacity, int mode);

/* Native LoadAFileEntryRange contract: copy=1 returns buffer, copy=0 returns
 * buffer + ((member_start + offset) & 2047). Only payload bytes are written;
 * sector prefix/suffix are never read outside the member. Capacity includes
 * the prefix for copy=0. *result is NULL while pending/on error.
 */
int btn_archive_load_entry_range(BtnArchiveLoad *load, BtnArchive *archive,
    unsigned member, uint32_t offset, uint32_t length, uint8_t *buffer,
    size_t capacity, int copy, int mode, uint8_t **result);

/* Native afile_start_range/poll_range/range_done contract, [first,last).
 * Start: 0 queued, 1 already busy, -1 invalid request. Poll: 1 completed,
 * -1 no active request, -2 I/O error. Completion remains visible to range_done.
 * Errors terminate instead of repeating the retail infinite retry loops.
 */
int btn_archive_start_range(BtnArchiveLoad *load, BtnArchive *archive,
    unsigned first, unsigned last, void *destination, size_t capacity);
int btn_archive_poll_range(BtnArchiveLoad *load);
bool btn_archive_range_done(const BtnArchiveLoad *load);
bool btn_archive_read_idle(const BtnArchiveLoad *load);
void btn_archive_load_cancel(BtnArchiveLoad *load);

#endif
