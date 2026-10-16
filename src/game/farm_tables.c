/* Included by src/ui.c; shares its private declarations. */
void farm_tables_register(s32 skipRuns)
{
    u8 *work = gameState;

    *(u8 **)(work + 0x20) = storage8004B1A4;
    *(u8 **)(work + 0x24) = storage80049B1C;
    *(u8 **)(work + 0x28) = storage8004AFBC;
    *(u8 **)(work + 0x2C) = storage8004B044;
    *(u8 **)(work + 0x34) = pictureCodes;
    if (skipRuns == 0) {
        ((u8 **)(work + 0xDD8C))[0] = estateRecords + 0x0;
        ((u8 **)(work + 0xDD8C))[1] = estateRecords + 0x60;
        ((u8 **)(work + 0xDD8C))[2] = estateRecords + 0x150;
        ((u8 **)(work + 0xDD8C))[3] = estateRecords + 0x360;
        ((u8 **)(work + 0xDD8C))[4] = estateRecords + 0x480;
        ((u8 **)(work + 0xDD8C))[5] = estateRecords + 0x5A0;
        ((u8 **)(work + 0xDD8C))[6] = estateRecords + 0x6C0;
        ((u8 **)(work + 0xDD8C))[7] = estateRecords + 0x840;
        ((u8 **)(work + 0xDD8C))[8] = estateRecords + 0x960;
        ((u8 **)(work + 0xDD8C))[9] = estateRecords + 0xA08;
        ((u8 **)(work + 0xDD8C))[10] = estateRecords + 0xA80;
        ((u8 **)(work + 0xDD8C))[11] = estateRecords + 0xBB8;
        ((u8 **)(work + 0xDD8C))[12] = estateRecords + 0xCF0;
        ((u8 **)(work + 0xDD8C))[13] = estateRecords + 0xDF8;
        ((u8 **)(work + 0xDD8C))[14] = estateRecords + 0xED0;
        ((u8 **)(work + 0xDD8C))[15] = estateRecords + 0x1038;
        ((u8 **)(work + 0xDD8C))[16] = estateRecords + 0x1068;
        ((u8 **)(work + 0xDD8C))[17] = estateRecords + 0x1158;
        ((u8 **)(work + 0xDD8C))[18] = estateRecords + 0x1248;
        ((s16 *)(work + 0xDDD8))[0] = 4;
        ((s16 *)(work + 0xDDD8))[1] = 10;
        ((s16 *)(work + 0xDDD8))[2] = 22;
        ((s16 *)(work + 0xDDD8))[3] = 12;
        ((s16 *)(work + 0xDDD8))[4] = 12;
        ((s16 *)(work + 0xDDD8))[5] = 12;
        ((s16 *)(work + 0xDDD8))[6] = 16;
        ((s16 *)(work + 0xDDD8))[7] = 12;
        ((s16 *)(work + 0xDDD8))[8] = 7;
        ((s16 *)(work + 0xDDD8))[9] = 5;
        ((s16 *)(work + 0xDDD8))[10] = 13;
        ((s16 *)(work + 0xDDD8))[11] = 13;
        ((s16 *)(work + 0xDDD8))[12] = 11;
        ((s16 *)(work + 0xDDD8))[13] = 9;
        ((s16 *)(work + 0xDDD8))[14] = 15;
        ((s16 *)(work + 0xDDD8))[15] = 2;
        ((s16 *)(work + 0xDDD8))[16] = 10;
        ((s16 *)(work + 0xDDD8))[17] = 10;
        ((s16 *)(work + 0xDDD8))[18] = 2;
    }
}
void farm_entry_args_clear(void)
{
    u8 *work = gameState;
    *(u32 *)(work + 0x4488) = 0;
    *(u32 *)(work + 0x448C) = 0;
}
void *farm_work_sector_get(void)
{
    if (gameState[0xCD] != 0) {
        return 0;
    }
    return &gameState[0xD8];
}
void farm_work_sector_release(void)
{
    storage8006C905[0] = 0;
}
