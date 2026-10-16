/* Included by src/boot.c; shares its private declarations. */
int main(void)
{
    u_long *env;
    FrameState *fb;

    inputPad1View = inputRecordsView;
    inputPad2View = &inputRecordsView[1];
    vsyncPendingCallbackView = 0;
    ResetCallback();
    CdInit();
    SetMem(2);
    VSync(3);
    VSync(3);
    VSync(3);
    VSync(3);
    VSync(3);
    VSync(3);
    VSync(3);
    VSync(3);
    VSync(3);
    VSync(3);
    screen_clear_rect(0, 0, 640, 480, 0, 0, 0);
    while (DrawSync(1) != 0) {
    }
    sound_fader_init();
    sound_init();
    SetVideoMode(0);
    ResetGraph(0);
    mainSceneView = SCENE_TITLE;
    afile_init();

    fb = frameRecordsView;
    SetDefDrawEnv(&fb[0].drawEnv, 0, 0, 320, 240);
    SetDefDrawEnv(&fb[1].drawEnv, 0, 240, 320, 240);
    SetDefDispEnv(&fb[0].dispEnv, 0, 240, 320, 240);
    SetDefDispEnv(&fb[1].dispEnv, 0, 0, 320, 240);
    fb[0].drawEnv.ofs[0] = 0;
    fb[0].drawEnv.ofs[1] = 0;
    fb[1].drawEnv.ofs[0] = 0;
    fb[1].drawEnv.ofs[1] = 240;
    fb[1].dispEnv.screen.x = 0;
    fb[0].dispEnv.screen.x = 0;
    fb[1].dispEnv.screen.y = 0;
    fb[0].dispEnv.screen.y = 0;
    SetGraphDebug(0);
    VSync(3);
    VSync(3);
    InitGeom();
    SetGeomOffset(0, 0);
    SetGeomScreen(1000);
    GsInitGraph(320, 240, 0, 0, 0);
    GsDefDispBuff(0, 0, 0, 0);
    GsSetOffset(0, 0);
    GsSetDrawBuffOffset();
    GsInit3D();
    MemCardInit(0);
    MemCardStart();
    McxStartCom();
    pad_init();
    pad_reset_records();
    screen_clear_rect(0, 0, 1023, 511, 0xFF, 0xFF, 0xFF);
    VSyncCallback(vsyncFrameCallback);
    currentFrameView = &fb[1];
    storage8005E3A4View = currentFrameView != frameRecordsView;
    env = storage8005E5D8View;
    if (activeOtView == env) {
        env += 2;
    }
    activeOtView = env;
    main_frame_reset(0);
    main_frame_begin();
    camera_init();
    frame_buffers_layout();
    farm_state_init();
    sound_init_stereo_xa();
    sound_update();
    text_set_palette(3);
    text_set_speed(3);
    primCopyPendingView = 0;
    while (1) {
        if (primCopyPendingView == 1) {
            main_copy_prim_buffer(1, 0);
        }
        main_frame_reset(0);
        if (mainClearScreenEnabledView == 1) {
            screen_clear_rect(0, 0, 320, 480, 0, 0, 0);
        }
        pad_set_repeat(20, 7);
        pad_set_repeat2(20, 7);
        mainFrameStartedView = 1;
        switch (mainSceneView) {
        case SCENE_DEBUG:
            main_set_debug_mode(1);
            LoadAFileEntryToMemory((AFILE_DEBUG), (AFILE_DEBUG) + 1,
                                   overlayRegionTableView[OVERLAY_LOW],
                                   overlayRegionTableView[OVERLAY_LOW]);
            sceneFrameHandlerView = debugTick;
            sceneOverlayLoadedView = 0;
            break;
        case SCENE_PDA_SAMPLE:
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_PDA_SAMPLE), (AFILE_PDA_SAMPLE) + 1,
                                   overlayRegionTableView[OVERLAY_LOW],
                                   overlayRegionTableView[OVERLAY_LOW]);
            sceneOverlayLoadedView = 0;
            sceneFrameHandlerView = (void (*)(void))pdaSampleRun;
            break;
        case SCENE_FARM:
            if (sceneOverlayLoadedView == 0) {
                loading_screen_init();
                main_set_debug_mode(0);
                LoadAFileEntryToMemory((AFILE_FARM), (AFILE_FARM) + 1,
                                       overlayRegionTableView[OVERLAY_LOW],
                                       overlayRegionTableView[OVERLAY_LOW]);
                sceneOverlayLoadedView = 1;
                farm_entry_args_clear();
            }
            sceneFrameHandlerView = farmEnterDispatch;
            break;
        case SCENE_MG1:
            main_frame_reset(1);
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_MG1), (AFILE_MG1) + 1,
                                   overlayRegionTableView[OVERLAY_LOW],
                                   overlayRegionTableView[OVERLAY_LOW]);
            sceneFrameHandlerView = mg1RunState;
            sceneOverlayLoadedView = 0;
            break;
        case SCENE_MG2:
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_MG2), (AFILE_MG2) + 1,
                                   overlayRegionTableView[OVERLAY_LOW],
                                   overlayRegionTableView[OVERLAY_LOW]);
            sceneFrameHandlerView = mg2PhaseRun;
            sceneOverlayLoadedView = 0;
            break;
        case SCENE_MG3:
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_MG3), (AFILE_MG3) + 1,
                                   overlayRegionTableView[OVERLAY_LOW],
                                   overlayRegionTableView[OVERLAY_LOW]);
            sceneFrameHandlerView = mg3StepDispatch;
            sceneOverlayLoadedView = 0;
            break;
        case SCENE_FARM_MINIGAME_EXIT:
            if (sceneOverlayLoadedView == 0) {
                main_set_debug_mode(0);
                LoadAFileEntryToMemory((AFILE_FARM), (AFILE_FARM) + 1,
                                       overlayRegionTableView[OVERLAY_LOW],
                                       overlayRegionTableView[OVERLAY_LOW]);
                sceneOverlayLoadedView = 1;
            }
            main_set_put_env(1);
            gameStateView.progress.entryStep = 4;
            sceneFrameHandlerView = farmEnterDispatch;
            break;
        case SCENE_SAVE:
            main_frame_reset(1);
            main_copy_prim_buffer(0, 1);
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_SAVELOAD), (AFILE_SAVELOAD) + 1,
                                   overlayRegionTableView[OVERLAY_HIGH],
                                   overlayRegionTableView[OVERLAY_HIGH]);
            gameStateView.progress.entrySubStep = 0;
            sceneFrameHandlerView = saveScreenTick;
            break;
        case SCENE_LOAD:
            main_frame_reset(1);
            main_copy_prim_buffer(0, 1);
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_SAVELOAD), (AFILE_SAVELOAD) + 1,
                                   overlayRegionTableView[OVERLAY_HIGH],
                                   overlayRegionTableView[OVERLAY_HIGH]);
            gameStateView.progress.entrySubStep = 0;
            sceneFrameHandlerView = loadScreenTick;
            break;
        case SCENE_LIVESTOCK_TRADE:
            main_frame_reset(1);
            main_copy_prim_buffer(0, 1);
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_SAVELOAD), (AFILE_SAVELOAD) + 1,
                                   overlayRegionTableView[OVERLAY_HIGH],
                                   overlayRegionTableView[OVERLAY_HIGH]);
            gameStateView.progress.entrySubStep = 0;
            sceneFrameHandlerView = saveTransferScreenTick;
            break;
        case SCENE_MG4:
            main_frame_reset(1);
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_MG4), (AFILE_MG4) + 1,
                                   overlayRegionTableView[OVERLAY_LOW],
                                   overlayRegionTableView[OVERLAY_LOW]);
            sceneFrameHandlerView = mg4RunStep;
            sceneOverlayLoadedView = 0;
            break;
        case SCENE_MG5:
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_MG5), (AFILE_MG5) + 1,
                                   overlayRegionTableView[OVERLAY_LOW],
                                   overlayRegionTableView[OVERLAY_LOW]);
            sceneFrameHandlerView = mg5StepRun;
            sceneOverlayLoadedView = 0;
            break;
        case SCENE_TITLE:
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_TITLE), (AFILE_TITLE) + 1,
                                   overlayRegionTableView[OVERLAY_LOW],
                                   overlayRegionTableView[OVERLAY_LOW]);
            farm_entry_args_clear();
            sceneFrameHandlerView = titleStepDispatch;
            sceneOverlayLoadedView = 0;
            break;
        case SCENE_FARM_LOADED:
            if (sceneOverlayLoadedView == 0) {
                loading_screen_init();
                main_set_debug_mode(0);
                LoadAFileEntryToMemory((AFILE_FARM), (AFILE_FARM) + 1,
                                       overlayRegionTableView[OVERLAY_LOW],
                                       overlayRegionTableView[OVERLAY_LOW]);
                sceneOverlayLoadedView = 1;
            }
            gameStateView.progress.entryStep = 5;
            sceneFrameHandlerView = farmEnterDispatch;
            break;
        case SCENE_FARM_SAVED:
            if (sceneOverlayLoadedView == 0) {
                main_set_debug_mode(0);
                LoadAFileEntryToMemory((AFILE_FARM), (AFILE_FARM) + 1,
                                       overlayRegionTableView[OVERLAY_HIGH],
                                       overlayRegionTableView[OVERLAY_HIGH]);
                sceneOverlayLoadedView = 1;
            }
            gameStateView.progress.entryStep = 6;
            sceneFrameHandlerView = farmEnterDispatch;
            break;
        case SCENE_STAFF_ROLL:
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_STAFF), (AFILE_STAFF) + 1,
                                   overlayRegionTableView[OVERLAY_LOW],
                                   overlayRegionTableView[OVERLAY_LOW]);
            farm_entry_args_clear();
            sceneOverlayLoadedView = 0;
            sceneFrameHandlerView = debugHandlers;
            break;
        case SCENE_MG_DEBUG:
            if (sceneOverlayLoadedView == 0) {
                LoadAFileEntryToMemory((AFILE_FARM), (AFILE_FARM) + 1,
                                       overlayRegionTableView[OVERLAY_LOW],
                                       overlayRegionTableView[OVERLAY_LOW]);
                sceneOverlayLoadedView = 1;
                loading_screen_stop();
            }
            main_frame_reset(1);
            main_copy_prim_buffer(0, 0);
            LoadAFileEntryToMemory((AFILE_SWIND), (AFILE_SWIND) + 1,
                                   overlayRegionTableView[OVERLAY_HIGH],
                                   overlayRegionTableView[OVERLAY_HIGH]);
            sceneFrameHandlerView = swindWindowTick;
            break;
        case SCENE_MEMCARD:
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_MCARD), (AFILE_MCARD) + 1,
                                   overlayRegionTableView[OVERLAY_LOW],
                                   overlayRegionTableView[OVERLAY_LOW]);
            farm_entry_args_clear();
            sceneFrameHandlerView = mcardScreenTick;
            sceneOverlayLoadedView = 0;
            break;
        case SCENE_FARM_MENU_RETURN:
            if (sceneOverlayLoadedView == 0) {
                main_set_debug_mode(0);
                LoadAFileEntryToMemory((AFILE_FARM), (AFILE_FARM) + 1,
                                       overlayRegionTableView[OVERLAY_LOW],
                                       overlayRegionTableView[OVERLAY_LOW]);
                sceneOverlayLoadedView = 1;
                gameStateView.progress.entryStep = 4;
            } else {
                sound_fade_in(10);
                fade_start_in(10, 0, 2);
                farmAreaView.mode.eventMode = FARM_MODE_IDLE;
            }
            sceneFrameHandlerView = farmEnterDispatch;
            break;
        case SCENE_FARM_STAFF_EXIT:
            main_set_debug_mode(0);
            LoadAFileEntryToMemory((AFILE_FARM), (AFILE_FARM) + 1,
                                   overlayRegionTableView[OVERLAY_LOW],
                                   overlayRegionTableView[OVERLAY_LOW]);
            sceneOverlayLoadedView = 1;
            farm_entry_args_clear();
            gameStateView.progress.entryStep = 7;
            sceneFrameHandlerView = farmEnterDispatch;
            break;
        }
        main_run_frame();
    }
}

