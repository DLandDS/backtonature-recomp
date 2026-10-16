/* Included by src/system.c; shares its private declarations. */
s32 LoadAFileEntryToMemory(s32 first, s32 last, u8 *dst, u8 *buf)
{
    AFileEntry *end;
    AFileEntry *start;
    u32 s;
    u32 e;
    s32 sectors;
    s32 size;
    u8 param[4];

    ResetCallback();
    DrawSync(0);
    end = &afileEntryTable[last];
    start = &afileEntryTable[first];
    do {
        s = start->offset;
        e = end->offset;
        sectors = -(s >> 11) + (e >> 11);
        sectors += (e & 0x7FF) != 0;
        afileOffset = s & 0x7FF;
        afileSector = (s >> 11) + afileBaseSector;
        size = e - s;
        func_80035A44(0, 0);
        CdControl(2, (u8 *)CdIntToPos(afileSector, &afileCdLoc), 0);
        param[0] = 0x80;
        while (CdControl(0xE, param, 0) == 0) {
        }
        VSync(3);
        CdRead(sectors, (u32 *)buf, 0x80);
    } while (CdReadSync(0, 0) != 0);
    if (dst != buf + afileOffset) {
        memmove(dst, buf + afileOffset, size);
    }
    return 0;
}
/* Empty register ties keep both stride shifts before their independent updates. */
typedef struct {
    u8 pad00[0x24];
    u32 offset;
} AFileEntryTail;
extern AFileEntryTail afileMembers[];
