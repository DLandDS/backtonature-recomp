#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"
#include "title.h"

static const char *action_names[BTN_ACTIONS] = {"up", "down", "left", "right", "a", "b", "start"};

typedef struct {
    const char *data_dir, *save_dir;
    unsigned fps, frames, volume;
    SDL_Scancode keys[BTN_ACTIONS];
    bool runtime_test;
    bool title, headless;
    const char *input_script, *output;
} Options;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Gamepad *gamepad;
    BtnInput input;
    bool keys[SDL_SCANCODE_COUNT];
    bool focused, minimized, running, reset_clock;
    BtnClock clock;
    BtnSimulation simulation;
    uint32_t pixels[BTN_WIDTH * BTN_HEIGHT];
    BtnTitle title;
    SDL_AudioStream *audio;
    bool handoff_reported;
} App;

static void usage(void)
{
    SDL_Log("Usage: btn-native --runtime-test [--fps 1..1000] [--frames N]\n"
        "       btn-native --title --data-dir PATH [--headless --frames N]\n"
        "         [--input-script FILE] [--output NEW.bmp]\n"
        "  [--data-dir PATH] [--save-dir PATH] [--volume 0..100]\n"
        "  [--bind up|down|left|right|a|b|start=SDL_SCANCODE_NAME]\n"
        "Defaults: arrows, Z, X, Return. Escape exits; F11 toggles fullscreen.\n"
        "Runtime test: synthetic 60 Hz. Title: native 30 Hz title/setup with original assets/music.\n"
        "Gameplay/save-load and PS1 parity are unavailable; setup provides a typed native handoff.\n"
        "Title: Return opens menu, up/down selects, Z/Return confirms, X returns.\n"
        "Replay: headless-only LE32 tick/mask pairs, strictly increasing ticks from zero.");
}

static bool number(const char *text, unsigned minimum, unsigned maximum, unsigned *out)
{
    char *end;
    unsigned long value;
    if (!text[0] || text[0] < '0' || text[0] > '9') return false;
    errno = 0;
    value = strtoul(text, &end, 10);
    if (errno || *end || value < minimum || value > maximum) return false;
    *out = (unsigned)value;
    return true;
}

/* Returns 0 for help, 1 to run, -1 on invalid configuration. */
static int options_parse(Options *options, int argc, char **argv)
{
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (!strcmp(arg, "--help")) { usage(); return 0; }
        if (!strcmp(arg, "--runtime-test")) { options->runtime_test = true; continue; }
        if (!strcmp(arg, "--title")) { options->title = true; continue; }
        if (!strcmp(arg, "--headless")) { options->headless = true; continue; }
        if (i + 1 >= argc) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Missing value or unknown option: %s", arg);
            return -1;
        }
        const char *value = argv[++i];
        if (!strcmp(arg, "--data-dir")) options->data_dir = value;
        else if (!strcmp(arg,"--input-script")) options->input_script=value;
        else if (!strcmp(arg,"--output")) options->output=value;
        else if (!strcmp(arg, "--save-dir")) options->save_dir = value;
        else if (!strcmp(arg, "--fps")) {
            if (!number(value, 1, 1000, &options->fps)) goto invalid;
        } else if (!strcmp(arg, "--frames")) {
            if (!number(value, 1, 1000000, &options->frames)) goto invalid;
        } else if (!strcmp(arg, "--volume")) {
            if (!number(value, 0, 100, &options->volume)) goto invalid;
        } else if (!strcmp(arg, "--bind")) {
            const char *equals = strchr(value, '=');
            bool found = false;
            if (!equals) goto invalid;
            for (unsigned action = 0; action < BTN_ACTIONS; ++action) {
                size_t length = strlen(action_names[action]);
                if ((size_t)(equals - value) != length || strncmp(value, action_names[action], length)) continue;
                SDL_Scancode key = SDL_GetScancodeFromName(equals + 1);
                if (key == SDL_SCANCODE_UNKNOWN || key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_F11) goto invalid;
                options->keys[action] = key;
                found = true;
                break;
            }
            if (!found) goto invalid;
        } else goto invalid;
        continue;
