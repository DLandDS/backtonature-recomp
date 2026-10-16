/* Included by src/archive.c; shares its private declarations. */
s32 image_bank_upload(u8 *entry, void *dest, u16 size, s16 x, s16 y)
{
    u8 *out;
    u8 *data;
    void *buf = dest;
    RECT rect;
    u16 n;
    u16 i;
    u32 word;
    u32 len;
    u32 flags;
    s8 run;
    s32 repeat;
    u8 value;
    s32 row;
    u16 width;
    u16 height;
    s32 col;

    if (*entry != 13) {
        return 0;
    }
    n = *(u16 *)(entry + 2);
    entry += 4;
    for (i = 0; i < n; i++) {
        word = *(u32 *)entry;
        len = word & 0xFFFFFF;
        if (i != size) {
            entry += (((len + 3) >> 2) << 2) + 12;
        } else {
            flags = word >> 24;
            if (x == -1) {
                rect.x = *(u16 *)(entry + 4);
                rect.y = *(u16 *)(entry + 6);
            } else {
                rect.x = x;
                rect.y = y;
            }
            rect.w = *(u16 *)(entry + 8);
            rect.h = *(u16 *)(entry + 10);
            data = entry + 12;
            if (!(flags & 12)) {
                run = 0;
                repeat = 0;
                value = 0;
                out = buf;
                row = 0;
                width = (u16)(rect.w * 2);
                height = rect.h;
                for (; row < height; row++) {
                    if (width != 0) {
                        col = width;
                        do {
                            if (run <= 0) {
                                run = *(s8 *)data++;
                                if (run < 0) {
                                    repeat = 0;
                                    run = -run;
                                } else {
                                    repeat = 1;
                                    value = *data++;
                                }
                            }
                            if (repeat == 0) {
                                *out = *data++;
                            } else {
                                *out = value;
                            }
                            out++;
                            col--;
                            run--;
                        } while (col != 0);
                    }
                }
                LoadImage(&rect, buf);
            } else {
                LoadImage(&rect, (u32 *)data);
            }
            break;
        }
    }
    return rect.w * (rect.h << 1);
}