extern s32 vsyncBeforeDraw;
extern s32 vsyncAfterDraw;
extern s32 storage8005E3A4;
extern void (*sceneFrameHandler)(void);
extern void (*vsyncPendingCallback)(void);
extern BootContext frameRecords[];
extern u8 storage8005E5D8[];
extern u8 *activeOt;
extern s32 gameState[];
extern int DrawSync(int mode);
extern int VSync(int mode);
void main_set_speed_mode(u8 value);
extern void rand_range(s32 arg0);
extern void pad_vibration_tick(void);
extern void pad_poll_ports(void);
extern void sound_load_tick(void);
extern s32 loading_screen_is_active(void);
extern void display_on(void);
void main_frame_begin(void);
extern void screen_fade_frame(void);
extern void screen_fade_step(void);
void main_draw_ots(void);

void main_run_frame(void)
{
    u8 *ctx;
    u8 *base;
    u8 *p;

    vsyncAfterDraw = 0;
    vsyncBeforeDraw = 0;
    mainLoopRunning = 1;
    main_set_speed_mode(2);
    while (DrawSync(1) != 0) {
    }
    while (mainLoopRunning == 1) {
        rand_range(0xFFFF);
        pad_vibration_tick();
        pad_poll_ports();
        sound_load_tick();
        if (loading_screen_is_active() == 1) {
            sceneFrameHandler();
            DrawSync(0);
            VSync(2);
        } else {
            display_on();
            ctx = (u8 *)frameRecords;
            base = ctx;
            if ((u8 *)currentFrame == ctx) {
                ctx += 0x134;
            }
            currentFrame = (BootContext *)ctx;
            storage8005E3A4 = ctx != base;
            p = storage8005E5D8;
            if (activeOt == p) {
                p += 8;
            }
            activeOt = p;
            main_frame_begin();
            screen_fade_frame();
            screen_fade_step();
            sceneFrameHandler();
            gameState[2]++;
            vsyncBeforeDraw = VSync(1);
            DrawSync(0);
            vsyncAfterDraw = VSync(1);
            vsyncPendingCallback = main_draw_ots;
            VSync(2);
        }
    }
}
extern void DrawOTag(u32 *p);
extern void *PutDispEnv(void *env);
extern void *PutDrawEnv(void *env);

