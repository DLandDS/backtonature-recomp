/* Included by src/system.c; shares its private declarations. */
u32 afile_entry_size(s32 index)
{
    u8 *baseAddr;
    s32 curOff;
    s32 nextOff;
    u32 nextVal;
    u32 curVal;

    ResetCallback();
    DrawSync(0);
    baseAddr = (u8 *)afileMembers;
    curOff = index * 40;
    asm("" : "+r"(curOff));
    index++;
    nextOff = index * 40;
    asm("" : "+r"(nextOff));
    baseAddr += 0x24;
    nextVal = *(u32 *)(baseAddr + nextOff);
    curVal = *(u32 *)(baseAddr + curOff);
    return (nextVal - curVal + 3) / 4 * 4;
}
