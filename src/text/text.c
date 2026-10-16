/* Included by src/archive.c; shares its private declarations. */
void text_reset(void)
{
    text_window_clear();
}
extern u16 *textSources[];

void text_set_source(u32 arg0, u16 *source)
{
    u32 index;

    do {
        if ((arg0 & 0xFFFF) >= 11) {
            arg0 = 10;
        }
        index = arg0 & 0xFFFF;
    } while (0);
    textSources[index] = source;
}
