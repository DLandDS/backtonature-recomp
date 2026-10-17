#ifndef BTN_NATIVE_RUNTIME_H
#define BTN_NATIVE_RUNTIME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum { BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_A, BTN_B, BTN_START, BTN_ACTIONS };
enum { BTN_KEYBOARD, BTN_GAMEPAD, BTN_STICK, BTN_INPUT_SOURCES };
#define BTN_BIT(action) (UINT32_C(1) << (action))
#define BTN_TEST_HZ 60u /* Synthetic runtime frequency; not a measured BTN rate. */
#define BTN_WIDTH 320
#define BTN_HEIGHT 240

typedef struct {
    uint32_t source[BTN_INPUT_SOURCES];
    uint32_t held, pressed, released;
} BtnInput;

typedef struct {
    uint64_t phase, dropped_ns;
} BtnClock;

typedef struct {
    int x, y;
    uint64_t ticks;
} BtnSimulation;

/* Serialized PS1 addresses stay 32-bit. They are never host pointers.
 * No retail code/overlay is linked yet; address relocation requires its audit.
 */
typedef uint32_t BtnPs1Address;
_Static_assert(sizeof(BtnPs1Address) == 4, "PS1 addresses must occupy four bytes");

uint32_t btn_read_le32(const uint8_t bytes[4]);
bool btn_span_valid(size_t total, uint32_t offset, uint32_t length);
void btn_input_set(BtnInput *input, unsigned source, uint32_t mask);
void btn_input_clear(BtnInput *input);
void btn_input_end_tick(BtnInput *input);
unsigned btn_clock_advance(BtnClock *clock, uint64_t delta_ns, bool active);
/* Constant rate per clock, 1..1000 Hz. A rate change requires clearing phase. */
unsigned btn_clock_advance_hz(BtnClock *clock, uint64_t delta_ns, bool active, unsigned hz);
void btn_simulate(BtnSimulation *state, const BtnInput *input);

#endif
