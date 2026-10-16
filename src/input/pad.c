/* Included by src/system.c; shares its private declarations. */
void pad_init(void)
{
    pad_start_ports();
    ChangeClearPAD(0);
}

extern void pad_set_repeat(s32 arg0, s32 arg1);
extern void pad_set_repeat2(s32 arg0, s32 arg1);
extern void *memset(void *dst, s32 value, u32 size);
extern u8 padBuffers[][34];
extern u8 padActuators[][8];

void pad_reset_records(void)
{
    memset(inputPad1, 0, sizeof(SystemState));
    memset(inputPad2, 0, sizeof(SystemState));
    memset(padBuffers, 0, 0x44);
    memset(padActuators, 0, 0x10);
    pad_set_repeat(0x14, 7);
    pad_set_repeat2(0x14, 7);
}

void pad_set_repeat(s32 delay, s32 rate)
{
    SystemState *ac = inputPad2;
    SystemState *a8 = inputPad1;

    ac->unk18 = delay;
    a8->unk18 = delay;
    ac->unk1C = rate;
    a8->unk1C = rate;
}
void pad_set_repeat2(s32 delay, s32 rate)
{
    SystemState *ac = inputPad2;
    SystemState *a8 = inputPad1;

    ac->unk2C = delay;
    a8->unk2C = delay;
    ac->unk30 = rate;
    a8->unk30 = rate;
}

void pad_set_started(s32 value)
{
    padStarted = value;
}

s32 pad_is_started(void)
{
    return padStarted;
}
typedef struct {
    s8 port[2];
} PadPorts;

/* PROBABLE: the two pad port numbers, copied whole into a local. */
extern PadPorts padPorts[];
typedef struct {
    u8 pad00[0xC];
    u32 flags;
    u8 pad10[0xC];
    u32 *pad;
} PadWork;
extern PadWork gameState;
extern SystemState inputRecords[];
extern s16 padPortMode[2];
extern s32 padNeedsMode[2];
/* PadWork.flags (gameState + 0xC) as an incomplete array: the struct field
 * form ties the %hi temp to the loaded value and misallocates. */
extern s32 storage8006C844[];
extern int PadGetState(int port);
extern int PadInfoMode(int port, int term, int offs);
void pad_read_record(SystemState *s, u8 port);
void pad_clear_record(SystemState *state);
s32 pad_align_actuators(s32 index);

/* Pad connection poll. retry is re-armed at the top of each port (cc1
 * moves it into the back-branch delay slot); an armed port whose
 * pad_align_actuators succeeds re-polls without clearing it. */
void pad_poll_ports(void)
{
    PadPorts ports = padPorts[0];
    s32 i;
    s32 retry;

    for (i = 0; i < 2; i++) {
        retry = 1;
        do {
            switch (PadGetState(((u8 *)&ports)[i])) {
            case 0:
            case 1:
                if (storage8006C844[0] & 0x10000) {
                    goto other;
                }
                padPortMode[i] = 0;
                padNeedsMode[i] = 1;
                pad_clear_record(&inputRecords[i]);
                retry = 0;
                break;
            case 4:
            case 5:
                pad_clear_record(&inputRecords[i]);
                retry = 0;
                break;
            case 6:
                if (padNeedsMode[i] != 1) {
                    goto other;
                }
                padPortMode[i] = PadInfoMode(((u8 *)&ports)[i], 2, 0);
                if (pad_align_actuators(i) == 0) {
                    goto other;
                }
                padNeedsMode[i] = 0;
                pad_clear_record(&inputRecords[i]);
                break;
            default:
            other:
                pad_read_record(&inputRecords[i], i);
                retry = 0;
                break;
            }
        } while (retry == 1);
    }
}
extern u8 main_get_speed_mode(void);
void pad_set_vibration(u8 arg0, u8 arg1, s32 port);
s32 pad_read_buttons(s32 port);
s32 pad_read_analog(s32 port);