void main_draw_ots(void)
{
    if (mainPutEnvEnabled == 1) {
        PutDispEnv((u8 *)currentFrame + 0x5C);
        PutDrawEnv(currentFrame);
    } else {
        DrawOTag((u32 *)(activeOt + 4));
        return;
    }
    if (currentFrame->ot3 != 0) {
        DrawOTag((u32 *)currentFrame->ot3 + currentFrame->len3 - 1);
    }
    if (currentFrame->ot2 != 0) {
        DrawOTag((u32 *)currentFrame->ot2 + currentFrame->len2 - 1);
    }
    DrawOTag((u32 *)currentFrame->ot4 + 0x7FF);
    if (currentFrame->ot1 != 0) {
        DrawOTag((u32 *)currentFrame->ot1 + currentFrame->len1 - 1);
    }
    if (currentFrame->ot0 != 0) {
        DrawOTag((u32 *)currentFrame->ot0 + currentFrame->len0 - 1);
    }
    DrawOTag((u32 *)(activeOt + 4));
}
void main_set_debug_mode(s32 unused) {}
void main_request_scene(s32 value)
{
    s32 previous;

    previous = mainScene;
    mainLoopRunning = 0;
    mainScene = value;
    mainPreviousScene = previous;
}
s32 main_get_previous_scene(void)
{
    return mainPreviousScene;
}
s32 main_get_scene(void)
{
    return mainScene;
}
void main_set_unused_flag(s32 value)
{
    mainUnused8005E280 = value;
}
void main_set_speed_mode(u8 value)
{
    storage8005E274 = value;
}
u8 main_get_speed_mode(void)
{
    return storage8005E274;
}
void main_set_put_env(s32 value)
{
    mainPutEnvEnabled = value;
}
void main_set_clear_screen(s32 value)
{
    mainClearScreenEnabled = value;
}
/* Bump-allocate `size` bytes off the top of the arena and return the old top.
 *
 * text.c calls this with 0 to read the current top without advancing it, so
 * the return value is the block the caller owns, not the new top.
 *
 * The bounds check is `sltu`, unsigned -- these are addresses, so retail does
 * not want the signed comparison here that vsync_elapsed wants on a counter.
 *
 * CONFIRMED: the out-of-arena arm does not return. It spins on host_poll
 * until that reports zero and only then raises SystemError(0x50, 0), which is
 * fatal -- so execution reaching the commit below implies the check passed.
 * The 0x50 lands in the branch's delay slot, i.e. it is re-set on every
 * iteration of the spin and consumed only once the loop falls through; a
 * plain statement after the loop reproduces that placement. */
