/* Included by src/gameplay.c; shares its private declarations. */
void vec_from_polar(VECTOR *out, s32 radius, s32 y, s32 arg3)
{
    s32 angle = arg3 & 0xFFF;

    out->vy = -y;
    out->vx = (rcos(angle) * radius) >> 12;
    out->vz = (rsin(angle) * radius) >> 12;
}
s32 math_int_pow(s32 base, s32 exponent)
{
    s32 result = 1;

    if (exponent > 0) {
        do {
            exponent--;
            result *= base;
        } while (exponent != 0);
    }
    return result;
}
void math_lerp_vec3(s32 step, s32 steps, s32 *from, s32 *to, s32 *arg4)
{
    s32 i;
    s32 *out;

    if (steps < step) {
        step = steps;
    }
    step = steps - step;
    out = arg4;
    i = 2;
    do {
        *out = (*from - *to) * step / steps + *to;
        out++;
        i--;
        from++;
        to++;
    } while (i >= 0);
}
void math_lerp_scalar(s32 step, s32 steps, s32 from, s32 to, s32 *out)
{
    s32 delta;
    s32 result;

    if (steps < step) {
        step = steps;
    }
    delta = steps - step;
    result = (from - to) * delta / steps + to;
    *out = result;
}
void vec3_copy(s32 *dst, s32 *src)
{
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
}
void vec3_add(s32 *dst, s32 *src)
{
    dst[0] += src[0];
    dst[1] += src[1];
    dst[2] += src[2];
}
void vec3_sub(s32 *dst, s32 *src)
{
    dst[0] -= src[0];
    dst[1] -= src[1];
    dst[2] -= src[2];
}
s32 vec3_differs(s32 *a, s32 *b)
{
    if (a[0] != b[0]) {
        return 1;
    }
    if (a[1] != b[1]) {
        return 1;
    }
    return a[2] != b[2];
}
extern s32 SquareRoot0(s32 a);

s32 vec3_move_toward(s32 *from, s32 dist, s32 *to, s32 *out)
{
    s32 d[3];
    s32 len;
    s32 result = 1;

    vec3_copy(d, from);
    vec3_sub(d, to);
    len = SquareRoot0(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
    if (out != 0) {
        if (len == 0) {
            vec3_copy(out, to);
        } else {
            out[0] = to[0] + d[0] * dist / len;
            out[1] = to[1] + d[1] * dist / len;
            out[2] = to[2] + d[2] * dist / len;
        }
    }
    if (len >= dist) {
        result = 0;
    }
    return result;
}
u32 math_cross2_sign(s32 x0, s32 y0, s32 x1, s32 y1)
{
    s32 value = x0 * y1 - y0 * x1;
    u32 result = ~(u32)value >> 31;

    if (value == 0) {
        result |= 2;
    }
    return result;
}
s32 vec_dot_normalized(VECTOR *a, VECTOR *b)
{
    VECTOR normal0;
    VECTOR normal1;

    VectorNormal(a, &normal0);
    VectorNormal(b, &normal1);
    return (normal0.vx * normal1.vx) + (normal0.vy * normal1.vy) + (normal0.vz * normal1.vz);
}
typedef struct ClutSave {
    u32 *buf;
    s16 unk04;
    s16 unk06;
    s16 unk08;
    s16 unk0A;
    s16 unk0C;
    s16 unk0E;
    s16 unk10;
    s16 unk12;
} ClutSave;
s32 clut_fade_page(void);
extern int DrawSync(int mode);
extern int StoreImage(RECT *rect, u32 *p);
