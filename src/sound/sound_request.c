/* Included by src/gameplay.c; shares its private declarations. */
s32 sound_play(s16 id)
{
    return sound_key_on_free_voice(id, 0, 0x17);
}
void sound_release_voice(s32 voice)
{
    sound_key_off_voice(voice);
}
s32 sound_last_voice_id(void)
{
    return 0x17;
}
s32 sound_stop_last_voice(void)
{
    sound_voice_stop(0x17);
    return 0;
}
s32 sound_seq_start_reverb(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4)
{
    sound_seq_start(arg0, 0, arg3, arg3, arg1);
    SsUtSetReverbType(reverbTable[arg2 * 2]);
    SsUtReverbOn();
    SsUtSetReverbDepth(reverbTable[arg2 * 2 + 1], reverbTable[arg2 * 2 + 1]);
    return 0;
}
s32 sound_seq_stop_first(s16 value)
{
    sound_seq_stop((s32)value, 0);
    return 0;
}
void sound_set_seq_arg(s32 unused, s32 buffer)
{
    seqArg = buffer;
}
extern s16 soundRequestUnused8005E31C;
extern u8 seqBuffer[];
extern u8 storage80049296[];
void sound_clear_stamps(void);
s32 sound_begin_load();
s32 sound_queue_seq(s32 arg0);
s32 sound_queue_seq_pair(s32 arg0, s32 arg1);

void sound_request(s16 id)
{
    u8 *e;

    storage80061F1C[0] = 0;
    switch (id) {
    case 0:
        if (soundRequestUnused8005E31C == 0) {
            SsUtAllKeyOff(0);
            sound_clear_stamps();
            sound_begin_load(seqBuffer, seqArg, 0, 0);
            sound_queue_seq(0);
            soundRequestUnused8005E31C = 0;
        }
        break;
    case 999:
        sound_queue_seq(2);
        break;
    case 1000:
        sound_queue_seq(1);
        break;
    default:
        e = &storage80049296[id * 2];
        sound_queue_seq(e[0]);
        sound_queue_seq_pair(e[1], 2);
        break;
    }
}
void sound_request_stub_1(void) {}
void sound_fader_init(void)
{
    masterFader[0] = 0;
    masterFader[1] = 0;
    masterFader[2] = 0x7F0000;
    masterFader[3] = 0;
    masterFader[4] = 0x7F0000;
    masterFader[5] = 1;
}
void sound_load_tick(void)
{
    if (sound_current_entry() != (u32)-1) {
        sound_load_step();
    } else {
        masterFader[5] = 1;
    }
    if (masterFader[1] != 0) {
        sound_fader_step();
    }
}
s32 sound_fader_ready(void)
{
    return storage80061F1C[0] != 0;
}
/* One allocation-only form exhausted: retail keeps the state base in a0;
 * cc1 reassigned it and moved the signed rounding path, producing 21 diffs.
 * Restore the exact reference body. */
void sound_fader_step(void)
{
    s32 volume;

    masterFader[2] += masterFader[3];
    if (masterFader[2] <= 0) {
        masterFader[2] = 0;
        masterFader[1] = 0;
    }
    if (masterFader[2] > 0x7F0000) {
        masterFader[2] = 0x7F0000;
        masterFader[1] = 0;
    }

    volume = masterFader[2];
    if (volume < 0) {
        volume += 0xFFFF;
    }
    volume >>= 16;
    SsSetMVol((s16)volume, (s16)volume);
}
void sound_fade_in(s32 ticks)
{
    masterFader[3] = masterFader[4] / ticks;
    masterFader[1] = 1;
}
s32 sound_fade_out(s32 ticks)
{
    s32 result = -masterFader[2] / ticks;

    masterFader[3] = result;
    masterFader[1] = 2;
    return result;
}
s32 sound_fader_mode(void)
{
    return storage80061F0C[0];
}
void sound_init_stereo_xa(void)
{
    sound_set_stereo();
    xa_search_files();
}
typedef struct SpuVolumeAttr16 {
    s16 left;
    s16 right;
} SpuVolumeAttr16;
typedef struct SpuCommonAttrLite {
    u32 mask;
    SpuVolumeAttr16 mvol;
    SpuVolumeAttr16 mvolmode;
    SpuVolumeAttr16 mvolx;
    SpuVolumeAttr16 cdVolume;
    s32 cdReverb;
    s32 cdMix;
} SpuCommonAttrLite;
extern SpuCommonAttrLite spuAttr;
extern u8 soundTable[];
extern s32 soundLoadState;
extern void SsInit(void);
extern void SsSetTableSize(u8 *table, s16 sMax, s16 tMax);
extern void SsSetTickMode(s32 mode);
extern void SpuSetCommonAttr(SpuCommonAttrLite *attr);
extern void SsSetSerialAttr(s8 sNum, s8 attr, s8 mode);
extern void SsSetSerialVol(s8 sNum, s16 voll, s16 volr);
extern int CdControl(u8 com, u8 *param, u8 *result);
s32 sound_reserve_voice(s32 arg0);
void sound_reset_slots(void);