invalid:
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid option/value: %s %s", arg, value);
        return -1;
    }
    if (options->runtime_test==options->title || (options->title&&!options->data_dir)
        || (!options->title&&(options->headless||options->input_script||options->output))
        || ((options->input_script||options->output)&&!options->headless)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Select --runtime-test or --title --data-dir PATH; replay/output require headless title.");
        return -1;
    }
    if (options->headless&&!options->frames) options->frames=1;
    return 1;
}

static bool paths_prepare(Options *options, char **owned_save_dir)
{
    SDL_PathInfo info;
    if (options->data_dir) {
        const char *names[] = {"A_FILE.HDT", "A_FILE.BIN"};
        for (unsigned i = 0; i < 2; ++i) {
            char *path = NULL;
            if (SDL_asprintf(&path, "%s/%s", options->data_dir, names[i]) < 0) return false;
            bool valid = SDL_GetPathInfo(path, &info) && info.type == SDL_PATHTYPE_FILE && info.size > 0;
            if (!valid) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Required data file missing or empty: %s", path);
            SDL_free(path);
            if (!valid) return false;
        }
        SDL_Log("Data files exist; selected resources are validated by their loaders.");
    }
    if (options->title) return true;
    if (!options->save_dir) {
        *owned_save_dir = SDL_GetPrefPath("backtonature-recomp", "native");
        options->save_dir = *owned_save_dir;
    }
    if (!options->save_dir || !options->save_dir[0] || !SDL_CreateDirectory(options->save_dir)
        || !SDL_GetPathInfo(options->save_dir, &info) || info.type != SDL_PATHTYPE_DIRECTORY) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot prepare save directory '%s': %s",
            options->save_dir ? options->save_dir : "(default)", SDL_GetError());
        return false;
    }
    SDL_Log("Save directory: %s (save/load not implemented); volume: %u%% (audio playback not implemented)",
        options->save_dir, options->volume);
    return true;
}

static void open_gamepad(App *app)
{
    int count = 0;
    SDL_JoystickID *ids;
    if (app->gamepad) return;
    ids = SDL_GetGamepads(&count);
    for (int i = 0; i < count; ++i) {
        app->gamepad = SDL_OpenGamepad(ids[i]);
        if (app->gamepad) {
            SDL_Log("Controller connected: %s", SDL_GetGamepadName(app->gamepad));
            break;
        }
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Cannot open controller: %s", SDL_GetError());
    }
    SDL_free(ids);
}

static uint32_t gamepad_button(Uint8 button)
{
    switch (button) {
    case SDL_GAMEPAD_BUTTON_DPAD_UP: return BTN_BIT(BTN_UP);
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return BTN_BIT(BTN_DOWN);
    case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return BTN_BIT(BTN_LEFT);
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return BTN_BIT(BTN_RIGHT);
    case SDL_GAMEPAD_BUTTON_SOUTH: return BTN_BIT(BTN_A);
    case SDL_GAMEPAD_BUTTON_EAST: return BTN_BIT(BTN_B);
    case SDL_GAMEPAD_BUTTON_START: return BTN_BIT(BTN_START);
    default: return 0;
    }
}

