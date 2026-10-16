/* Included by src/ui.c; shares its private declarations. */
/* Preserve the retail leaf frame for this empty helper. */
void debug_print_buf(void)
{
    u8 reserved[16];
}
void debug_stub_1(void) {}
s32 debug_stub_2(void)
{
    return 0;
}
void debug_stub_3(void) {}
s32 debug_stub_4(void)
{
    return 0;
}
void debug_stub_5(void) {}
void debug_stub_6(void) {}
void debug_stub_7(void) {}
extern void *memset(void *dst, s32 value, u32 size);
extern s32 overlayRegionTable; /* nongp */
extern s32 storage8001009C;    /* nongp */
extern s32 farmArea[];
void farm_tables_register(s32 arg0);
s32 main_get_region_base(void);
