/* Included by src/system.c; shares its private declarations. */
/* First seeded form in round 15 was 43 instructions against retail 39, with
 * seven tail differences from +0x84. This form keeps the file-size load in a
 * CdlFILE object and the read-poll jump structure explicit. */
s32 cd_read_file(char *name, u32 *buffer)
{
    CdlFILE file;
    s32 sectors;

    ResetCallback();
    DrawSync(0);
    while (CdSearchFile(&file, name) == 0) {
    }
    sectors = (file.size + 0x7FF) >> 11;
    CdControl(2, (u8 *)&file, 0);
    CdRead(sectors, buffer, 0x80);
    while (CdReadSync(1, 0) > 0) {
        VSync(0);
    }
    return 0;
}
typedef struct AFileEntry {
    u32 offset;
    u8 pad[0x24];
} AFileEntry;
extern AFileEntry afileEntryTable[];
extern s32 afileOffset;
extern s32 afileSector;
extern s32 afileBaseSector;
extern CdlLOC afileCdLoc;
extern s32 func_80035A44(s32 mode, u8 *result);
extern void *memmove(void *dst, void *src, s32 n);
