#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "archive_load.h"

static void usage(void)
{
    puts("Usage: btn-archive --data-dir PATH --archive-list\n"
        "       btn-archive --data-dir PATH --archive-dump INDEX --output NEW_FILE\n"
        "         [--offset BYTES] [--length BYTES]\n"
        "Indices: 0..31. Numbers are decimal. Default range: remaining member span.\n"
        "Output must not exist. Lists allocated spans; payload formats are unverified.");
}

static bool number(const char *text, uint32_t *out)
{
    char *end;
    if (text[0] < '0' || text[0] > '9') return false;
    errno = 0;
    unsigned long long value = strtoull(text, &end, 10);
    if (errno || *end || value > UINT32_MAX) return false;
    *out = (uint32_t)value;
    return true;
}

static FILE *create_output(const char *path)
{
    /* C11 exclusive creation also rejects aliases of the read-only inputs. */
#ifdef _WIN32
    wchar_t *wide = (wchar_t *)SDL_iconv_string("UTF-16LE", "UTF-8", path, strlen(path) + 1);
    if (!wide) return NULL;
    FILE *file = NULL;
    _wfopen_s(&file, wide, L"wbx");
    SDL_free(wide);
#else
    FILE *file = fopen(path, "wbx");
#endif
    return file;
}

static bool dump(BtnArchive *archive, unsigned member, uint32_t offset,
    uint32_t length, const char *path)
{
    uint32_t absolute;
    uint8_t buffer[64 * 1024];
    if (!btn_archive_range(&archive->index, member, offset, length, &absolute)) return false;
    FILE *output = create_output(path);
    if (!output) return SDL_SetError("Cannot create new output file (it may already exist): %s", path);
    bool ok = true;
    BtnArchiveLoad load = {0};
    uint32_t done = 0;
    while (done < length) {
        uint32_t count = length - done;
        if (count > sizeof(buffer)) count = (uint32_t)sizeof(buffer);
        if (btn_archive_read_async(&load, archive, member, offset + done, count,
                buffer, sizeof(buffer), 1) != 0
            || btn_archive_read_async(&load, archive, member, offset + done, count,
                buffer, sizeof(buffer), 0) != 1) {
            ok = false;
            break;
        }
        if (fwrite(buffer, 1, count, output) != count) {
            SDL_SetError("Could not write complete dump: %s", path);
            ok = false;
            break;
        }
        done += count;
    }
    if (fclose(output) && ok) return SDL_SetError("Could not finish dump: %s", path);
    if (ok) printf("Dumped %" PRIu32 " bytes from member %u at offset %" PRIu32 "\n", length, member, offset);
    return ok;
}

int main(int argc, char **argv)
{
    const char *directory = NULL, *output = NULL;
    uint32_t member = 0, offset = 0, length = 0;
    bool list = false, extract = false, has_offset = false, has_length = false;
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (!strcmp(arg, "--help")) { usage(); return 0; }
        if (!strcmp(arg, "--archive-list")) {
            if (list || extract) goto invalid;
            list = true;
            continue;
        }
        if (i + 1 >= argc) goto invalid;
        const char *value = argv[++i];
        if (!strcmp(arg, "--data-dir")) {
            if (directory || !value[0]) goto invalid;
            directory = value;
        } else if (!strcmp(arg, "--output")) {
            if (output || !value[0]) goto invalid;
            output = value;
        } else if (!strcmp(arg, "--archive-dump")) {
            if (list || extract || !number(value, &member) || member >= BTN_ARCHIVE_MEMBERS) goto invalid;
            extract = true;
        } else if (!strcmp(arg, "--offset")) {
            if (has_offset || !number(value, &offset)) goto invalid;
            has_offset = true;
        } else if (!strcmp(arg, "--length")) {
            if (has_length || !number(value, &length)) goto invalid;
            has_length = true;
        } else goto invalid;
    }
    if (!directory || (!list && !extract) || (list && (output || has_offset || has_length))
        || (extract && !output)) goto invalid;

    char *hdt_path = NULL, *bin_path = NULL;
    BtnArchive archive = {0};
    int result = 1;
    if (SDL_asprintf(&hdt_path, "%s/A_FILE.HDT", directory) < 0
        || SDL_asprintf(&bin_path, "%s/A_FILE.BIN", directory) < 0) goto cleanup;
    if (!btn_archive_open(&archive, hdt_path, bin_path)) goto cleanup;
    if (list) {
        printf("# members=32 hdt_extra_unclassified=%" PRIu64 " bin_prefix_unclassified=%" PRIu32
            " bin_tail_unclassified=%" PRIu32 "\n", archive.index.hdt_extra_bytes,
            archive.index.offsets[0], archive.index.bin_size - archive.index.offsets[BTN_ARCHIVE_MEMBERS]);
        puts("index\toffset\tend\tspan_bytes");
        for (unsigned i = 0; i < BTN_ARCHIVE_MEMBERS; ++i) {
            uint32_t start = archive.index.offsets[i], end = archive.index.offsets[i + 1];
            printf("%u\t%" PRIu32 "\t%" PRIu32 "\t%" PRIu32 "\n", i, start, end, end - start);
        }
        result = 0;
    } else {
        uint32_t span = archive.index.offsets[member + 1] - archive.index.offsets[member];
        if (!has_length) {
            if (offset > span) {
                SDL_SetError("Requested offset exceeds member span");
                goto cleanup;
            }
            length = span - offset;
        }
        if (dump(&archive, member, offset, length, output)) result = 0;
    }
cleanup:
    if (result) fprintf(stderr, "archive failed: %s\n", SDL_GetError());
    btn_archive_close(&archive);
    SDL_free(hdt_path);
    SDL_free(bin_path);
    return result;
invalid:
    fputs("Invalid archive arguments. Use --help.\n", stderr);
    return 2;
}
