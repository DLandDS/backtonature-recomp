#ifndef PSYQ_GTE_H
#define PSYQ_GTE_H

/* Minimal interface declarations compatible with PSY-Q 4.6 LIBGTE.H and the
 * INLINE_C.H GTE register macros, restricted to what this build uses.
 * `<FILE>.H:<line>` notes point to the equivalent definition in the user's own
 * SDK; SDK commentary is not reproduced. Same reason as include/psyq_libgpu.h:
 * the SDK headers are never put on the cpp path, because they drag in
 * prototypes for library routines this build resolves from the retail
 * executable instead.
 *
 * Why this exists: eleven functions still in assembly carry cop2 traffic --
 * 1,859 instructions, including model_draw_tmd_object at 936, the largest remaining
 * function in the project. Retail reaches the GTE through these macros, and
 * nothing in that family can match while the transfers are hand-written.
 * `lwc2` and `swc2` are the gte_ld and gte_st families, `cfc2` and `ctc2` are
 * the control register accessors, and the two `nop`s before an operation are
 * part of gte_rtps itself rather than scheduling slack.
 *
 * These are __asm__ volatile in the SDK, not C expressions. That is the SDK
 * own spelling, so a body built from them is real source and not the
 * one-__asm__-per-retail-instruction case that section 8 forbids -- the same
 * standing this project already grants to the LIBGPU primitive macros.
 *
 * Note include/gte_macros.inc is the GAS-side macro file used by the assembly
 * build. It is unrelated to this header and neither replaces the other.
 */

/* LIBGTE.H:137 */
typedef struct {
    short m[3][3];
    long t[3];
} MATRIX;

/* LIBGTE.H:143 */
typedef struct {
    long vx, vy;
    long vz, pad;
} VECTOR;

/* LIBGTE.H:148 */
typedef struct {
    short vx, vy;
    short vz, pad;
} SVECTOR;

/* LIBGTE.H:153 */
typedef struct {
    u8 r, g, b, cd;
} CVECTOR;

/* LIBGTE.H:157 */
typedef struct {
    short vx, vy;
} DVECTOR;

/*
 * Vertex loads
 */

/* INLINE_C.H:16 */
#define gte_ldv0(r0)                                                                               \
    __asm__ volatile("lwc2	$0, 0( %0 );"                                                           \
                     "lwc2	$1, 4( %0 )"                                                            \
                     :                                                                             \
                     : "r"(r0))

/* INLINE_C.H:22 */
#define gte_ldv1(r0)                                                                               \
    __asm__ volatile("lwc2	$2, 0( %0 );"                                                           \
                     "lwc2	$3, 4( %0 )"                                                            \
                     :                                                                             \
                     : "r"(r0))

/* INLINE_C.H:28 */
#define gte_ldv2(r0)                                                                               \
    __asm__ volatile("lwc2	$4, 0( %0 );"                                                           \
                     "lwc2	$5, 4( %0 )"                                                            \
                     :                                                                             \
                     : "r"(r0))

/* INLINE_C.H:34 -- loads all three vertices in ONE asm statement.
 *
 * Not interchangeable with three separate gte_ldv0/1/2 calls even though the
 * emitted lwc2 sequence is identical: three statements let cc1 allocate each
 * pointer independently, one statement makes the three operands live together
 * and allocates them as a group. model_draw_quad_depth and model_draw_quad_ot came down
 * from nine differences to zero on that distinction alone -- retail holds the
 * pointers in $a1/$a0/$v1, three separate macros produced $v0/$v1/$a0. */
#define gte_ldv3(r0, r1, r2)                                                                       \
    __asm__ volatile("lwc2	$0, 0( %0 );"                                                           \
                     "lwc2	$1, 4( %0 );"                                                           \
                     "lwc2	$2, 0( %1 );"                                                           \
                     "lwc2	$3, 4( %1 );"                                                           \
                     "lwc2	$4, 0( %2 );"                                                           \
                     "lwc2	$5, 4( %2 )"                                                            \
                     :                                                                             \
                     : "r"(r0), "r"(r1), "r"(r2))