void *gs_work_alloc(s32 size)
{
    s32 limit;
    s32 top;

    limit = currentFrame->field12C + currentFrame->field128;
    top = (s32)GsGetWorkBase();

    if ((u32)limit < (u32)(top + size)) {
        while (host_poll()) {
            ;
        }
        SystemError(0x50, 0);
    }

    GsSetWorkBase((void *)(top + size));

    return (void *)top;
}
void gs_work_release(void *top)
{
    if ((u32)(currentFrame->field12C + currentFrame->field128) < (u32)top) {
        while (host_poll()) {
            ;
        }
    }
    GsSetWorkBase(top);
}
s32 main_region_gap_const(void)
{
    s32 avail = regionBase - storage8001009C;

    avail = avail + 0xFFF7D000;
    return avail - (storage8007AFF4[0] - storage8001009C);
}
s32 main_region_free_bytes(void)
{
    s32 avail = 0xFFFB7000;
    s32 used = regionBase - storage8001009C;

    avail += used;
    used = avail - farmSectionSize;
    return used - (storage8007AFF4[0] - storage8001009C);
}
s32 main_region_free_start(void)
{
    return regionBase - main_region_free_bytes();
}
s32 main_get_region_base(void)
{
    return regionBase;
}
extern s32 frameOtBase;