void pad_read_record(SystemState *s, u8 port)
{
    s32 div;
    s32 mode;
    u32 delay;
    s32 rate;
    u32 raw;
    s32 bits;
    PadWork *work;

    pad_set_vibration(s->unk36, s->unk37, port);
    s->unk04 = (u16)~pad_read_buttons(port);
    s->unk34 = pad_read_analog(port);
    work = &gameState;
    if ((work->flags & 0x10000) && s == inputPad1) {
        if (s->unk04 & 0x800) {
            *work->pad = 0x800;
        } else {
            raw = ((u16 *)work->pad)[2];
            bits = (raw >> 1) & 0x20;
            if (raw & 0x20) {
                bits |= 0x40;
            }
            raw &= ~0x60;
            raw |= bits;
            s->unk04 = raw;
            s->unk34 = 0;
        }
    }
    s->unk08 = (s->unk0C ^ s->unk04) & s->unk04;
    if (s->unk04 == s->unk0C) {
        mode = main_get_speed_mode();
        div = 1;
        if ((u16)(mode - 2) < 2) {
            div = mode;
        }
        delay = s->unk18 / div;
        rate = s->unk1C / div;
        if (rate <= 0) {
            rate = 1;
        }
        if (s->unk14 == 0) {
            s->unk00 = 0;
            s->unk10++;
            if (delay < s->unk10) {
                s->unk10 = 0;
                s->unk14 = 1;
            }
        } else {
            s->unk10++;
            if (s->unk10 % rate == 0) {
                s->unk00 = s->unk04;
            } else {
                s->unk00 = 0;
            }
        }
        delay = s->unk2C / div;
        rate = s->unk30 / div;
        if (rate <= 0) {
            rate = 1;
        }
        if (s->unk20 == 0) {
            s->unk24 = 0;
            s->unk28++;
            if (delay < s->unk28) {
                s->unk28 = 0;
                s->unk20 = 1;
            }
        } else {
            s->unk28++;
            if (s->unk28 % rate == 0) {
                s->unk24 = s->unk04;
            } else {
                s->unk24 = 0;
            }
        }
    } else {
        s->unk00 = s->unk08;
        s->unk24 = s->unk08;
        s->unk10 = 0;
        s->unk14 = 0;
        s->unk28 = 0;
        s->unk20 = 0;
    }
    s->unk0C = s->unk04;
}
void pad_clear_record(SystemState *state)
{
    state->unk04 = 0;
    state->unk08 = 0;
    state->unk34 = -1;
    state->unk0C = 0;
    state->unk10 = 0;
    state->unk14 = 0;
    state->unk28 = 0;
    state->unk20 = 0;
    state->unk00 = 0;
    state->unk24 = 0;
}
s32 pad_dpad_with_stick(s32 index);

s32 pad_read_buttons(s32 port)
{
    s32 type;

    if (padBuffers[port][0] != 0) {
        return 0xFF00FFFF;
    }
    type = padBuffers[port][1] >> 4;
    if (type != 4 && type != 7) {
        return 0xFF00FFFF;
    }
    return (pad_dpad_with_stick(port) << 8) | padBuffers[port][3];
}
extern void PadInitDirect(void *buf1, void *buf2);
extern void PadSetAct(u32 port, u8 *act, u32 len);
extern void PadStartCom(void);
extern u8 padBuffers[][34];
extern u8 padActuators[][8];
void pad_start_ports(void)
{
    PadInitDirect(padBuffers[0], padBuffers[1]);
    PadSetAct(0, padActuators, 8);
    PadSetAct(0x10, padActuators[1], 8);
    PadStartCom();
    padStarted = 1;
}
extern u8 padActuatorAlign[];
extern int PadGetState(int port);
extern int PadSetActAlign(int port, unsigned char *table);

s32 pad_align_actuators(s32 index)
{
    PadPorts ports = padPorts[0];
    u8 *port = (u8 *)&ports.port[index];
    s32 result = 0;

    if (PadGetState(*port) == 6) {
        result = PadSetActAlign(*port, padActuatorAlign);
    }
    return result;
}
s16 pad_stick_heading(u8 x, u8 y);

