#ifndef PSYQ_LIBGPU_H
#define PSYQ_LIBGPU_H

/* Minimal interface declarations compatible with the PSY-Q 4.6 GPU library:
 * primitive layouts and the primitive-building macros, restricted to what this
 * build uses. `LIBGPU.H:<line>` notes point to the equivalent definition in
 * the user's own SDK; SDK commentary is not reproduced. The SDK header itself
 * is never put on the cpp path because it drags in prototypes for library
 * routines this build resolves from the retail executable instead.
 *
 * Why this exists: three rounds of archive.c failures were read as size or
 * scheduling problems, and they are neither. Retail builds GPU primitives
 * through these macros. `setlen`/`setcode` are the `sb 0x3`/`sb 0x7` pair,
 * `setSemiTrans` is the 0x40/0x42 and 0x60/0x62 branch, and `addPrim` is the
 * 24-bit bitfield read-modify-write that shows up as the 0x00FFFFFF and
 * 0xFF000000 mask pair. Fifteen functions still in assembly carry that mask
 * pair; nothing in that family can match while the layout is hand-written.
 *
 * The bitfield order matters and is not negotiable: `addr:24` before `len:8`
 * is what puts the length in the top byte of the tag word.
 */

/* LIBGPU.H:392 */
typedef struct {
    unsigned addr : 24;
    unsigned len : 8;
    u8 r0, g0, b0, code;
} P_TAG;

/* LIBGPU.H:496 */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
} LINE_F2;

/* LIBGPU.H:409 */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s16 x3, y3;
} POLY_F4;

/* LIBGPU.H:452 */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} POLY_G4;

/* LIBGPU.H:589 */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
} TILE_1;

/* LIBGPU.H:582 */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TILE;

/* LIBGPU.H:428 */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad2;
} POLY_FT4;

/* LIBGPU.H:557 */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} SPRT;

/* LIBGPU.H:350 */
typedef struct {
    s16 x, y;
    s16 w, h;
} RECT;

/* LIBGPU.H:610 -- Note this is 12 bytes and carries len 2,
   where DR_TPAGE is 8 bytes and carries len 1; the retail `sb` of 1 or 2 at
   +0x3 is what tells the two apart, not the 0xE1xxxxxx word, which both share. */
typedef struct {
    u32 tag;
    u32 code[2];
} DR_MODE;

/* LIBGPU.H:641 */
typedef struct {
    u32 tag;
    u32 code[1];
} DR_TPAGE;

/* LIBGPU.H:119 */
#define setRGB0(p, _r0, _g0, _b0) (p)->r0 = _r0, (p)->g0 = _g0, (p)->b0 = _b0

/* LIBGPU.H:122 */
#define setRGB1(p, _r1, _g1, _b1) (p)->r1 = _r1, (p)->g1 = _g1, (p)->b1 = _b1

/* LIBGPU.H:125 */
#define setRGB2(p, _r2, _g2, _b2) (p)->r2 = _r2, (p)->g2 = _g2, (p)->b2 = _b2

/* LIBGPU.H:128 */
#define setRGB3(p, _r3, _g3, _b3) (p)->r3 = _r3, (p)->g3 = _g3, (p)->b3 = _b3

/* LIBGPU.H:134 */
#define setXY0(p, _x0, _y0) (p)->x0 = (_x0), (p)->y0 = (_y0)

/* LIBGPU.H:137 */
#define setXY2(p, _x0, _y0, _x1, _y1)                                                              \
    (p)->x0 = (_x0), (p)->y0 = (_y0), (p)->x1 = (_x1), (p)->y1 = (_y1)

/* LIBGPU.H:146 */
#define setXY4(p, _x0, _y0, _x1, _y1, _x2, _y2, _x3, _y3)                                          \
    (p)->x0 = (_x0), (p)->y0 = (_y0), (p)->x1 = (_x1), (p)->y1 = (_y1), (p)->x2 = (_x2),           \
    (p)->y2 = (_y2), (p)->x3 = (_x3), (p)->y3 = (_y3)

/* LIBGPU.H:161 */
#define setWH(p, _w, _h) (p)->w = _w, (p)->h = _h

/* LIBGPU.H:236 */
#define setlen(p, _len)   (((P_TAG *)(p))->len = (u8)(_len))
#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (u32)(_addr))
#define setcode(p, _code) (((P_TAG *)(p))->code = (u8)(_code))