/* INLINE_C.H:101 */
#define gte_ldlv0(r0)                                                                              \
    __asm__ volatile("lhu	$13, 4( %0 );"                                                           \
                     "lhu	$12, 0( %0 );"                                                           \
                     "sll	$13, $13, 16;"                                                           \
                     "or	$12, $12, $13;"                                                           \
                     "mtc2	$12, $0;"                                                               \
                     "lwc2	$1, 8( %0 )"                                                            \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13")

/* INLINE_C.H:150 */
#define gte_ldclmv(r0)                                                                             \
    __asm__ volatile("lhu	$12, 0( %0 );"                                                           \
                     "lhu	$13, 6( %0 );"                                                           \
                     "lhu	$14, 12( %0 );"                                                          \
                     "mtc2	$12, $9;"                                                               \
                     "mtc2	$13, $10;"                                                              \
                     "mtc2	$14, $11"                                                               \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14")

/*
 * Control register loads -- matrix and colour setup
 */

/* INLINE_C.H:245 */
#define gte_SetBackColor(r0, r1, r2)                                                               \
    __asm__ volatile("sll	$12, %0, 4;"                                                             \
                     "sll	$13, %1, 4;"                                                             \
                     "sll	$14, %2, 4;"                                                             \
                     "ctc2	$12, $13;"                                                              \
                     "ctc2	$13, $14;"                                                              \
                     "ctc2	$14, $15"                                                               \
                     :                                                                             \
                     : "r"(r0), "r"(r1), "r"(r2)                                                   \
                     : "$12", "$13", "$14")

/* INLINE_C.H:297 */
#define gte_SetRotMatrix(r0)                                                                       \
    __asm__ volatile("lw	$12, 0( %0 );"                                                            \
                     "lw	$13, 4( %0 );"                                                            \
                     "ctc2	$12, $0;"                                                               \
                     "ctc2	$13, $1;"                                                               \
                     "lw	$12, 8( %0 );"                                                            \
                     "lw	$13, 12( %0 );"                                                           \
                     "lw	$14, 16( %0 );"                                                           \
                     "ctc2	$12, $2;"                                                               \
                     "ctc2	$13, $3;"                                                               \
                     "ctc2	$14, $4"                                                                \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14")

/* INLINE_C.H:321 */
#define gte_SetLightMatrix(r0)                                                                     \
    __asm__ volatile("lw	$12, 0( %0 );"                                                            \
                     "lw	$13, 4( %0 );"                                                            \
                     "ctc2	$12, $8;"                                                               \
                     "ctc2	$13, $9;"                                                               \
                     "lw	$12, 8( %0 );"                                                            \
                     "lw	$13, 12( %0 );"                                                           \
                     "lw	$14, 16( %0 );"                                                           \
                     "ctc2	$12, $10;"                                                              \
                     "ctc2	$13, $11;"                                                              \
                     "ctc2	$14, $12"                                                               \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14")

/* INLINE_C.H:345 */
#define gte_SetColorMatrix(r0)                                                                     \
    __asm__ volatile("lw	$12, 0( %0 );"                                                            \
                     "lw	$13, 4( %0 );"                                                            \
                     "ctc2	$12, $16;"                                                              \
                     "ctc2	$13, $17;"                                                              \
                     "lw	$12, 8( %0 );"                                                            \
                     "lw	$13, 12( %0 );"                                                           \
                     "lw	$14, 16( %0 );"                                                           \
                     "ctc2	$12, $18;"                                                              \
                     "ctc2	$13, $19;"                                                              \
                     "ctc2	$14, $20"                                                               \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14")

/* INLINE_C.H:360 */
#define gte_SetTransMatrix(r0)                                                                     \
    __asm__ volatile("lw	$12, 20( %0 );"                                                           \
                     "lw	$13, 24( %0 );"                                                           \
                     "ctc2	$12, $5;"                                                               \
                     "lw	$14, 28( %0 );"                                                           \
                     "ctc2	$13, $6;"                                                               \
                     "ctc2	$14, $7"                                                                \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14")

/*
 * Operations.  The two leading nops are part of the macro, not slack.
 */

/* INLINE_C.H:484, with one deliberate deviation.
 *
 * The SDK spells the operation itself as `.word 0x0000007f`. That constant is
 * a DMPSX marker, not an instruction: the PSY-Q assembler rewrites it into the
 * real cop2 word. This build assembles with GNU as, which does not, so the
 * literal SDK text emits 0x0000007f where retail has 0x4A180001 -- one
 * difference, in the operation itself, with every surrounding instruction
 * already exact.
 *
 * The mnemonic below resolves through include/gte_macros.inc, the GAS-side
 * macro file this repository already carries for the assembly build, which
 * encodes it as cop2op 0x01, 0x01. The two leading nops stay verbatim: they
 * are part of the SDK macro, not scheduling slack.
 *
 * Same class of adjustment as the documented maspsx rewrites -- the shape is
 * the SDK's, the encoding is this toolchain's.
 */
#define gte_rtps()                                                                                 \
    __asm__ volatile("nop;"                                                                        \
                     "nop;"                                                                        \
                     "rtps")

/* INLINE_C.H:489, same deviation -- SDK spells this `.word 0x000000bf`. */
#define gte_rtpt()                                                                                 \
    __asm__ volatile("nop;"                                                                        \
                     "nop;"                                                                        \
                     "rtpt")

/* INLINE_C.H:764, same deviation -- SDK spells this `.word 0x0000117f`. */
#define gte_nclip()                                                                                \
    __asm__ volatile("nop;"                                                                        \
                     "nop;"                                                                        \
                     "nclip")

/* INLINE_C.H:774, same deviation -- SDK spells this `.word 0x000011ff`. */
#define gte_avsz4()                                                                                \
    __asm__ volatile("nop;"                                                                        \
                     "nop;"                                                                        \
                     "avsz4")

/* INLINE_C.H:769, same deviation -- SDK spells this `.word 0x000011bf`. */
#define gte_avsz3()                                                                                \
    __asm__ volatile("nop;"                                                                        \
                     "nop;"                                                                        \
                     "avsz3")

/*
 * Type 2 functions without nop -- INLINE_C.H:876 onward.
 *
 * The SDK carries a `_b` twin of every operation that drops the two leading
 * nops. They are not an optimization to reach for: the nops cover the GTE
 * pipeline delay, and the `_b` form is correct only where enough unrelated
 * instructions already sit between the previous operation and this one.
 *
 * Read the retail gap and pick the variant it implies. Round 12 found both
 * spellings inside the same two functions -- `nclip` with no nops because a
 * dozen instructions already separated it from the preceding `rtpt`, and
 * `avsz4` with its nops intact immediately after a `swc2`. Do not assume a
 * function uses one variant throughout.
 */

/* INLINE_C.H:876 */
#define gte_rtps_b() __asm__ volatile("rtps")

/* INLINE_C.H:877 */
#define gte_rtpt_b() __asm__ volatile("rtpt")

/* INLINE_C.H:880 */
#define gte_nclip_b() __asm__ volatile("nclip")

/* INLINE_C.H:882 */
#define gte_avsz4_b() __asm__ volatile("avsz4")

/* INLINE_C.H:881 */
#define gte_avsz3_b() __asm__ volatile("avsz3")

/* INLINE_C.H:809 and :816.
 *
 * The SDK builds the operation word arithmetically and emits it through a
 * `.word %0` with a "g" constraint, because DMPSX has no mnemonic for the
 * parameterized form. include/gte_macros.inc does, taking the same five
 * fields, so the parameters are stringified into that GAS macro instead.
 * Same deviation as gte_rtps, and for the same reason.
 */
#define gte_mvmva(sf, mx, v, cv, lm)                                                               \
    __asm__ volatile("nop;"                                                                        \
                     "nop;"                                                                        \
                     "mvmva " #sf ", " #mx ", " #v ", " #cv ", " #lm)

#define gte_mvmva_b(sf, mx, v, cv, lm)                                                             \
    __asm__ volatile("mvmva " #sf ", " #mx ", " #v ", " #cv ", " #lm)

/*
 * Result stores
 */

/* INLINE_C.H:900 */
#define gte_stsxy(r0) __asm__ volatile("swc2	$14, 0( %0 )" : : "r"(r0) : "memory")

/* INLINE_C.H:906 */
#define gte_stsxy3(r0, r1, r2)                                                                     \
    __asm__ volatile("swc2	$12, 0( %0 );"                                                          \
                     "swc2	$13, 0( %1 );"                                                          \
                     "swc2	$14, 0( %2 )"                                                           \
                     :                                                                             \
                     : "r"(r0), "r"(r1), "r"(r2)                                                   \
                     : "memory")

/* INLINE_C.H:954 onward -- the per-primitive stsxy3 family.
 *
 * These write the three screen coordinates through ONE base pointer at the
 * field offsets of a specific primitive, instead of taking three separate
 * addresses the way gte_stsxy3 does. Retail using one of these is visible
 * directly in the disassembly: three swc2 sharing a base register with
 * constant offsets, and no addiu computing intermediate addresses.
 *
 * Several of them are byte-identical (8/16/24 covers _g3, _ft3, _g4, _ft4,
 * _gt4); pick the one matching the primitive the function is actually
 * building, which the packet code byte tells you.
 */

/* INLINE_C.H:954 */
#define gte_stsxy3_f3(r0)                                                                          \
    __asm__ volatile("swc2	$12, 8( %0 );"                                                          \
                     "swc2	$13, 12( %0 );"                                                         \
                     "swc2	$14, 16( %0 )"                                                          \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "memory")

/* INLINE_C.H:962 */
#define gte_stsxy3_g3(r0)                                                                          \
    __asm__ volatile("swc2	$12, 8( %0 );"                                                          \
                     "swc2	$13, 16( %0 );"                                                         \
                     "swc2	$14, 24( %0 )"                                                          \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "memory")

/* INLINE_C.H:970 */
#define gte_stsxy3_ft3(r0)                                                                         \
    __asm__ volatile("swc2	$12, 8( %0 );"                                                          \
                     "swc2	$13, 16( %0 );"                                                         \
                     "swc2	$14, 24( %0 )"                                                          \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "memory")

/* INLINE_C.H:978 */
#define gte_stsxy3_gt3(r0)                                                                         \
    __asm__ volatile("swc2	$12, 8( %0 );"                                                          \
                     "swc2	$13, 20( %0 );"                                                         \
                     "swc2	$14, 32( %0 )"                                                          \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "memory")

/* INLINE_C.H:986 */
#define gte_stsxy3_f4(r0)                                                                          \
    __asm__ volatile("swc2	$12, 8( %0 );"                                                          \
                     "swc2	$13, 12( %0 );"                                                         \
                     "swc2	$14, 16( %0 )"                                                          \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "memory")

/* INLINE_C.H:994 */
#define gte_stsxy3_g4(r0)                                                                          \
    __asm__ volatile("swc2	$12, 8( %0 );"                                                          \
                     "swc2	$13, 16( %0 );"                                                         \
                     "swc2	$14, 24( %0 )"                                                          \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "memory")

/* INLINE_C.H:1002 */
#define gte_stsxy3_ft4(r0)                                                                         \
    __asm__ volatile("swc2	$12, 8( %0 );"                                                          \
                     "swc2	$13, 16( %0 );"                                                         \
                     "swc2	$14, 24( %0 )"                                                          \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "memory")

/* INLINE_C.H:1024 */
#define gte_stflg(r0)                                                                              \
    __asm__ volatile("cfc2	$12, $31;"                                                              \
                     "nop;"                                                                        \
                     "sw	$12, 0( %0 )"                                                             \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "memory")

/* INLINE_C.H:1082 */
#define gte_stszotz(r0)                                                                            \
    __asm__ volatile("mfc2	$12, $19;"                                                              \
                     "nop;"                                                                        \
                     "sra	$12, $12, 2;"                                                            \
                     "sw	$12, 0( %0 )"                                                             \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "memory")

/* INLINE_C.H:1091 */
#define gte_stotz(r0) __asm__ volatile("swc2	$7, 0( %0 )" : : "r"(r0) : "memory")

/* INLINE_C.H:1097 */
#define gte_stopz(r0) __asm__ volatile("swc2	$24, 0( %0 )" : : "r"(r0) : "memory")

/* INLINE_C.H:1103 */
#define gte_stlvl(r0)                                                                              \
    __asm__ volatile("swc2	$9, 0( %0 );"                                                           \
                     "swc2	$10, 4( %0 );"                                                          \
                     "swc2	$11, 8( %0 )"                                                           \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "memory")

/* INLINE_C.H:1148 */
#define gte_stclmv(r0)                                                                             \
    __asm__ volatile("mfc2	$12, $9;"                                                               \
                     "mfc2	$13, $10;"                                                              \
                     "mfc2	$14, $11;"                                                              \
                     "sh	$12, 0( %0 );"                                                            \
                     "sh	$13, 6( %0 );"                                                            \
                     "sh	$14, 12( %0 )"                                                            \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14", "memory")

/* INLINE_C.H:1244 -- reads the whole rotation matrix and translation vector
 * back out of the control registers in one macro. This is what a run of
 * eight cfc2 against $0..$7 with interleaved sw means; model_save_gte_rot is
 * exactly one call to it, which is why the disassembler flagged that
 * function as handwritten. */
#define gte_ReadRotMatrix(r0)                                                                      \
    __asm__ volatile("cfc2	$12, $0;"                                                               \
                     "cfc2	$13, $1;"                                                               \
                     "sw	$12, 0( %0 );"                                                            \
                     "sw	$13, 4( %0 );"                                                            \
                     "cfc2	$12, $2;"                                                               \
                     "cfc2	$13, $3;"                                                               \
                     "cfc2	$14, $4;"                                                               \
                     "sw	$12, 8( %0 );"                                                            \
                     "sw	$13, 12( %0 );"                                                           \
                     "sw	$14, 16( %0 );"                                                           \
                     "cfc2	$12, $5;"                                                               \
                     "cfc2	$13, $6;"                                                               \
                     "cfc2	$14, $7;"                                                               \
                     "sw	$12, 20( %0 );"                                                           \
                     "sw	$13, 24( %0 );"                                                           \
                     "sw	$14, 28( %0 )"                                                            \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14", "memory")

/*
 * Outer product and matrix-multiply composites (INLINE_C.H gte_ldopv1,
 * gte_ldopv2, gte_op12, gte_stlvnl, gte_OuterProduct12, gte_rtir and
 * gte_MulMatrix0). camera_build_view uses both composites.
 */
#define gte_ldopv1(r0)                                                                             \
    __asm__ volatile("lw	$12, 0( %0 );"                                                            \
                     "lw	$13, 4( %0 );"                                                            \
                     "ctc2	$12, $0;"                                                               \
                     "lw	$14, 8( %0 );"                                                            \
                     "ctc2	$13, $2;"                                                               \
                     "ctc2	$14, $4"                                                                \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "$12", "$13", "$14")

#define gte_ldopv2(r0)                                                                             \
    __asm__ volatile("lwc2	$11, 8( %0 );"                                                          \
                     "lwc2	$9, 0( %0 );"                                                           \
                     "lwc2	$10, 4( %0 )"                                                           \
                     :                                                                             \
                     : "r"(r0))

#define gte_op12()                                                                                 \
    __asm__ volatile("nop;"                                                                        \
                     "nop;"                                                                        \
                     "op 1")

#define gte_stlvnl(r0)                                                                             \
    __asm__ volatile("swc2	$25, 0( %0 );"                                                          \
                     "swc2	$26, 4( %0 );"                                                          \
                     "swc2	$27, 8( %0 )"                                                           \
                     :                                                                             \
                     : "r"(r0)                                                                     \
                     : "memory")

#define gte_OuterProduct12(r0, r1, r2)                                                             \
    {                                                                                              \
        gte_ldopv1(r0);                                                                            \
        gte_ldopv2(r1);                                                                            \
        gte_op12();                                                                                \
        gte_stlvnl(r2);                                                                            \
    }

#define gte_rtir() gte_mvmva(1, 0, 3, 3, 0)

#define gte_MulMatrix0(r1, r2, r3)                                                                 \
    {                                                                                              \
        gte_SetRotMatrix(r1);                                                                      \
        gte_ldclmv(r2);                                                                            \
        gte_rtir();                                                                                \
        gte_stclmv(r3);                                                                            \
        gte_ldclmv((char *)(r2) + 2);                                                              \
        gte_rtir();                                                                                \
        gte_stclmv((char *)(r3) + 2);                                                              \
        gte_ldclmv((char *)(r2) + 4);                                                              \
        gte_rtir();                                                                                \
        gte_stclmv((char *)(r3) + 4);                                                              \
    }

#endif /* PSYQ_GTE_H */
