#include "archive_load.h"
#include <SDL3/SDL_error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "line %d: %s (%s)\n", __LINE__, #condition, SDL_GetError()); exit(1); \
} } while (0)

static BtnArchive fixture(uint8_t *bytes, size_t size)
{
    BtnArchive archive = {0};
    archive.index.bin_size = 8192;
    for (unsigned i = 0; i < BTN_ARCHIVE_OFFSETS; ++i) archive.index.offsets[i] = 2061;
    archive.index.offsets[0] = 2047;
    archive.index.offsets[1] = archive.index.offsets[2] = 2053;
    archive.index.offsets[32] = 8192;
    archive.bin = SDL_IOFromConstMem(bytes, size);
    CHECK(archive.bin);
    return archive;
}

int main(void)
{
    uint8_t bytes[8192], buffer[4096], other[16], *result;
    for (unsigned i = 0; i < sizeof(bytes); ++i) bytes[i] = (uint8_t)(i * 13 + 5);
    BtnArchive archive = fixture(bytes, sizeof(bytes));
    BtnArchiveLoad load = {0};
    memset(buffer, 0xCC, sizeof(buffer));
    CHECK(btn_archive_read_idle(&load));
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 6, buffer, sizeof(buffer), 0) == 0);
    CHECK(buffer[0] == 0xCC && !btn_archive_read_idle(&load));
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 6, buffer, sizeof(buffer), 0) == 1);
    CHECK(!memcmp(buffer, bytes + 2047, 6) && buffer[6] == 0xCC);
    CHECK(btn_archive_read_idle(&load));

    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 6, buffer, sizeof(buffer), 1) == 0);
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 6, other, 1, 1) == -1);
    CHECK(load.destination == buffer && load.state == BTN_LOAD_PENDING);
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 6, other, sizeof(other), 0) == -1);
    CHECK(load.destination == buffer && load.state == BTN_LOAD_PENDING);
    CHECK(btn_archive_read_async(&load, &archive, 0, 1, 2, other, sizeof(other), 1) == 0);
    CHECK(btn_archive_read_async(&load, &archive, 0, 1, 2, other, sizeof(other), 0) == 1);
    CHECK(!memcmp(other, bytes + 2048, 2));
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 1, buffer, sizeof(buffer), 2) == -1);
    CHECK(btn_archive_read_async(&load, &archive, 32, 0, 0, buffer, sizeof(buffer), 1) == -1);
    CHECK(btn_archive_read_async(&load, &archive, 0, UINT32_MAX, UINT32_MAX, buffer, sizeof(buffer), 1) == -1);

    for (int copy = 0; copy <= 1; ++copy) {
        memset(buffer, 0xCC, sizeof(buffer));
        result = other;
        CHECK(btn_archive_load_entry_range(&load, &archive, 0, 0, 6,
            buffer, sizeof(buffer), copy, 1, &result) == 0 && result == NULL);
        CHECK(buffer[0] == 0xCC && !btn_archive_read_idle(&load));
        CHECK(btn_archive_load_entry_range(&load, &archive, 0, 0, 6,
            buffer, sizeof(buffer), copy, 0, &result) == 1);
        CHECK(result == buffer + (copy ? 0 : 2047));
        CHECK(!memcmp(result, bytes + 2047, 6));
        CHECK(copy || buffer[2046] == 0xCC);
        CHECK(btn_archive_read_idle(&load));
    }
    CHECK(btn_archive_load_entry_range(&load, &archive, 0, 0, 6,
        buffer, 2052, 0, 1, &result) == -1 && result == NULL);
    CHECK(btn_archive_load_entry_range(&load, &archive, 0, 0, 6,
        buffer, sizeof(buffer), 2, 1, &result) == -1);
    CHECK(btn_archive_load_entry_range(&load, &archive, 0, 0, 6,
        buffer, sizeof(buffer), 1, 1, NULL) == -1);

    CHECK(btn_archive_start_range(&load, &archive, 0, 3, buffer, 13) == -1);
    memset(buffer, 0xCC, sizeof(buffer));
    CHECK(btn_archive_start_range(&load, &archive, 0, 3, buffer, sizeof(buffer)) == 0);
    CHECK(!btn_archive_range_done(&load) && buffer[0] == 0xCC);
    CHECK(btn_archive_start_range(&load, &archive, 0, 1, other, sizeof(other)) == 1);
    CHECK(btn_archive_poll_range(&load) == 1 && btn_archive_range_done(&load));
    CHECK(!memcmp(buffer, bytes + 2047, 14) && buffer[14] == 0xCC);
    CHECK(btn_archive_poll_range(&load) == -1 && btn_archive_range_done(&load));
    CHECK(!btn_archive_read_members(&archive, 0, 3, other, 13));
    CHECK(!btn_archive_read_members(&archive, 0, 33, buffer, sizeof(buffer)));
    CHECK(!btn_archive_read_members(&archive, 3, 0, buffer, sizeof(buffer)));
    CHECK(!btn_archive_read_members(&archive, 0, 3, NULL, sizeof(buffer)));
    CHECK(btn_archive_start_range(&load, &archive, 1, 2, NULL, 0) == 0);
    CHECK(btn_archive_poll_range(&load) == 1 && btn_archive_range_done(&load));
    btn_archive_load_cancel(&load);
    CHECK(btn_archive_read_async(&load, &archive, 1, 0, 0, NULL, 0, 0) == 0);
    CHECK(btn_archive_read_async(&load, &archive, 1, 0, 0, NULL, 0, 0) == 1);
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 1, buffer, sizeof(buffer), 1) == 0);
    btn_archive_load_cancel(&load);
    CHECK(btn_archive_poll_range(&load) == -1 && !btn_archive_range_done(&load));
    btn_archive_close(&archive);

    archive = fixture(bytes, 2049);
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 6, buffer, sizeof(buffer), 1) == 0);
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 6, buffer, sizeof(buffer), 0) == -1);
    CHECK(load.state == BTN_LOAD_FAILED && !btn_archive_range_done(&load));
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 6, buffer, sizeof(buffer), 0) == -1);
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 1, buffer, sizeof(buffer), 1) == 0);
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 1, buffer, sizeof(buffer), 0) == 1);
    CHECK(btn_archive_start_range(&load, &archive, 0, 3, buffer, sizeof(buffer)) == 0);
    CHECK(btn_archive_poll_range(&load) == -2 && !btn_archive_range_done(&load));
    CHECK(btn_archive_poll_range(&load) == -2);
    btn_archive_load_cancel(&load);
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 1, buffer, sizeof(buffer), 1) == 0);
    btn_archive_close(&archive);
    CHECK(btn_archive_read_async(&load, &archive, 0, 0, 1, buffer, sizeof(buffer), 0) == -1);
    btn_archive_load_cancel(&load);
    btn_archive_load_cancel(NULL);
    puts("native-archive-load: start/poll/complete, restart/cancel, copy pointers, consecutive/empty members and I/O errors passed");
    return 0;
}
