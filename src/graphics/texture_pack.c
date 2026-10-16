/* Included by src/archive.c; shares its private declarations. */
void texture_pack_load_stp(void *container)
{
    DecodeContainer0D(container, 0, 0, 0, 0, 1);
}

void texture_pack_load_at(void *container, s16 x, s16 y, s16 altX, s16 altY)
{
    DecodeContainer0D(container, x, y, altX, altY, 0);
}

void texture_pack_load_keep_wide(void *container, s16 x, s16 y, s16 altX, s16 altY)
{
    DecodeContainer0D(container, x, y, altX, altY, 2);
}
extern u8 textureUploadBuffer[];
u16 anim_find_bank_image(void *archive, void *entry, u16 id);

void sprite_load_bank_image(void *archive, void *entry, u8 group, u8 index, s16 x, s16 y)
{
    image_bank_upload(
        entry, textureUploadBuffer,
        anim_find_bank_image(archive, entry, anim_part_palette(archive, group, index)), x, y);
}
/* Header halves read in memory order (4, 6, 8, 0xA) with no asm barrier
 * ahead of them: the backward scheduler then places the and of the lw
 * right after the loads, which demotes the lw to first, as retail. */
extern u16 textureRowBuffer[];
extern int DrawSync(int mode);