void main_frame_begin(void)
{
    s32 k;

    GsSetWorkBase((void *)currentFrame->field12C);
    k = 0x38E50;
    frameOtBase = regionBase + (storage8005E3A4 * 0x3000 + k);
    ClearOTagR(activeOt, 2);
    if (currentFrame->ot0 != 0) {
        ClearOTagR(currentFrame->ot0, currentFrame->len0);
    }
    if (currentFrame->ot1 != 0) {
        ClearOTagR(currentFrame->ot1, currentFrame->len1);
    }
    if (currentFrame->ot2 != 0) {
        ClearOTagR(currentFrame->ot2, currentFrame->len2);
    }
    if (currentFrame->ot3 != 0) {
        ClearOTagR(currentFrame->ot3, currentFrame->len3);
    }
    ClearOTagR(currentFrame->ot4, 0x800);
    currentFrame->field118 = currentFrame->field124;
}

void main_frame_clear(void)
{
    if (currentFrame->ot0 != 0) {
        ClearOTagR(currentFrame->ot0, currentFrame->len0);
    }
    if (currentFrame->ot1 != 0) {
        ClearOTagR(currentFrame->ot1, currentFrame->len1);
    }
    if (currentFrame->ot2 != 0) {
        ClearOTagR(currentFrame->ot2, currentFrame->len2);
    }
    if (currentFrame->ot3 != 0) {
        ClearOTagR(currentFrame->ot3, currentFrame->len3);
    }
    ClearOTagR(currentFrame->ot4, 0x800);
    currentFrame->field118 = currentFrame->field124;
}
extern int DrawSync(int mode);
extern void SetGeomOffset(s32 x, s32 y);
extern void SetGeomScreen(s32 value);
extern void GsSetOffset(s32 x, s32 y);
extern void GsSetDrawBuffOffset(void);
extern void (*frameResetFuncs[])(void);
void main_frame_begin(void);