static void event_handle(App *app, const Options *options, const SDL_Event *event)
{
    switch (event->type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        app->running = false;
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST: app->focused = false; app->reset_clock = true; break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED: app->focused = true; app->reset_clock = true; break;
    case SDL_EVENT_WINDOW_MINIMIZED: app->minimized = true; app->reset_clock = true; break;
    case SDL_EVENT_WINDOW_RESTORED: app->minimized = false; app->reset_clock = true; break;
    case SDL_EVENT_GAMEPAD_ADDED: open_gamepad(app); break;
    case SDL_EVENT_GAMEPAD_REMOVED:
        if (app->gamepad && event->gdevice.which == SDL_GetGamepadID(app->gamepad)) {
            SDL_CloseGamepad(app->gamepad);
            app->gamepad = NULL;
            btn_input_set(&app->input, BTN_GAMEPAD, 0);
            btn_input_set(&app->input, BTN_STICK, 0);
            SDL_Log("Controller disconnected; controller input cleared.");
            open_gamepad(app);
        }
        break;
    default: break;
    }
    if (!app->focused || app->minimized) {
        memset(app->keys, 0, sizeof(app->keys));
        btn_input_clear(&app->input);
        app->clock.phase = 0;
        return;
    }
    if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP) {
        if (event->key.repeat) return;
        bool down = event->type == SDL_EVENT_KEY_DOWN;
        SDL_Scancode key = event->key.scancode;
        if (down && key == SDL_SCANCODE_ESCAPE) app->running = false;
        if (down && key == SDL_SCANCODE_F11) {
            bool fullscreen = (SDL_GetWindowFlags(app->window) & SDL_WINDOW_FULLSCREEN) != 0;
            if (!SDL_SetWindowFullscreen(app->window, !fullscreen))
                SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Fullscreen change failed: %s", SDL_GetError());
        }
        if (key > SDL_SCANCODE_UNKNOWN && key < SDL_SCANCODE_COUNT) app->keys[key] = down;
        uint32_t mask = 0;
        for (unsigned action = 0; action < BTN_ACTIONS; ++action)
            if (app->keys[options->keys[action]]) mask |= BTN_BIT(action);
        btn_input_set(&app->input, BTN_KEYBOARD, mask);
    }
    if ((event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || event->type == SDL_EVENT_GAMEPAD_BUTTON_UP)
        && app->gamepad && event->gbutton.which == SDL_GetGamepadID(app->gamepad)) {
        uint32_t button = gamepad_button(event->gbutton.button);
        uint32_t mask = app->input.source[BTN_GAMEPAD];
        if (event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) mask |= button;
        else mask &= ~button;
        btn_input_set(&app->input, BTN_GAMEPAD, mask);
    }
    if (event->type == SDL_EVENT_GAMEPAD_AXIS_MOTION && app->gamepad
        && event->gaxis.which == SDL_GetGamepadID(app->gamepad)) {
        uint32_t mask = app->input.source[BTN_STICK];
        int value = event->gaxis.value;
        if (event->gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) {
            mask &= ~(BTN_BIT(BTN_LEFT) | BTN_BIT(BTN_RIGHT));
            if (value < -8000) mask |= BTN_BIT(BTN_LEFT);
            if (value > 8000) mask |= BTN_BIT(BTN_RIGHT);
        } else if (event->gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) {
            mask &= ~(BTN_BIT(BTN_UP) | BTN_BIT(BTN_DOWN));
            if (value < -8000) mask |= BTN_BIT(BTN_UP);
            if (value > 8000) mask |= BTN_BIT(BTN_DOWN);
        }
        btn_input_set(&app->input, BTN_STICK, mask);
    }
}

