/* Included by src/state_core.c; shares its private declarations. */
void pad_vibration_tick(void)
{
    StateCallback callback;
    u32 index = 0;
    StateCallback *callbacks = padVibrationHandlers;
    s16 *stateIndex = vibrationActive;

    do {
        callback = callbacks[*stateIndex];

        if (callback != 0) {
            callback(index);
        }
        index++;
        stateIndex++;
    } while (index < 2);
}
typedef struct {
    u8 priority;
    u8 pad01[3];
    s32 script;
} RumbleEntry;
extern RumbleEntry vibrationPatterns[];
extern u32 storage8006C844[];
extern s32 pad_is_started(void);
void pad_vibration_set_script(s32 index, s32 value);

void pad_vibration_start(s32 index, s32 value)
{
    if (storage8006C844[0] & 0x10000) {
        return;
    }
    if (pad_is_started() == 0) {
        return;
    }
    if (vibrationActive[index] != 0) {
        if (vibrationPriorities[index] > vibrationPatterns[value].priority) {
            return;
        }
        if (vibrationCursors[index] == vibrationPatterns[value].script) {
            return;
        }
    }
    pad_vibration_set_script(index, vibrationPatterns[value].script);
    vibrationActive[index] = 1;
    vibrationPriorities[index] = vibrationPatterns[value].priority;
}
extern void pad_vibration_clear(s32 index);
void pad_vibration_stop(s32 value)
{
    pad_vibration_clear(value);
}
/* Reusing the byte local for the later script read keeps the arithmetic shift. */
void pad_vibration_step(s32 index)
{
    StateData *pad = &inputRecords;
    s32 *cursor = &vibrationCursors[index];
    u8 *script = (u8 *)*cursor;
    s16 *timer;
    s32 value;

    if (script != 0) {
        timer = &vibrationRepeats[index];
        value = script[2];
        if ((value >> 1) < ++*timer) {
            script += 3;
            *timer = 0;
            *cursor += 3;
        }
        value = *script;
        if (value == 0xFF) {
            pad_vibration_clear(index);
            return;
        }
        pad->unk36 = 0;
        pad->unk37 = 0;
        if (*script & 1) {
            pad->unk36 = 1;
        }
        if (*script & 2) {
            pad->unk37 = script[1];
        }
    }
}
void pad_vibration_set_script(s32 index, s32 value)
{
    vibrationCursors[index] = value;
}
void pad_vibration_clear(s32 index)
{
    inputRecords.unk36 = 0;
    inputRecords.unk37 = 0;
    vibrationCursors[index] = 0;
    vibrationActive[index] = 0;
    vibrationPriorities[index] = 0;
    vibrationRepeats[index] = 0;
}