void main_frame_reset(s32 index)
{
    while (DrawSync(1) != 0) {
    }
    SetGeomOffset(0, 0);
    SetGeomScreen(1000);
    GsSetOffset(0, 0);
    GsSetDrawBuffOffset();
    frameResetFuncs[index]();
    main_frame_begin();
    main_frame_begin();
}
extern s32 frameOtBaseInit;
extern void *memset(void *dst, s32 c, u32 n);
extern void frame_buffers_layout(void);

void main_init_frame_records(void)
{
    s32 i;
    u8 *base;
    s32 one;

    base = (u8 *)0x801BE1B0;
    regionBase = 0x801BE1B0;
    memset((void *)0x801BE1B0, 0, 0x3EE50);
    frameOtBase = 0x801F7000;
    frameOtBaseInit = 0x801F7000;
    one = 1;
    for (i = 0; i < 2; i++) {
        frameRecords[i].ot0 = (u32 *)(base + i * 0x100);
        frameRecords[i].len0 = 0x40;
        frameRecords[i].count0 = 6;
        frameRecords[i].ot1 = (u32 *)(base + (i * 0x40 + 0x200));
        frameRecords[i].len1 = 0x10;
        frameRecords[i].count1 = 4;
        frameRecords[i].ot2 = (u32 *)(base + (i * 8 + 0x280));
        frameRecords[i].len2 = 2;
        frameRecords[i].count2 = one;
        frameRecords[i].ot3 = 0;
        frameRecords[i].len3 = 0;
        frameRecords[i].count3 = 0;
        frameRecords[i].field110 = 0x800;
        frameRecords[i].field114 = 0xB;
        frameRecords[i].ot4 = (u32 *)(base + (i * 0x2000 + 0x290));
        frameRecords[i].unk016 = one;
        frameRecords[i].unk017 = one;
        frameRecords[i].unk018 = one;
        frameRecords[i].unk019 = 0;
        frameRecords[i].unk01A = 0;
        frameRecords[i].unk01B = 0;
        frameRecords[i].band[0].count = 6;
        frameRecords[i].band[0].unk0C = 0;
        frameRecords[i].band[0].unk08 = 0;
        frameRecords[i].band[0].first = frameRecords[i].ot0;
        frameRecords[i].band[0].last = (u32 *)frameRecords[i].ot0 + 0x3F;
        frameRecords[i].band[1].count = 4;
        frameRecords[i].band[1].unk0C = 0;
        frameRecords[i].band[1].unk08 = 0;
        frameRecords[i].band[1].first = frameRecords[i].ot1;
        frameRecords[i].band[1].last = (u32 *)frameRecords[i].ot1 + 0xF;
        frameRecords[i].band[2].count = one;
        frameRecords[i].band[2].unk0C = 0;
        frameRecords[i].band[2].unk08 = 0;
        frameRecords[i].band[2].first = frameRecords[i].ot2;
        frameRecords[i].band[2].last = (u32 *)frameRecords[i].ot2 + 1;
        frameRecords[i].band[3].count = 0;
        frameRecords[i].band[3].unk0C = 0;
        frameRecords[i].band[3].unk08 = 0;
        frameRecords[i].band[3].first = 0;
        frameRecords[i].band[3].last = 0;
        frameRecords[i].field128 = 0x2EE0;
        frameRecords[i].field12C = (s32)(base + (i * 0x2EE0 + 0x33090));
        frameRecords[i].field130 = base + (i * 0x17700 + 0x4290);
    }
    frame_buffers_layout();
}
extern s32 frameOtBaseInit;
extern void *memset(void *dst, s32 c, u32 n);