/* LIBGPU.H:241 */
#define getcode(p) (u8)(((P_TAG *)(p))->code)
#define getaddr(p) (u32)(((P_TAG *)(p))->addr)

/* LIBGPU.H:247 */
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)

/* LIBGPU.H:253 */
#define setSemiTrans(p, abe)                                                                       \
    ((abe) ? setcode(p, getcode(p) | 0x02) : setcode(p, getcode(p) & ~0x02))

/* LIBGPU.H:337 */
#define setTile(p) setlen(p, 3), setcode(p, 0x60)
/* LIBGPU.H:325 */
#define setPolyF4(p) setlen(p, 5), setcode(p, 0x28)
/* LIBGPU.H:327 */
#define setPolyG4(p) setlen(p, 8), setcode(p, 0x38)
/* LIBGPU.H:334 */
#define setTile1(p) setlen(p, 2), setcode(p, 0x68)
/* LIBGPU.H:338 */
#define setLineF2(p) setlen(p, 3), setcode(p, 0x40)

/* LIBGPU.H:110 */
#define setTPage(p, tp, abr, x, y) ((p)->tpage = getTPage(tp, abr, x, y))

/* LIBGPU.H:113 */
#define setClut(p, x, y) ((p)->clut = getClut(x, y))

/* LIBGPU.H:166 */
#define setUV0(p, _u0, _v0) (p)->u0 = (_u0), (p)->v0 = (_v0)

/* LIBGPU.H:174 */
#define setUV4(p, _u0, _v0, _u1, _v1, _u2, _v2, _u3, _v3)                                          \
    (p)->u0 = (_u0), (p)->v0 = (_v0), (p)->u1 = (_u1), (p)->v1 = (_v1), (p)->u2 = (_u2),           \
    (p)->v2 = (_v2), (p)->u3 = (_u3), (p)->v3 = (_v3)

/* LIBGPU.H:180 */
#define setUVWH(p, _u0, _v0, _w, _h)                                                               \
    (p)->u0 = (_u0), (p)->v0 = (_v0), (p)->u1 = (_u0) + (_w), (p)->v1 = (_v0), (p)->u2 = (_u0),    \
    (p)->v2 = (_v0) + (_h), (p)->u3 = (_u0) + (_w), (p)->v3 = (_v0) + (_h)

/* LIBGPU.H:259 */
#define getTPage(tp, abr, x, y)                                                                    \
    ((((tp) & 0x3) << 7) | (((abr) & 0x3) << 5) | (((y) & 0x100) >> 4) | (((x) & 0x3ff) >> 6) |    \
     (((y) & 0x200) << 2))

/* LIBGPU.H:263 */
#define getClut(x, y) (((y) << 6) | (((x) >> 4) & 0x3f))

/* LIBGPU.H:275 */
#define _get_mode(dfe, dtd, tpage)                                                                 \
    ((0xe1000000) | ((dtd) ? 0x0200 : 0) | ((dfe) ? 0x0400 : 0) | ((tpage) & 0x9ff))

/* LIBGPU.H:279 */
#define setDrawTPage(p, dfe, dtd, tpage) setlen(p, 1), ((u32 *)(p))[1] = _get_mode(dfe, dtd, tpage)

/* LIBGPU.H:326 */
#define setPolyFT4(p) setlen(p, 9), setcode(p, 0x2c)
/* LIBGPU.H:332 */
#define setSprt(p) setlen(p, 4), setcode(p, 0x64)

/* LIBGPU.H:283 */
#define _get_tw(tw)                                                                                \
    (tw ? ((0xe2000000) | (((((tw)->y & 0xff) >> 3) << 15)) | ((((tw)->x & 0xff) >> 3) << 10) |    \
           ((((~((tw)->h - 1)) & 0xff) >> 3) << 5) | ((((~((tw)->w - 1)) & 0xff) >> 3)))           \
        : 0)

/* LIBGPU.H:312 */
#define setDrawMode(p, dfe, dtd, tpage, tw)                                                        \
    setlen(p, 2), ((u32 *)p)[1] = _get_mode(dfe, dtd, tpage), ((u32 *)p)[2] = _get_tw((RECT *)tw)

u16 GetTPage(int tp, int abr, int x, int y);
u16 GetClut(int x, int y);

#endif /* PSYQ_LIBGPU_H */