static bool present(App *app)
{
    if (app->title.assets) {
        if (!btn_title_draw(&app->title,app->pixels,BTN_WIDTH*BTN_HEIGHT)
            || !SDL_UpdateTexture(app->texture,NULL,app->pixels,BTN_WIDTH*(int)sizeof(uint32_t))
            || !SDL_SetRenderDrawColor(app->renderer,0,0,0,255) || !SDL_RenderClear(app->renderer)
            || !SDL_RenderTexture(app->renderer,app->texture,NULL,NULL)
            || !SDL_SetRenderDrawColor(app->renderer,255,255,255,255)
            || !SDL_RenderDebugText(app->renderer,4,4,"NATIVE TITLE / PS1 check pending")) return false;
        if (app->title.state.page==BTN_TITLE_UNAVAILABLE) {
            if (!SDL_SetRenderDrawColor(app->renderer,0,0,0,255)
                || !SDL_RenderFillRect(app->renderer,&(SDL_FRect){0,184,320,56})
                || !SDL_SetRenderDrawColor(app->renderer,255,255,255,255)
                || !SDL_RenderDebugText(app->renderer,4,188,
                    "Continue unavailable\nSave/load requires phase 07\nX/Return: back")) return false;
        } else if (app->title.state.page!=BTN_TITLE_SETUP && app->title.state.page!=BTN_TITLE_READY
            && !SDL_RenderDebugText(app->renderer,4,224,"Enter: Start  Z: choose  X: back")) return false;
        return SDL_RenderPresent(app->renderer);
    }
    for (int y = 0; y < BTN_HEIGHT; ++y)
        for (int x = 0; x < BTN_WIDTH; ++x)
            app->pixels[y * BTN_WIDTH + x] = ((x / 16 + y / 16) % 2) ? 0x00182330u : 0x001e2b3au;
    for (int y = app->simulation.y; y < app->simulation.y + 8; ++y)
        for (int x = app->simulation.x; x < app->simulation.x + 8; ++x)
            app->pixels[y * BTN_WIDTH + x] = app->input.held & BTN_BIT(BTN_A) ? 0x00ffcc55u : 0x0055dd99u;
    return SDL_UpdateTexture(app->texture, NULL, app->pixels, BTN_WIDTH * (int)sizeof(uint32_t))
        && SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255)
        && SDL_RenderClear(app->renderer)
        && SDL_RenderTexture(app->renderer, app->texture, NULL, NULL)
        && SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255)
        && SDL_RenderDebugText(app->renderer, 8, 8, "BTN SYNTHETIC RUNTIME / 60 Hz")
        && SDL_RenderDebugTextFormat(app->renderer, 8, 24, "Ticks: %llu  Input: %02x",
            (unsigned long long)app->simulation.ticks, (unsigned)app->input.held)
        && SDL_RenderDebugText(app->renderer, 8, 216, "Arrows/Z/X/Return - F11 - Escape")
        && SDL_RenderPresent(app->renderer);
}

#ifndef BTN_SDL_TEST
static unsigned name_length(const uint16_t name[9])
{
    unsigned length=0;
    while (length<8 && name[length]!=0xFFFF) ++length;
    return length;
}

static bool replay_load(const char *path, uint8_t **data, size_t *size, unsigned frames)
{
    if (!path) return true;
    SDL_IOStream *file=SDL_IOFromFile(path,"rb");
    if (!file) return false;
    Sint64 n=SDL_GetIOSize(file);
    bool ok=n>0 && n<=1024*1024 && n%8==0;
    if (ok) {
        *data=malloc((size_t)n); *size=(size_t)n;
        ok=*data && SDL_ReadIO(file,*data,*size)==*size;
    }
    if (!SDL_CloseIO(file)) ok=false;
    for (size_t i=0; ok && i<*size; i+=8) {
        uint32_t tick=btn_read_le32(*data+i), mask=btn_read_le32(*data+i+4);
        if (tick>=frames || mask>=(1u<<BTN_ACTIONS) || (!i&&tick!=0)
            || (i&&tick<=btn_read_le32(*data+i-8))) ok=false;
    }
    return ok ? true : SDL_SetError("Replay requires LE32 tick/mask pairs, first tick 0, increasing ticks below --frames");
}

static bool capture(const char *path, App *app)
{
    SDL_PathInfo info;
    if (SDL_GetPathInfo(path,&info)) return SDL_SetError("Capture output already exists: %s",path);
    SDL_ClearError();
    SDL_Surface *surface=SDL_CreateSurfaceFrom(BTN_WIDTH,BTN_HEIGHT,SDL_PIXELFORMAT_XRGB8888,
        app->pixels,BTN_WIDTH*(int)sizeof(uint32_t));
    if (!surface) return false;
    bool ok=SDL_SaveBMP(surface,path); SDL_DestroySurface(surface); return ok;
}

static bool title_audio_queue(App *app, unsigned volume, bool active)
{
    if (!app->audio) return true;
    if (!active) return SDL_PauseAudioStreamDevice(app->audio) && SDL_ClearAudioStream(app->audio);
    if (!SDL_ResumeAudioStreamDevice(app->audio)) return false;
    int16_t pcm[735*2];
    int queued=SDL_GetAudioStreamQueued(app->audio);
    if (queued<0) return false;
    /* Main thread produces at most 100 ms; SDL consumes PCM on its device thread. */
    while (queued<(int)(BTN_AUDIO_RATE*4u/10u)) {
        if (!btn_title_audio(&app->title,pcm,735)) return false;
        for (unsigned i=0; i<735*2; ++i) pcm[i]=(int16_t)((int)pcm[i]*(int)volume/100);
        if (!SDL_PutAudioStreamData(app->audio,pcm,(int)sizeof(pcm))) return false;
        queued+=(int)sizeof(pcm);
    }
    return true;
}

