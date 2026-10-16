/* Included by src/ui.c; shares its private declarations. */
/* Keep the loop 1 dst value live so combine leaves the three loop 2 GIV
 * initializers separate and loop.c emits their retail increment order. */
void CopyMainUiTextTableString(u16 *dst)
{
    u16 *start;
    u16 *src;
    u16 *s;
    u8 *work;
    s32 i;
    s32 j;

    s = (u16 *)((u8 *)storage8005392E + mainUiTextTableRuntimeOffset[0]);
    do {
        i = 0;
        if (s[0] != 0xFFFF) {
        loop:
            *dst++ = s[i++];
            if (s[i] != 0xFFFF) {
                goto loop;
            }
        }
        dst++;
    } while (0);
    __asm__("" : : "r"(dst));
    start = dst;
    j = 0;
    src = (u16 *)nameBuffer;
    do {
        if (src[j] == 0xFFFF) {
            work = gameState;
            dst[j] = (*(u32 *)(work + 8) % 15 != 0) ? 0xFFD : 0x75;
            break;
        }
        start[j] = src[j];
    } while (++j < 8);
}
extern u16 storage800537B2[];
extern u16 storage80053888[];
void text_set_spacing(s16 arg0, s16 arg1);
void DrawTextWrapper(void *display, const u16 *stream, s16 x, s16 y);
