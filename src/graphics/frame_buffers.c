/* Included by src/gameplay.c; shares its private declarations. */
/* Returning the original primitive pointer keeps it live through the ordering-table writes. */
void *ot_insert_prim(s32 depth, s32 size)
{
    OtContext *ctx = currentFrame;
    u32 *ot;
    u32 *prim;

    debug_hook_run(0x80);
    if (depth >= ctx->otLen) {
        depth = ctx->otLen - 1;
    }
    ot = &ctx->ot[depth];
    prim = ctx->prim;
    *prim = (*prim & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (u32)prim & 0xFFFFFF;
    ctx->prim = (u32 *)((u8 *)ctx->prim + size);
    return prim;
}
void debug_hook_run(s32 unused0) {}
extern u8 *regionBase; /* nongp */
extern void ClearOTagR(void *ot, s32 n);

void frame_buffers_layout(void)
{
    s32 i;
    s32 j;
    GfxCtx *ctx;
    u8 *base;

    base = regionBase;
    for (i = 0; i < 2; i++) {
        ctx = &(&frameRecords)[i];
        ctx->unk110 = 0x800;
        ctx->unk114 = 0xB;
        ctx->unk11C = 0x17700;
        ctx->unk120 = base + (i * 0x2000 + 0x290);
        ctx->unk124 = base + (i * 0x17700 + 0x4290);
        ClearOTagR(ctx->unk120, 0x800);
        for (j = 0; j < 0x17700; j++) {
            ctx->unk124[j] = 0;
        }
        ctx->unk118 = ctx->unk124;
    }
}
/* Ported from hmbtn-decomp (CC0), vec_length_bounded. */
#ifndef gte_ldlvl
#define gte_ldlvl(r0)                                                                              \
    __asm__ volatile("lwc2 $9, 0(%0);"                                                             \
                     "lwc2 $10, 4(%0);"                                                            \
                     "lwc2 $11, 8(%0)"                                                             \
                     :                                                                             \
                     : "r"(r0))
#endif
#ifndef gte_sqr0_b
#define gte_sqr0_b() __asm__ volatile(".word 0x4AA00428")
#endif
extern s32 SquareRoot0(s32 a);
