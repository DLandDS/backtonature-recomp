/* Included by src/ui.c; shares its private declarations. */
u32 sound_current_entry(void)
{
    return queuedSounds[currentEntry];
}
/* Ported from hmbtn-decomp (CC0), sound_load_step. */
short SsVabOpenHead(unsigned char *, short);
extern short SsVabTransCompleted(short immediate);
extern s32 afile_read_async(s32 arg0, u32 arg1, u32 arg2, u8 *arg3, s32 arg4);
extern s32 currentEntry;
extern s32 storage8006BF88[8];
void *memcpy();
extern s32 queuedSounds[];
extern u8 *seqData[8];
extern s32 seqOffsets[8];
extern s32 seqSlot;
extern s32 seqVabs[8];
extern s32 soundLoadState;
s32 sound_load_step(void);
extern s32 soundOffsets[];
extern u8 *soundReadPtr;
void sound_schedule_load(s16 id);
extern s32 soundSeqBytes;
void sound_vab_stream_setup(u32 arg0);
void sound_vab_stream_start(s32 arg0, s32 arg1, s32 arg2);
s32 sound_vab_stream_step(void);
extern u8 *soundWritePtr;
extern u8 *vabBodies[8];
extern u8 *vabEnds[8];
extern s32 vabFreeMark;
extern s32 vabIds[8];
extern s32 vabSlot;
extern s32 vabStarts[8];
enum SoundLoadState {
    SOUND_LOAD_IDLE = 0,
    SOUND_LOAD_VAB_OPEN = 10,
    SOUND_LOAD_VAB_DONE = 20,
    SOUND_LOAD_VAB_HEADER = 30,
    SOUND_LOAD_SEQ = 40,
    SOUND_LOAD_WHOLE = 50,
    SOUND_LOAD_VAB_STREAM = 60,
};

s32 sound_load_step(void)
{
    s32 entry;

    switch (soundLoadState) {
    case SOUND_LOAD_SEQ:
        seqOffsets[seqSlot] = soundOffsets[queuedSounds[currentEntry] + (1)];
        soundSeqBytes = soundOffsets[queuedSounds[currentEntry] + (2)] -
                        soundOffsets[queuedSounds[currentEntry] + (0)];
        memcpy(soundWritePtr, soundReadPtr, soundSeqBytes);
        soundLoadState = SOUND_LOAD_IDLE;
        seqData[seqSlot] = soundWritePtr;
        seqVabs[seqSlot] = vabIds[storage8006BF88[currentEntry]];
        soundWritePtr += soundOffsets[queuedSounds[currentEntry] + (2)] -
                         soundOffsets[queuedSounds[currentEntry] + (0)];
        seqSlot++;
        currentEntry++;
        break;
    case SOUND_LOAD_VAB_OPEN:
        memcpy(soundWritePtr, soundReadPtr,
               soundOffsets[queuedSounds[currentEntry] + (1)] -
                   soundOffsets[queuedSounds[currentEntry] + (0)]);
        vabStarts[vabSlot] = vabFreeMark;
        vabIds[vabSlot] = SsVabOpenHead(soundWritePtr, -1);
        sound_vab_stream_setup(vabIds[vabSlot]);
        soundLoadState = SOUND_LOAD_VAB_STREAM;
        break;
    case SOUND_LOAD_VAB_DONE:
        SsVabTransCompleted(1);
        soundLoadState = SOUND_LOAD_IDLE;
        vabBodies[vabSlot] = soundWritePtr;
        soundWritePtr += soundOffsets[queuedSounds[currentEntry] + (1)] -
                         soundOffsets[queuedSounds[currentEntry] + (0)];
        vabEnds[vabSlot] = soundWritePtr;
        vabSlot++;
        currentEntry++;
        break;
    case SOUND_LOAD_IDLE:
        sound_schedule_load(queuedSounds[currentEntry]);
        break;
    case SOUND_LOAD_VAB_HEADER:
        entry = queuedSounds[currentEntry];
        if (afile_read_async(9, soundOffsets[entry], soundOffsets[entry + 1] - soundOffsets[entry],
                             soundReadPtr, 0) == 1) {
            soundLoadState = SOUND_LOAD_VAB_OPEN;
            sound_vab_stream_start(entry, soundReadPtr, (45 * 4096));
        }
        break;
    case SOUND_LOAD_WHOLE:
        entry = queuedSounds[currentEntry];
        if (afile_read_async(9, soundOffsets[entry], soundOffsets[entry + 2] - soundOffsets[entry],
                             soundReadPtr, 0) == 1) {
            soundLoadState = SOUND_LOAD_SEQ;
        }
        break;
    case SOUND_LOAD_VAB_STREAM:
        if (sound_vab_stream_step() != 0) {
            soundLoadState = SOUND_LOAD_VAB_DONE;
            vabFreeMark += soundOffsets[queuedSounds[currentEntry] + (2)] -
                           soundOffsets[queuedSounds[currentEntry] + (1)];
        }
        break;
    }
    return queuedSounds[currentEntry];
}

extern int VSync(int mode);
extern void SpuGetAllKeysStatus(u8 *array);
extern s32 voiceCategories[];
extern s32 voicePriorities[];
extern s16 soundDefs[];
extern s16 SsUtKeyOnV(s16 voice, s16 vab, s16 prog, s16 tone, s16 note, s16 fine, s16 voll,
                      s16 volr);
extern u32 rand_range(u32 range);
s32 sound_find_free_voice(s32 now, s32 lo, s32 hi, u8 *keys);
s32 sound_find_lowest_voice(s32 lo, s32 hi);

