/* Exercise the private SDL event adapter without introducing a public API. */
#define BTN_SDL_TEST
#include "../native/main.c"
#include <stdio.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "line %d: %s (%s)\n", __LINE__, #condition, SDL_GetError()); exit(1); \
} } while (0)

static void key_event(App *app, const Options *options, SDL_Scancode key, bool down)
{
    SDL_Event event = {0};
    event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    event.key.scancode = key;
    event_handle(app, options, &event);
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    Options options = {NULL, NULL, 60, 0, 100,
        {SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
         SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_RETURN}, false};
    char *arguments[] = {"btn-native", "--runtime-test", "--save-dir", argv[1], "--bind", "up=W"};
    CHECK(options_parse(&options, 6, arguments) == 1);
    CHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD));
    char *owned = NULL;
    CHECK(paths_prepare(&options, &owned));
    App app = {0};
    CHECK(SDL_CreateWindowAndRenderer("input test", BTN_WIDTH, BTN_HEIGHT, 0, &app.window, &app.renderer));
    app.texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_XRGB8888,
        SDL_TEXTUREACCESS_STREAMING, BTN_WIDTH, BTN_HEIGHT);
    CHECK(app.texture);
    app.running = app.focused = true;
    key_event(&app, &options, SDL_SCANCODE_UP, true);
    CHECK(!app.input.held); /* Remapping replaces the old binding. */
    key_event(&app, &options, SDL_SCANCODE_W, true);
    CHECK(app.input.held == BTN_BIT(BTN_UP));
    SDL_Event event = {0};
    event.type = SDL_EVENT_WINDOW_FOCUS_LOST;
    event_handle(&app, &options, &event);
    CHECK(!app.input.held && !app.input.pressed && app.reset_clock);
    key_event(&app, &options, SDL_SCANCODE_Z, true);
    CHECK(!app.input.held);
    event.type = SDL_EVENT_WINDOW_FOCUS_GAINED;
    event_handle(&app, &options, &event);
    CHECK(!app.input.held);
    key_event(&app, &options, SDL_SCANCODE_Z, true);
    CHECK(app.input.held == BTN_BIT(BTN_A));
    event.type = SDL_EVENT_WINDOW_MINIMIZED;
    event_handle(&app, &options, &event);
    CHECK(!app.input.held);
    event.type = SDL_EVENT_WINDOW_RESTORED;
    event_handle(&app, &options, &event);
    CHECK(!app.input.held);

    SDL_VirtualJoystickDesc descriptor;
    SDL_INIT_INTERFACE(&descriptor);
    descriptor.type = SDL_JOYSTICK_TYPE_GAMEPAD;
    descriptor.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
    descriptor.naxes = SDL_GAMEPAD_AXIS_COUNT;
    descriptor.button_mask = (1u << SDL_GAMEPAD_BUTTON_COUNT) - 1;
    descriptor.axis_mask = (1u << SDL_GAMEPAD_AXIS_COUNT) - 1;
    descriptor.name = "BTN synthetic controller";
    SDL_JoystickID id = SDL_AttachVirtualJoystick(&descriptor);
    CHECK(id);
    event.type = SDL_EVENT_GAMEPAD_ADDED;
    event.gdevice.which = id;
    event_handle(&app, &options, &event);
    CHECK(app.gamepad && SDL_GetGamepadID(app.gamepad) == id);
    event.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
    event.gbutton.which = id;
    event.gbutton.button = SDL_GAMEPAD_BUTTON_SOUTH;
    event_handle(&app, &options, &event);
    CHECK(app.input.held == BTN_BIT(BTN_A));
    key_event(&app, &options, SDL_SCANCODE_Z, true);
    event.type = SDL_EVENT_GAMEPAD_AXIS_MOTION;
    event.gaxis.which = id;
    event.gaxis.axis = SDL_GAMEPAD_AXIS_LEFTX;
    event.gaxis.value = 16000;
    event_handle(&app, &options, &event);
    CHECK(app.input.held == (BTN_BIT(BTN_A) | BTN_BIT(BTN_RIGHT)));
    CHECK(SDL_DetachVirtualJoystick(id));
    event.type = SDL_EVENT_GAMEPAD_REMOVED;
    event.gdevice.which = id;
    event_handle(&app, &options, &event);
    CHECK(!app.gamepad && app.input.held == BTN_BIT(BTN_A));
    CHECK(!app.input.source[BTN_GAMEPAD] && !app.input.source[BTN_STICK]);
    key_event(&app, &options, SDL_SCANCODE_Z, false);
    CHECK(!app.input.held);
    CHECK(present(&app));
    event.type = SDL_EVENT_QUIT;
    event_handle(&app, &options, &event);
    CHECK(!app.running);
    SDL_DestroyTexture(app.texture);
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_free(owned);
    SDL_Quit();
    puts("native-input: remapping, focus/minimize, virtual controller attach/detach, mixed input and framebuffer passed");
    return 0;
}
