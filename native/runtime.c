#include "runtime.h"

uint32_t btn_read_le32(const uint8_t bytes[4])
{
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8)
        | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

bool btn_span_valid(size_t total, uint32_t offset, uint32_t length)
{
    return offset <= total && length <= total - offset;
}

void btn_input_set(BtnInput *input, unsigned source, uint32_t mask)
{
    uint32_t before = input->held;
    if (source >= BTN_INPUT_SOURCES) return;
    input->source[source] = mask & ((UINT32_C(1) << BTN_ACTIONS) - 1);
    input->held = 0;
    for (unsigned i = 0; i < BTN_INPUT_SOURCES; ++i) input->held |= input->source[i];
    input->pressed |= input->held & ~before;
    input->released |= before & ~input->held;
}

void btn_input_clear(BtnInput *input)
{
    input->released |= input->held;
    input->pressed = input->held = 0;
    for (unsigned i = 0; i < BTN_INPUT_SOURCES; ++i) input->source[i] = 0;
}

void btn_input_end_tick(BtnInput *input)
{
    input->pressed = input->released = 0;
}

unsigned btn_clock_advance(BtnClock *clock, uint64_t delta_ns, bool active)
{
    const uint64_t maximum = UINT64_C(250000000);
    if (!active) {
        clock->phase = 0;
        return 0;
    }
    /* Bound catch-up to 250 ms. Do not replay an entire suspend/debugger gap. */
    if (delta_ns > maximum) {
        clock->dropped_ns += delta_ns - maximum;
        delta_ns = maximum;
    }
    clock->phase += delta_ns * BTN_TEST_HZ;
    unsigned ticks = (unsigned)(clock->phase / UINT64_C(1000000000));
    clock->phase %= UINT64_C(1000000000);
    return ticks;
}

void btn_simulate(BtnSimulation *state, const BtnInput *input)
{
    state->x += 2 * ((int)((input->held >> BTN_RIGHT) & 1u)
        - (int)((input->held >> BTN_LEFT) & 1u));
    state->y += 2 * ((int)((input->held >> BTN_DOWN) & 1u)
        - (int)((input->held >> BTN_UP) & 1u));
    if (state->x < 0) state->x = 0;
    if (state->y < 0) state->y = 0;
    if (state->x > BTN_WIDTH - 8) state->x = BTN_WIDTH - 8;
    if (state->y > BTN_HEIGHT - 8) state->y = BTN_HEIGHT - 8;
    ++state->ticks;
}