s32 sound_key_on_free_voice(s32 id, s32 lo, s32 hi)
{
    u8 keys[24];
    s32 now;
    s32 voice;
    s16 *sfx;
    s16 vab;
    s16 prog;
    s16 tone;
    s16 note;
    s16 fine;
    s16 voll;
    s16 volr;

    now = VSync(-1);
    SpuGetAllKeysStatus(keys);
    voice = sound_find_free_voice(now, lo, hi, keys);
    if (voice < 0) {
        voice = sound_find_lowest_voice(lo, hi);
    }
    sfx = &soundDefs[id * 7];
    voiceCategories[voice] = sfx[1];
    voicePriorities[voice] = VSync(-1);
    note = sfx[4];
    volr = sfx[6];
    vab = sfx[0];
    prog = sfx[2];
    tone = sfx[3];
    fine = sfx[5];
    voll = volr;
    if (id == 0x46) {
        note = rand_range(4) + 0x3C;
        voll = rand_range(0x1C) + 0x64;
        volr = rand_range(0x1C) + 0x64;
    }
    if (id == 0x47) {
        note = rand_range(9) + 0x38;
        voll = rand_range(0x1C) + 0x64;
        volr = rand_range(0x1C) + 0x64;
    }
    SsUtKeyOnV(voice, vab, prog, tone, note, fine, voll, volr);
    voiceStamps[voice] = now;
    return voice;
}
extern void SpuGetAllKeysStatus(u8 *array);
extern s32 SsUtKeyOffV(s32 arg0);
extern s32 voiceCategories[];
extern s32 voicePriorities[];

s32 sound_key_off_voice(s32 voice)
{
    u8 buf[24];
    u8 status;

    SpuGetAllKeysStatus(buf);
    status = buf[voice];
    if (status == 0 || status == 3) {
        return 0;
    }
    voiceCategories[voice] = 0;
    voicePriorities[voice] = 0x1000000;
    SsUtKeyOffV((s16)voice);
    return 0;
}
extern s32 queuedSoundHasArg[];
extern s16 soundScheduledValue;
extern u8 *soundReadPtr;
extern s32 soundLoadState;
extern s32 afile_read_async(s32 arg0, u32 arg1, u32 arg2, u8 *arg3, s32 arg4);

void sound_schedule_load(s16 id)
{
    soundScheduledValue = id;
    if ((s32)queuedSounds[currentEntry] >= 0) {
        if (queuedSoundHasArg[currentEntry] != 0) {
            afile_read_async(9, soundOffsets[id], soundOffsets[id + 2] - soundOffsets[id],
                             soundReadPtr, 1);
            soundLoadState = 50;
        } else {
            afile_read_async(9, soundOffsets[id], soundOffsets[id + 1] - soundOffsets[id],
                             soundReadPtr, 1);
            soundLoadState = 30;
        }
    } else {
        soundLoadState = 0;
    }
}
void sound_vab_stream_start(s32 sound, s32 dest, s32 chunkSize)
{
    vabStream[0] = sound;
    vabStream[1] = dest;
    vabStream[2] = chunkSize;
}
void sound_vab_stream_setup(u32 vabId)
{
    s32 n;

    n = vabStream[0];
    vabStream[3] = (u32)(soundOffsets[n + 2] - soundOffsets[n + 1]) / 184320;
    vabStream[6] = vabId;
    vabStream[7] = 0;
    vabStream[4] = 0;
    vabStream[5] = soundOffsets[n + 1];
}
typedef struct UiVabStream {
    u32 unk00;
    u8 *addr;
    u32 chunk;
    u32 count;
    u32 index;
    u32 offset;
    s32 vabId;
    s32 state;
} UiVabStream;
extern UiVabStream vabStreamCopy;
extern s32 afile_read_async(s32 arg0, u32 arg1, u32 arg2, u8 *arg3, s32 arg4);
extern short SsVabTransBodyPartly(u8 *addr, u32 bufsize, short vabid);
extern short SsVabTransCompleted(short immediate);

#define VS ((UiVabStream *)vabStream)
s32 sound_vab_stream_step(void)
{
    if (VS->index <= VS->count) {
        switch (VS->state) {
        case 0:
            afile_read_async(9, VS->offset, VS->chunk, VS->addr, 1);
            VS->state = 1;
            break;
        case 1:
            if (afile_read_async(9, VS->offset, VS->chunk, VS->addr, 0) == 1) {
                SsVabTransBodyPartly(VS->addr, VS->chunk, VS->vabId);
                vabStreamCopy = *(UiVabStream *)vabStream;
                VS->state = 0;
                while (SsVabTransCompleted(1) == 0) {
                }
                VS->offset += VS->chunk;
                VS->index++;
            }
            break;
        }
        return 0;
    }
    return 1;
}
#undef VS
s32 sound_find_free_voice(s32 value, s32 from, s32 to, u8 *states)
{
    s32 idx;

    if (to < from) {
        goto fail;
    }
    idx = from;
    do {
        u8 flag = states[idx];
        if ((flag == 0 || flag == 3) && (u32)(voiceStamps[idx] - value) >= 2) {
            return idx;
        }
        idx++;
    } while (to >= idx);
fail:
    return -1;
}
s32 sound_find_lowest_voice(s32 start, s32 end)
{
    s32 found = -1;
    s32 priority;
    s32 i, best;
    for (priority = 0; priority < 4; priority++) {
        best = 0x1000000;
        for (i = start; i <= end; i++) {
            if (voiceCategories[i] == priority && voicePriorities[i] < best) {
                found = i;
                best = voicePriorities[i];
            }
        }
        if (found != -1)
            return found;
    }
    return -1;
}
