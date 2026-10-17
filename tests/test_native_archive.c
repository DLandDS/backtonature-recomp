#include "archive.h"
#include <SDL3/SDL_error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "line %d: %s (%s)\n", __LINE__, #condition, SDL_GetError()); exit(1); \
} } while (0)

static void word(uint8_t *bytes, uint32_t value)
{
    for (unsigned i = 0; i < 4; ++i) bytes[i] = (uint8_t)(value >> (i * 8));
}

static void fixture(uint8_t hdt[BTN_ARCHIVE_HDT_BYTES])
{
    for (unsigned i = 0; i < BTN_ARCHIVE_OFFSETS; ++i) word(hdt + i * 4, 2053);
    word(hdt, 2047);
    word(hdt + 4, 2049);
    word(hdt + 8, 2049); /* Empty member 1. */
    word(hdt + BTN_ARCHIVE_MEMBERS * 4, 4096);
}

static void parser(void)
{
    uint8_t hdt[BTN_ARCHIVE_HDT_BYTES + 2];
    BtnArchiveIndex index = {0};
    fixture(hdt);
    hdt[BTN_ARCHIVE_HDT_BYTES] = 0xFF;
    hdt[BTN_ARCHIVE_HDT_BYTES + 1] = 0;
    CHECK(btn_archive_parse(&index, hdt, sizeof(hdt), 4100));
    CHECK(index.offsets[0] == 2047 && index.offsets[1] == 2049);
    CHECK(index.offsets[31] == 2053 && index.offsets[32] == 4096);
    CHECK(index.hdt_extra_bytes == 2 && index.bin_size == 4100);
    BtnArchiveIndex before = index;
    const size_t short_sizes[] = {0, 1, 128, 131};
    for (unsigned i = 0; i < sizeof(short_sizes) / sizeof(short_sizes[0]); ++i) {
        CHECK(!btn_archive_parse(&index, hdt, short_sizes[i], 4100));
        CHECK(!memcmp(&index, &before, sizeof(index)));
    }
    CHECK(!btn_archive_parse(&index, NULL, sizeof(hdt), 4100));
    CHECK(!btn_archive_parse(&index, hdt, sizeof(hdt), UINT64_C(0x100000000)));
    CHECK(!btn_archive_parse(&index, hdt, sizeof(hdt), 4095));
    word(hdt + 8, 2048);
    CHECK(!btn_archive_parse(&index, hdt, sizeof(hdt), 4100));
    CHECK(!memcmp(&index, &before, sizeof(index)));

    memset(hdt, 0, sizeof(hdt));
    CHECK(btn_archive_parse(&index, hdt, BTN_ARCHIVE_HDT_BYTES, 0));
    CHECK(index.offsets[32] == 0 && index.bin_size == 0);
    word(hdt + 32 * 4, UINT32_MAX);
    CHECK(btn_archive_parse(&index, hdt, BTN_ARCHIVE_HDT_BYTES, UINT32_MAX));
    uint32_t absolute = 7;
    CHECK(btn_archive_range(&index, 31, UINT32_MAX, 0, &absolute) && absolute == UINT32_MAX);
    CHECK(!btn_archive_range(&index, 31, UINT32_MAX, 1, &absolute));
    CHECK(!btn_archive_range(&index, 31, UINT32_MAX - 1, UINT32_MAX, &absolute));
    CHECK(absolute == UINT32_MAX);
}

static void reads(void)
{
    uint8_t hdt[BTN_ARCHIVE_HDT_BYTES], bytes[4096], result[8];
    BtnArchive archive = {0};
    fixture(hdt);
    for (unsigned i = 0; i < sizeof(bytes); ++i) bytes[i] = (uint8_t)(i * 17 + 3);
    CHECK(btn_archive_parse(&archive.index, hdt, sizeof(hdt), sizeof(bytes)));
    archive.bin = SDL_IOFromConstMem(bytes, sizeof(bytes));
    CHECK(archive.bin);
    memset(result, 0xCC, sizeof(result));
    CHECK(btn_archive_read(&archive, 0, 0, 2, result + 1, 2));
    CHECK(!memcmp(result + 1, bytes + 2047, 2));
    CHECK(result[0] == 0xCC && result[3] == 0xCC); /* No sector over-read/write. */
    CHECK(btn_archive_read(&archive, 31, 2042, 1, result, sizeof(result)));
    CHECK(result[0] == bytes[4095]);
    CHECK(btn_archive_read(&archive, 0, 1, 1, result, sizeof(result)));
    CHECK(result[0] == bytes[2048]); /* Backward seek, partial read. */
    CHECK(btn_archive_read(&archive, 0, 2, 0, NULL, 0));
    CHECK(btn_archive_read(&archive, 1, 0, 0, NULL, 0));
    CHECK(!btn_archive_read(&archive, 1, 0, 1, result, sizeof(result)));
    CHECK(!btn_archive_read(&archive, 0, 3, 0, result, sizeof(result)));
    CHECK(!btn_archive_read(&archive, 32, 0, 0, result, sizeof(result)));
    CHECK(!btn_archive_read(&archive, UINT32_MAX, 0, 0, result, sizeof(result)));
    CHECK(!btn_archive_read(&archive, 0, UINT32_MAX, UINT32_MAX, result, sizeof(result)));
    memset(result, 0xCC, sizeof(result));
    CHECK(!btn_archive_read(&archive, 0, 0, 2, result, 1));
    CHECK(result[0] == 0xCC);
    CHECK(!btn_archive_read(&archive, 0, 0, 2, NULL, 2));
    uint32_t absolute = 7;
    CHECK(!btn_archive_range(&archive.index, 32, 0, 0, &absolute) && absolute == 7);
    CHECK(!btn_archive_open(&archive, "unused", "unused"));
    btn_archive_close(&archive);
    CHECK(!archive.bin && archive.index.bin_size == 0);
    CHECK(!btn_archive_read(&archive, 0, 0, 0, NULL, 0));
    btn_archive_close(&archive);
    btn_archive_close(NULL);

    /* A file shortened after indexing must fail, never report a full read. */
    CHECK(btn_archive_parse(&archive.index, hdt, sizeof(hdt), sizeof(bytes)));
    archive.bin = SDL_IOFromConstMem(bytes, 2048);
    CHECK(archive.bin);
    CHECK(!btn_archive_read(&archive, 0, 0, 2, result, sizeof(result)));
    btn_archive_close(&archive);
}

int main(void)
{
    parser();
    reads();
    puts("native-archive: LE32, 32 spans, empty/edge/unaligned ranges, overflow, capacity and short reads passed");
    return 0;
}