s32 pad_dpad_with_stick(s32 index)
{
    u8 *btn;
    s32 v;
    s32 type;
    s32 r;

    v = padBuffers[index][2];
    type = padBuffers[index][1] >> 4;
    btn = &padBuffers[index][2];

    if (type == 4) {
        return v;
    }
    if (type == 7) {
        if ((v & 0xF0) == 0xF0) {
            r = pad_stick_heading(padBuffers[index][6], padBuffers[index][7]);
            if (r == -1) {
                v = *btn;
            } else if ((u16)(r - 0x2D) >= 0x10F) {
                v = *btn & 0xEF;
            } else if ((u16)(r - 0x2E) < 0x59) {
                v = *btn & 0xDF;
            } else if ((u16)(r - 0x88) < 0x59) {
                v = *btn & 0xBF;
            } else if (r >= 0xE2) {
                v = *btn & 0x7F;
            }
        }
    }
    return v;
}
s32 pad_stick_tilt(u8 x, u8 y);

s32 pad_read_analog(s32 index)
{
    s32 type = padBuffers[index][1] >> 4;

    if (type != 4) {
        if (type != 7) {
            return -1;
        }
        if ((padBuffers[index][2] & 0xF0) != 0xF0) {
            return -1;
        }
        return pad_stick_tilt(padBuffers[index][6], padBuffers[index][7]);
    }
    return -1;
}
extern s32 csqrt(s32 value);
s32 pad_stick_tilt(u8 x0, u8 y)
{
    s32 x;
    s32 dx;
    s32 t;
    s32 v;

    x = x0;
    t = y;
    if (x >= 92 && x < 165 && t < 165 && t >= 92) {
        return 0;
    }
    x -= 128;
    t = y - 128;
    dx = x * x;
    t = t * t + dx;
    v = csqrt(t << 12);
    x = v / 4096;
    v = (u8)x;
    v -= 37;
    if (v < 0) {
        v = 0;
    }
    if (v > 90) {
        v = 90;
    }
    return (u8)v;
}
extern long ratan2(long y, long x);

s16 pad_stick_heading(u8 x, u8 y)
{
    s32 dx;
    s32 dy;
    s32 ang;
    s32 tx;
    s32 ty;

    if ((u32)(x - 0x5C) < 0x49 && y < 0xA5 && y >= 0x5C) {
        x = 0x80;
        y = 0x80;
    }
    if (x == 0x80 && y == 0x80) {
        return -1;
    }
    tx = x - 0x80;
    dx = tx < 0 ? -tx : tx;
    ty = y - 0x80;
    dy = ty < 0 ? -ty : ty;
    if (dy == 0) {
        ang = 0x5A;
    } else if (dx == 0) {
        ang = 0;
    } else {
        ang = ratan2(dx, dy) * 360 / 4096;
    }
    if ((s8)x < 0) {
        if (y > 0x80) {
            ang = 180 - ang;
        }
    } else {
        if (y > 0x80) {
            ang += 180;
        } else {
            ang = 360 - ang;
        }
    }
    if (ang == 360) {
        ang = 0;
    }
    return ang;
}
/* Ported from hmbtn-decomp (CC0), pad_set_vibration. */
void pad_set_vibration(u8 motor, u8 force, s32 port)
{
    if (padPortMode[port] == 0) {
        padActuators[port][0] = 0x40;
        padActuators[port][1] = motor != 0;
        if (force) {
            if (motor)
                motor = 0;
            padActuators[port][1] = 1;
        }
        padActuators[port][2] = 0;
    } else {
        padActuators[port][1] = motor != 0;
        padActuators[port][2] = force;
    }
    if (padStarted == 0) {
        memset(padActuators, 0, 0x10);
    }
}

extern s32 ResetCallback(void);

/* Correct-LIBCD first form generated 37 instructions versus retail 39 and 26 differences from
 * +0x30. Retail keeps file.size in $s0, rounds it, and shifts it in the CdControl delay slot; the
 * candidate used $a0 and formed different polling tails. This is a first uniterated form, not a
 * bounded failure. Preserve ASM. */
