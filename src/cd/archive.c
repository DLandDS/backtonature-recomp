/* Included by src/archive.c; shares its private declarations. */
/* Archive table init. The destination is ONE byte pointer, assigned the
 * table base and then bumped by 0x24 before src is set up: that keeps the
 * %hi load, the base and the +0x24 in a single register as retail does. */
s32 afile_init(void)
{
    CdlFILE file;
    u32 *src;
    char *name = storage80010088;
    s32 i;
    s32 pos;
    u8 *dst;

    cd_read_file(storage80010078, gameState);
    dst = (u8 *)afileMembers;
    dst += 0x24;
    src = gameState;
    i = 0x20;
    do {
        *(u32 *)dst = *src++;
        dst += 0x28;
        i--;
    } while (i >= 0);
    while (CdSearchFile(&file, name) == 0) {
    }
    pos = CdPosToInt(&file.pos);
    afileSector = pos;
    afileBaseSector = pos;
    return 0;
}
extern s32 afileStartIndex;
extern s32 afileOffset;
extern CdlLOC afileCdLoc;
extern s32 func_80035A44(s32 mode, u8 *result);
/* Sector read with retry. Retail keeps the constant 1 for afileRangeActive in a
 * saved register across the retry loop: the success stores sit INSIDE the
 * loop and break out of it, so loop.c hoists the constant; return 0 follows
 * the loop. */
s32 afile_start_range(s32 startIndex, s32 endIndex, s32 arg2)
{
    u32 start;
    u32 end;
    s32 count;
    s32 buf;

    start = afileMembers[startIndex].offset;
    end = afileMembers[endIndex].offset;
    afileStartIndex = startIndex;
    count = -(start >> 11) + (end >> 11);
    count += (end & 0x7FF) != 0;
    afileSector = (start >> 11) + afileBaseSector;
    afileOffset = start & 0x7FF;
    if (afileRangeActive == 0) {
        while (1) {
            func_80035A44(0, 0);
            afileRangeStart = startIndex;
            afileRangeEnd = endIndex;
            afileRangeBuf = arg2;
            buf = arg2 - afileOffset;
            if (CdControl(2, (u8 *)CdIntToPos(afileSector, &afileCdLoc), 0) != 0 &&
                CdRead(count, (u32 *)buf, 0x80) != 0) {
                afileRangeActive = 1;
                afileBusy = 0;
                break;
            }
        }
        return 0;
    }
    return 1;
}
extern volatile s32 afileReadActive;
extern s16 afileReadEntry;
extern u32 afileReadOffset;
extern u8 *afileReadDest;
extern u32 afileReadSize;
extern s32 afileSector;
extern s32 afileBaseSector;
extern CdlLOC afileCdLoc;
extern int CdReadBreak(void);
extern void CdFlush(void);
extern s32 func_80035A44(s32 mode, u8 *result);
extern int VSync(int mode);

s32 afile_read_async(s16 entry, u32 off, u32 size, u8 *buf, s32 mode)
{
    s32 sectors;
    u8 param[8];
    s32 r;
    s32 one;

    one = 1;

    afileSector = (afileMembers[entry].offset >> 11) + (off >> 11) + afileBaseSector;
    sectors = ((size + 0x7FF) >> 11) + 1;
    if (afileReadActive == 0 || mode == 1) {
        for (;;) {
            CdReadBreak();
            CdFlush();
            func_80035A44(0, 0);
            afileReadEntry = entry;
            afileReadOffset = off;
            afileReadDest = buf;
            afileReadSize = size;
            if (CdControl(2, (u8 *)CdIntToPos(afileSector, &afileCdLoc), 0) == 0) {
                continue;
            }
            param[0] = 0x80;
            while (CdControl(0xE, param, 0) == 0) {
            }
            VSync(3);
            if (CdRead(sectors, (u32 *)buf, 0x80) != 0) {
                break;
            }
        }
        afileReadActive = 1;
        return 0;
    }
    if (afileReadActive != 0) {
        r = CdReadSync(1, 0);
        if (r != -1) {
            if (r != 0) {
                return 0;
            }
            afileReadActive = 0;
            return one;
        }
        afileReadActive = 0;
        return afile_read_async(entry, off, size, buf, mode);
    }
    return -one;
}
s32 afile_range_done(void)
{
    return afileBusy;
}
s32 afile_poll_range(void)
{
    s32 status;

    if (afileRangeActive != 0) {
        status = CdReadSync(1, 0);
        switch (status) {
        case 0:
            afileRangeActive = 0;
            afileBusy = 1;
            return 1;
        case -1:
            afileRangeActive = 0;
            afile_start_range(afileRangeStart, afileRangeEnd, afileRangeBuf);
            afileBusy = 0;
            return 0;
        default:
            afileBusy = 0;
            return 0;
        }
    }
    return -1;
}
extern s32 afileBaseSector;
extern s32 afileSector;
extern CdlLOC afileCdLoc;
extern int CdReadBreak(void);
extern void CdFlush(void);
extern s32 func_80035A44(s32 mode, u8 *result);
extern int VSync(int mode);
void afile_read_sectors(s32 unused, u32 offset, u32 size, s32 mode, u32 *buffer)
{
    u8 param[8];
    u32 start = offset >> 11;
    u32 sectors = size >> 11;
    do {
        afileSector = start + afileBaseSector;
        CdReadBreak();
        CdFlush();
        func_80035A44(0, 0);
        CdControl(2, (u8 *)CdIntToPos(afileSector, &afileCdLoc), 0);
        if (mode == -1)
            break;
        param[0] = 0x80;
        while (!CdControl(14, param, 0)) {
        }
        VSync(3);
        CdRead(sectors, buffer, 0x80);
    } while (CdReadSync(0, 0));
}
s32 afile_read_idle(void)
{
    return afileStreamState == 0;
}
extern void cd_audio_stop(void);
extern void *memmove(void *dst, const void *src, u32 n);