int main(int argc, char **argv)
{
    Options options = {NULL, NULL, 60, 0, 100,
        {SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
         SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_RETURN}, false};
    App app = {0};
    BtnArchive archive={0}; uint8_t *replay=NULL; size_t replay_size=0,replay_cursor=0;
    char *owned_save_dir = NULL;
    int parsed = options_parse(&options, argc, argv);
    if (parsed <= 0) return parsed == 0 ? 0 : 2;
    int result = 1;
    unsigned frames = 0;
    if (!SDL_Init(options.headless ? 0 : SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | (options.title ? SDL_INIT_AUDIO : 0))) goto cleanup;
    if (!paths_prepare(&options, &owned_save_dir)) goto cleanup;
    if (options.title) {
        char *hdt=NULL,*bin=NULL;
        if (SDL_asprintf(&hdt,"%s/A_FILE.HDT",options.data_dir)<0) goto cleanup;
        if (SDL_asprintf(&bin,"%s/A_FILE.BIN",options.data_dir)<0) { SDL_free(hdt); goto cleanup; }
        bool opened=btn_archive_open(&archive,hdt,bin);
        SDL_free(hdt); SDL_free(bin);
        if (!opened || !btn_title_open(&app.title,&archive,options.data_dir)) goto cleanup;
        SDL_Log("Title preview: CPU-native Start/menu branch; %s",btn_title_notice(&app.title.state));
        SDL_Log("Native intro/scenery and setup; unsupported: attract gameplay, farm gameplay, save/load and PS1 parity. Audio is approximate dry playback.");
        if (!options.headless) {
            SDL_AudioSpec spec={SDL_AUDIO_S16,2,BTN_AUDIO_RATE};
            app.audio=SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,&spec,NULL,NULL);
            if (!app.audio) goto cleanup;
        }
    }
    if (!replay_load(options.input_script,&replay,&replay_size,options.frames)) goto cleanup;
    if (!options.headless) {
        if (!SDL_CreateWindowAndRenderer(options.title ? "BTN - native title preview" : "BTN - synthetic runtime", 960, 720,
            SDL_WINDOW_RESIZABLE, &app.window, &app.renderer)) goto cleanup;
        if (!SDL_SetRenderLogicalPresentation(app.renderer, BTN_WIDTH, BTN_HEIGHT,
            SDL_LOGICAL_PRESENTATION_LETTERBOX)) goto cleanup;
        app.texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_XRGB8888,
            SDL_TEXTUREACCESS_STREAMING, BTN_WIDTH, BTN_HEIGHT);
        if (!app.texture || !SDL_SetTextureScaleMode(app.texture, SDL_SCALEMODE_NEAREST)) goto cleanup;
        if (!SDL_SetRenderVSync(app.renderer, 0))
            SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "VSync disable unavailable: %s", SDL_GetError());
        open_gamepad(&app);
    }
    app.focused = app.running = true;
    app.simulation.x = BTN_WIDTH / 2;
    app.simulation.y = BTN_HEIGHT / 2;
    unsigned hz=options.title ? BTN_TITLE_HZ : BTN_TEST_HZ;
    SDL_Log("SDL %d; %s %u Hz; presentation cap %u FPS",SDL_GetVersion(),
        options.title ? "partial native title" : "synthetic simulation",hz,options.fps);
    uint64_t previous = SDL_GetTicksNS();
    while (app.running && (!options.frames || frames < options.frames)) {
        uint64_t start = SDL_GetTicksNS();
        SDL_Event event;
        while (!options.headless && SDL_PollEvent(&event)) event_handle(&app, &options, &event);
        if (!app.running) break;
        unsigned ticks = options.headless ? 1u : btn_clock_advance_hz(&app.clock, app.reset_clock ? 0 : start - previous,
            app.focused && !app.minimized,hz);
        app.reset_clock = false;
        previous = start;
        for (unsigned i = 0; i < ticks; ++i) {
            if (options.headless && replay_cursor<replay_size && btn_read_le32(replay+replay_cursor)==frames) {
                btn_input_set(&app.input,BTN_KEYBOARD,btn_read_le32(replay+replay_cursor+4)); replay_cursor+=8;
            }
            if (options.title) {
                BtnTitlePage before=app.title.state.page; unsigned choice=app.title.state.selection;
                BtnSetupStage setup_before=app.title.setup.stage;
                bool confirmed=app.title.setup.confirming;
                BtnDialogPhase dialog_before=app.title.dialog.phase;
                BtnDialogKind dialog_kind=app.title.dialog.kind;
                if (!btn_title_tick(&app.title,&app.input)) goto cleanup;
                if (before!=app.title.state.page || choice!=app.title.state.selection)
                    SDL_Log("title: tick=%u page=%u selection=%u %s",app.title.state.ticks-1,
                        (unsigned)app.title.state.page,app.title.state.selection,btn_title_notice(&app.title.state));
                if (setup_before!=app.title.setup.stage || confirmed!=app.title.setup.confirming)
                    SDL_Log("title setup: tick=%u stage=%u length=%u confirming=%u",app.title.state.ticks-1,
                        (unsigned)app.title.setup.stage,app.title.setup.length,(unsigned)app.title.setup.confirming);
                if (dialog_before!=app.title.dialog.phase || dialog_kind!=app.title.dialog.kind)
                    SDL_Log("title dialog: tick=%u kind=%u phase=%u",app.title.state.ticks-1,
                        (unsigned)app.title.dialog.kind,(unsigned)app.title.dialog.phase);
                const BtnNewGame *ready=btn_title_new_game(&app.title);
                if (!ready) app.handoff_reported=false;
                else if (!app.handoff_reported) {
                    SDL_Log("title handoff: names=%u/%u/%u birthday=%u/%u flags=%08x",name_length(ready->player),
                        name_length(ready->farm),name_length(ready->dog),(unsigned)ready->season,(unsigned)ready->day,
                        (unsigned)ready->flags);
                    app.handoff_reported=true;
                }
            } else btn_simulate(&app.simulation, &app.input);
            btn_input_end_tick(&app.input);
        }
        if (options.headless) {
            int16_t pcm[(BTN_AUDIO_RATE/BTN_TITLE_HZ)*2];
            if (!btn_title_draw(&app.title,app.pixels,BTN_WIDTH*BTN_HEIGHT)
                || !btn_title_audio(&app.title,pcm,BTN_AUDIO_RATE/BTN_TITLE_HZ)) goto cleanup;
        } else if (!title_audio_queue(&app,options.volume,app.focused&&!app.minimized) || !present(&app)) goto cleanup;
        ++frames;
        uint64_t elapsed = SDL_GetTicksNS() - start;
        uint64_t duration = UINT64_C(1000000000) / options.fps;
        if (!options.headless && elapsed < duration) SDL_DelayNS(duration - elapsed);
    }
    if (options.output && !capture(options.output,&app)) goto cleanup;
    if (options.title) SDL_Log("title stopped: ticks=%u page=%u selection=%u",app.title.state.ticks,
        (unsigned)app.title.state.page,app.title.state.selection);
    SDL_Log("Runtime stopped: frames=%u ticks=%llu dropped_ns=%llu", frames,
        (unsigned long long)(options.title ? app.title.state.ticks : app.simulation.ticks),
        (unsigned long long)app.clock.dropped_ns);
    result = 0;
cleanup:
    if (result) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Runtime failed: %s", SDL_GetError());
    SDL_CloseGamepad(app.gamepad);
    SDL_DestroyAudioStream(app.audio); btn_title_close(&app.title); btn_archive_close(&archive); free(replay);
    SDL_DestroyTexture(app.texture);
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_free(owned_save_dir);
    SDL_Quit();
    return result;
}
#endif
