#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>

/* CHECK remains active in Release builds, unlike assert(). */
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #condition); exit(1); \
} } while (0)

static BtnSimulation replay(unsigned fps)
{
    BtnClock clock = {0};
    BtnInput input = {0};
    BtnSimulation simulation = {100, 100, 0};
    uint64_t previous = 0;
    for (unsigned frame = 1; frame <= fps * 10; ++frame) {
        uint64_t now = (uint64_t)frame * UINT64_C(1000000000) / fps;
        unsigned ticks = btn_clock_advance(&clock, now - previous, true);
        previous = now;
        for (unsigned i = 0; i < ticks; ++i) {
            uint32_t mask = simulation.ticks % 80 < 40 ? BTN_BIT(BTN_LEFT) : BTN_BIT(BTN_RIGHT);
            if (simulation.ticks % 60 < 30) mask |= BTN_BIT(BTN_DOWN);
            else mask |= BTN_BIT(BTN_UP);
            btn_input_set(&input, BTN_KEYBOARD, mask);
            btn_simulate(&simulation, &input);
            btn_input_end_tick(&input);
        }
    }
    CHECK(simulation.ticks == 600);
    CHECK(clock.phase == 0 && clock.dropped_ns == 0);
    return simulation;
}

int main(void)
{
    BtnSimulation baseline = replay(30);
    const unsigned rates[] = {60, 144, 1000};
    for (unsigned i = 0; i < sizeof(rates) / sizeof(rates[0]); ++i) {
        BtnSimulation other = replay(rates[i]);
        CHECK(other.x == baseline.x && other.y == baseline.y && other.ticks == baseline.ticks);
    }
    BtnInput input = {0};
    btn_input_set(&input, BTN_KEYBOARD, BTN_BIT(BTN_A));
    CHECK(input.held == BTN_BIT(BTN_A) && input.pressed == BTN_BIT(BTN_A));
    btn_input_end_tick(&input);
    btn_input_set(&input, BTN_GAMEPAD, BTN_BIT(BTN_A));
    btn_input_set(&input, BTN_KEYBOARD, 0);
    CHECK(input.held == BTN_BIT(BTN_A) && !input.pressed && !input.released);
    btn_input_set(&input, BTN_GAMEPAD, 0);
    CHECK(!input.held && input.released == BTN_BIT(BTN_A));
    btn_input_end_tick(&input);
    btn_input_set(&input, BTN_KEYBOARD, BTN_BIT(BTN_B));
    btn_input_set(&input, BTN_KEYBOARD, 0); /* A tap shorter than one tick survives. */
    CHECK(input.pressed == BTN_BIT(BTN_B) && input.released == BTN_BIT(BTN_B));
    btn_input_set(&input, BTN_STICK, BTN_BIT(BTN_LEFT));
    btn_input_clear(&input);
    CHECK(!input.held && !input.pressed);
    for (unsigned i = 0; i < BTN_INPUT_SOURCES; ++i) CHECK(!input.source[i]);
    btn_input_set(&input, BTN_INPUT_SOURCES, UINT32_MAX);
    CHECK(!input.held);

    BtnClock clock = {0};
    CHECK(btn_clock_advance(&clock, UINT64_C(5000000000), true) == 15);
    CHECK(clock.dropped_ns == UINT64_C(4750000000));
    CHECK(btn_clock_advance(&clock, UINT64_C(10000000), true) == 0);
    CHECK(btn_clock_advance(&clock, UINT64_C(1000000000), false) == 0 && !clock.phase);
    CHECK(btn_clock_advance(&clock, UINT64_C(10000000), true) == 0);
    const uint8_t address[] = {0x78, 0x56, 0x34, 0x80};
    CHECK(btn_read_le32(address) == UINT32_C(0x80345678));
    CHECK(btn_span_valid(16, 0, 16));
    CHECK(btn_span_valid(16, 16, 0));
    CHECK(!btn_span_valid(16, 17, 0));
    CHECK(!btn_span_valid(16, 8, 9));
    CHECK(!btn_span_valid(16, UINT32_MAX, UINT32_MAX));
    puts("native-core: deterministic 30/60/144/1000 FPS, input edges, pause/catch-up, LE32 and bounds passed");
    return 0;
}
