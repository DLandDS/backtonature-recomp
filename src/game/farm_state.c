/* Included by src/ui.c; shares its private declarations. */
void farm_state_init(void)
{
    u32 size;
    u32 rounded;
    s32 top;

    memset(gameState, 0, 0xE7B8);
    memset(farmArea, 0, 0x256E4);
    farm_tables_register(0);
    size = storage8001009C - overlayRegionTable;
    rounded = (size >> 11) << 11;
    if (size & 0x7FF) {
        rounded += 0x800;
    }
    top = overlayRegionTable + rounded;
    farmArea[1] = top;
    farmArea[0] = top + 0x49000;
    *(s32 *)(gameState + 0x1C) = main_get_region_base() - 0x1008;
}
extern u8 storage8004B1A4[];
extern u8 storage80049B1C[];
extern u8 storage8004AFBC[];
extern u8 storage8004B044[];
extern u8 pictureCodes[];
extern u8 estateRecords[];