void main_init_frame_records_wide(void)
{
    s32 i;
    u8 *base;
    s32 one;
    u8 *base2;
    u8 *ot;
    BootContext *c;
    s32 j;
    s32 k;

    base = (u8 *)0x801BE1B0;
    regionBase = 0x801BE1B0;
    memset((void *)0x801BE1B0, 0, 0x3EE50);
    frameOtBase = 0x801FCFF8;
    frameOtBaseInit = 0x801FCFF8;
    one = 1;
    for (i = 0; i < 2; i++) {
        frameRecords[i].ot0 = (u32 *)(base + i * 0x100);
        frameRecords[i].len0 = 0x40;
        frameRecords[i].count0 = 6;
        frameRecords[i].ot1 = (u32 *)(base + (i * 0x40 + 0x200));
        frameRecords[i].len1 = 0x10;
        frameRecords[i].count1 = 4;
        frameRecords[i].ot2 = (u32 *)(base + (i * 8 + 0x280));
        frameRecords[i].len2 = 2;
        frameRecords[i].count2 = one;
        frameRecords[i].ot3 = 0;
        frameRecords[i].len3 = 0;
        frameRecords[i].count3 = 0;
        frameRecords[i].field110 = 0x800;
        frameRecords[i].field114 = 0xB;
        frameRecords[i].ot4 = (u32 *)(base + (i * 0x2000 + 0x290));
        frameRecords[i].unk016 = one;
        frameRecords[i].unk017 = one;
        frameRecords[i].unk018 = one;
        frameRecords[i].unk019 = 0;
        frameRecords[i].unk01A = 0;
        frameRecords[i].unk01B = 0;
        frameRecords[i].band[0].count = 6;
        frameRecords[i].band[0].unk0C = 0;
        frameRecords[i].band[0].unk08 = 0;
        frameRecords[i].band[0].first = frameRecords[i].ot0;
        frameRecords[i].band[0].last = (u32 *)frameRecords[i].ot0 + 0x3F;
        frameRecords[i].band[1].count = 4;
        frameRecords[i].band[1].unk0C = 0;
        frameRecords[i].band[1].unk08 = 0;
        frameRecords[i].band[1].first = frameRecords[i].ot1;
        frameRecords[i].band[1].last = (u32 *)frameRecords[i].ot1 + 0xF;
        frameRecords[i].band[2].count = one;
        frameRecords[i].band[2].unk0C = 0;
        frameRecords[i].band[2].unk08 = 0;
        frameRecords[i].band[2].first = frameRecords[i].ot2;
        frameRecords[i].band[2].last = (u32 *)frameRecords[i].ot2 + 1;
        frameRecords[i].band[3].count = 0;
        frameRecords[i].band[3].unk0C = 0;
        frameRecords[i].band[3].unk08 = 0;
        frameRecords[i].band[3].first = 0;
        frameRecords[i].band[3].last = 0;
        frameRecords[i].field128 = 0x5EDC;
        frameRecords[i].field12C = (s32)(base + (i * 0x5EDC + 0x33090));
        frameRecords[i].field130 = base + (i * 0x17700 + 0x4290);
    }
    base2 = (u8 *)regionBase;
    for (k = 0; k < 2;) {
        ot = base2 + (k * 0x2000 + 0x290);
        c = &frameRecords[k];
        c->field110 = 0x800;
        c->field114 = 0xB;
        c->ot4 = ot;
        c->field11C = 0x17700;
        c->field124 = (s32)(base2 + (k * 0x17700 + 0x4290));
        ClearOTagR(ot, 0x800);
        for (j = 0; j <= 0x176FF; j++) {
            ((u8 *)c->field124)[j] = 0;
        }
        k++;
        c->field118 = c->field124;
    }
}
extern s32 primCopyPending;
extern s32 primCopyFrameCount;
extern void *memcpy(void *dst, const void *src, unsigned int n);

void main_copy_prim_buffer(s32 mode, s32 which)
{
    s32 idx;
    s32 size;

    if (mode == 0) {
        if (which == 0) {
            idx = 1;
            size = 0x10000;
        } else {
            idx = 0;
            size = currentFrame->field11C * 2;
        }
        memcpy(frameRecords[idx].field130, (void *)storage8001009C, size);
        primCopyPending = 1;
        primCopyFrameCount = which;
    } else {
        if (primCopyFrameCount == 0) {
            idx = 1;
            size = 0x10000;
        } else {
            idx = 0;
            size = currentFrame->field11C * 2;
        }
        memcpy((void *)storage8001009C, frameRecords[idx].field130, size);
        primCopyPending = 0;
    }
}
