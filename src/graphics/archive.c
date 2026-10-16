/* Included by src/archive.c; shares its private declarations. */
u8 *LoadAFileEntryRange(s32 entry, u32 off, u32 size, u8 *buf, s32 copy, s32 mode)
{
    u8 *result;
    u32 start;
    u32 end;
    u32 first;
    u32 skip;
    s32 sectors;
    s32 one;
    s32 r;

    result = 0;
    start = afileMembers[entry].offset + off;
    end = start + size;
    r = end >> 11;
    r += (end & 0x7FF) != 0;
    first = start >> 11;
    skip = start & 0x7FF;
    sectors = r - first;
    if (afileReadActive == 0 || mode == (one = 1)) {
        cd_audio_stop();
        afileSector = first + afileBaseSector;
        CdReadBreak();
        CdFlush();
        func_80035A44(0, 0);
        CdControl(2, (u8 *)CdIntToPos(afileSector, &afileCdLoc), 0);
        CdRead(sectors, (u32 *)buf, 0x80);
        afileReadActive = 1;
        if (mode == 1) {
            afileStreamState = mode;
        }
    } else {
        r = CdReadSync(1, 0);
        if (r == -1) {
            goto retry;
        }
        if (r != 0) {
            goto busy;
        }
        if (*&copy == one) {
            if (buf != buf + skip) {
                memmove(buf, buf + skip, size);
            }
            result = buf;
        } else {
            result = buf + skip;
        }
        afileReadActive = 0;
        afileStreamState = 0;
        goto out;
    retry:
        afileReadActive = 0;
        LoadAFileEntryRange(entry, *&off, size, buf, copy, mode);
    busy:
        result = 0;
        afileStreamState = 1;
    }
out:
    return result;
}
