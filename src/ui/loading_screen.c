/* Included by src/state_core.c; shares its private declarations. */
void loading_screen_init(void)
{
    memset(loadingPackets, 0, 0x1300);
    loading_screen_start(0, 0, 0, 0, 0, 0);
}

void loading_screen_start_forward(s16 unused0, s16 unused1, s16 unused2, s16 unused3, s16 unused4,
                                  s16 unused5)
{
    loading_screen_start(unused0, unused1, unused2, unused3, unused4, unused5);
}

extern u8 loadingAnim[];
extern u8 storage800605F0[];
extern void anim_select_range(void *entry, u8 value);

void loading_screen_start(s16 unused0, s16 unused1, s16 unused2, s16 unused3, s16 unused4,
                          s16 unused5)
{
    u8 *ot = loadingPackets;
    u8 *data;
    u8 *palette;

    display_off();
    data = storage800605F0;
    DecodeContainer0DFromStart(data + *(s32 *)(data - 4));
    palette = loadingAnim;
    anim_bind_file(palette, data);
    ClearOTagR(ot, 0x20);
    ClearOTagR(ot + 0x80, 0x20);
    anim_select_range(palette, 0);
    loadingScreenActive = 1;
}
void loading_screen_stop(void)
{
    loadingScreenActive = 0;
}

s32 loading_screen_is_active(void)
{
    return loadingScreenActive;
}
extern s32 storage8005E3A4; /* nongp */
extern u8 *currentFrame;    /* nongp */
extern u8 frameRecords[];
extern s32 loadingScreenStarted;
extern int DrawSync(int mode);
extern void display_on(void);
extern void DrawOTag(void *ot);
extern int PutDispEnv(void *env);
extern int PutDrawEnv(void *env);
extern void anim_draw_sprt_slots_advance(void *arg0, void *arg1, void *arg2, void *entry, s16 arg4,
                                         s16 arg5);

void loading_screen_draw(void)
{
    u8 *buf = loadingPackets;
    u8 *env;
    u8 *base;
    u8 *ot;
    u8 *prim;
    u8 *scratch;

    if (loadingScreenActive != 0 && loadingScreenStarted != 1) {
        loadingScreenStarted = 1;
        DrawSync(0);
        display_on();
        DrawOTag(buf + storage8005E3A4 * 0x80 + 0x7C);
        base = frameRecords;
        env = base;
        if (currentFrame == base) {
            env = base + 0x134;
        }
        currentFrame = env;
        storage8005E3A4 = env != base;
        PutDispEnv(env + 0x5C);
        PutDrawEnv(currentFrame);
        ot = buf + storage8005E3A4 * 0x80;
        prim = buf + (storage8005E3A4 * 0x280 + 0x100);
        scratch = buf + (storage8005E3A4 * 0x180 + 0x600);
        ClearOTagR(ot, 0x20);
        anim_draw_sprt_slots_advance(ot, &prim, &scratch, loadingAnim, 0xA0, 0x78);
        loadingScreenStarted = 0;
    }
}
