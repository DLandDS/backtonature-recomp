/* Included by src/archive.c; shares its private declarations. */
void DecodeContainer0D(void *container, s16 x, s16 y, s16 clutx, s16 cluty, s32 mode)
{
    u8 *block;
    u8 *data;
    u8 *out;
    RECT rect;
    u32 len;
    register u32 mask __asm__("$2");
    u32 flags;
    u16 count;
    u16 n;
    s8 run;
    s32 repeat;
    u8 value;
    u16 width;
    u16 height;
    s32 row;
    register s32 col __asm__("$5");
    u16 z;
    u16 b;
    u32 f2;
    u16 rx, ry, rw, rh;

    block = container;
    value = 0;
    if (block[0] != 13) {
        return;
    }
    count = *(u16 *)(block + 2);
    block += 4;
    for (n = 0; n < count; n++) {
        mask = 0xFFFFFF;
        data = block + 12;
        len = *(u32 *)block & mask;
        flags = *(u32 *)block >> 24;
        rx = *(u16 *)(block + 4);
        ry = *(u16 *)(block + 6);
        rw = *(u16 *)(block + 8);
        rh = *(u16 *)(block + 10);
        rect.x = rx;
        rect.h = rh;
        rect.y = ry;
        rect.w = rw;
        if ((flags & 3) != 2) {
            rect.x = x - (-rx);
            rect.y = y - (-ry);
        } else if ((s16)rw < 256 || mode != 2) {
            rect.x = clutx - (-rx);
            rect.y = cluty - (-ry);
        }
        run = 0;
        repeat = 0;
        width = rect.w * 2;
        height = rect.h;
        if (!(flags & 12)) {
            rect.h = 1;
            for (row = 0; row < height; row++) {
                f2 = flags;
                rh = 2;
                f2 = f2 & rh;
                out = (u8 *)textureRowBuffer;
                if (width != 0) {
                    col = width;
                    __asm__("" : "+r"(col));
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
                        out += 1;
                        col--;
                        run--;
                    } while (col != 0);
                }
                if (f2 && mode == 1) {
                    for (z = 1; z < rect.w; z++) {
                        textureRowBuffer[z] |= 0x8000;
                    }
                }
                LoadImage(&rect, (u32 *)textureRowBuffer);
                rect.y++;
            }
        } else if (flags & 8) {
            rect.w = 1;
            for (row = 0; row < width; row += 2) {
                for (b = 0; b < 2; b++) {
                    f2 = flags & 2;
                    out = (u8 *)textureRowBuffer + (b != 0);
                    if (height != 0) {
                        col = height;
                        __asm__("" : "+r"(col));
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
                            out += 2;
                            col--;
                            run--;
                        } while (col != 0);
                    }
                }
                if (f2 && mode == 1) {
                    for (z = 1; z < rect.w; z++) {
                        textureRowBuffer[z] |= 0x8000;
                    }
                }
                LoadImage(&rect, (u32 *)textureRowBuffer);
                rect.x++;
            }
        } else {
            rect.h = 1;
            for (row = 0; row < height; row++) {
                f2 = flags;
                rh = 2;
                f2 = f2 & rh;
                out = (u8 *)textureRowBuffer;
                if (width != 0) {
                    col = width;
                    __asm__("" : "+r"(col));
                    do {
                        *out++ = *data++;
                        col--;
                    } while (col != 0);
                }
                if (f2 && mode == 1) {
                    for (z = 1; z < rect.w; z++) {
                        textureRowBuffer[z] |= 0x8000;
                    }
                }
                LoadImage(&rect, (u32 *)textureRowBuffer);
                rect.y++;
            }
        }
        while (DrawSync(1)) {
        }
        block += (((len + 3) >> 2) << 2) + 12;
    }
}
