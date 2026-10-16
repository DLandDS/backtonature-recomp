/* Included by src/gameplay.c; shares its private declarations. */
void sound_init(void)
{
    SsInit();
    SsSetTableSize(soundTable, 6, 1);
    SsSetTickMode(1);
    sound_reserve_voice(0x16);
    soundLoadState = 0;
    soundSsRunning = 0;
    soundBeginCount = 0;
    soundStereo = 1;
    SsUtReverbOff();
    spuAttr.mask = 0x2C3;
    spuAttr.mvol.left = 0x3FFF;
    spuAttr.mvol.right = 0x3FFF;
    spuAttr.cdVolume.left = 0x1FFF;
    spuAttr.cdVolume.right = 0x1FFF;
    spuAttr.cdMix = 1;
    SpuSetCommonAttr(&spuAttr);
    SsSetSerialAttr(0, 0, 1);
    SsSetSerialVol(0, 0, 0);
    CdControl(9, 0, 0);
    sound_reset_slots();
}
void sound_clear_stamps(void)
{
    s32 i;

    for (i = 23; i >= 0; i--) {
        voiceStamps[i] = 0;
    }
}
extern s32 vabSlot;
extern s32 currentEntry;
extern s32 seqSlot;
extern s32 vabBodies[];
extern s32 vabEnds[];
extern s32 seqOffsets[];
extern s32 voiceCategories[];
extern s32 voicePriorities[];

void sound_reset_slots(void)
{
    s32 i;
    s32 j;

    vabSlot = 0;
    currentEntry = 0;
    for (i = 0; i < 6; i++) {
        queuedSounds[i] = -1;
        queuedSoundHasArg[i] = 0;
        storage8006BF88[i] = 0;
        vabIds[i] = -1;
        seqSlots[i] = -1;
        seqData[i] = -1;
        vabBodies[i] = -1;
        vabEnds[i] = -1;
        ((s32 *)seqVabs)[i] = -1;
        seqOffsets[i] = -1;
        for (j = 0; j < 1; j++) {
            seqStates[i + j] = -1;
            seqHandles[i + j] = -1;
        }
    }
    queueCursor = 0;
    seqSlot = 0;
    for (i = 0; i < 24; i++) {
        voiceCategories[i] = 0;
        voicePriorities[i] = 0x1000000;
    }
}
s32 sound_begin_load(s32 arg0, s32 arg1)
{
    if (soundBeginCount != 0) {
        sound_release_all();
    }

    soundWritePtr = arg0;
    soundReadPtr = arg1;
    vabFreeMark = 0x1010;
    soundBeginCount++;
    sound_reset_slots();
    return 0;
}
s32 sound_queue_seq(s32 seq)
{
    s32 index = queueCursor;

    queuedSounds[index] = seq << 1;
    queueCursor = index + 1;
    return 0;
}
s32 sound_queue_seq_pair(s32 seq, s32 entryArg)
{
    s32 index = queueCursor;
    queuedSounds[index] = seq << 1;
    queuedSoundHasArg[index] = 1;
    storage8006BF88[index] = entryArg;
    queueCursor = index + 1;
    return 0;
}
s32 sound_seq_start(s32 soundId, s32 slotId, s32 left, s32 right, s16 handle)
{
    s32 *base = seqStates;
    s32 *slot = base + (slotId + soundId);

    if (*slot == -1) {
        SsUtAllKeyOff(0);
        SsSeqSetVol((s16)seqSlots[soundId], (s16)left, (s16)right);
        SsSeqStop((s16)seqSlots[soundId]);
        SsSeqPlay((s16)seqSlots[soundId], 1, 0);
        *slot = 0;
        seqHandles[slotId + soundId] = handle;
    }
    return 0;
}
s32 sound_seq_volume_if_idle(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 *base = seqStates;
    s32 *slot = base + (arg1 + arg0);

    if (*slot == 0) {
        SsSeqSetVol((s16)seqSlots[arg0], (s16)arg3, (s16)arg3);
    }
    return 0;
}

s32 sound_seq_set_level(s32 soundId, s32 slotId, s32 level)
{
    s32 *base = seqStates;
    s32 *slot = base + (slotId + soundId);

    if (*slot != -1) {
        if (level > 0) {
            SsSeqSetVol((s16)seqSlots[soundId], (s16)level, (s16)level);
            return 1;
        }
        SsSeqStop((s16)seqSlots[soundId]);
        *slot = -1;
        return 0;
    }
    return 0;
}

s32 sound_seq_set_volume(s32 soundId, s32 slotId, s32 volume)
{
    s32 *base = seqStates;
    s32 *slot = base + (slotId + soundId);
    s32 sentinel = -1;

    if ((*slot != sentinel) && (volume < 0x7E)) {
        SsSeqSetVol((s16)seqSlots[soundId], (s16)volume, (s16)volume);
        return 1;
    }
    return 0;
}
s32 sound_stub_1(void)
{
    return 0;
}
s32 sound_seq_stop(s32 soundId, s32 slotId)
{
    s32 *base = seqStates;
    s32 *slot = base + (slotId + soundId);
    s32 sentinel = -1;

    if (*slot != sentinel) {
        SsSeqStop((s16)seqSlots[soundId]);
        *slot = sentinel;
    }
    return 0;
}
s32 sound_seq_pause(s32 soundId, s32 slotId)
{
    s32 *base = seqStates;
    s32 *slot = base + (slotId + soundId);

    if (*slot == 0) {
        SsSeqPause((s16)seqSlots[soundId]);
        *slot = 1;
    }
    return 0;
}
s32 sound_seq_resume(s32 soundId, s32 slotId)
{
    s32 *base = seqStates;
    s32 *slot = base + (slotId + soundId);

    if (*slot == 1) {
        SsSeqReplay((s16)seqSlots[soundId]);
        *slot = 0;
    }
    return 0;
}
s32 sound_reserve_voice(s32 voice)
{
    if ((s8)SsSetReservedVoice((u8)voice) < 0) {
        return -1;
    }
    return 0;
}
extern u16 soundDefs[][7];
extern s16 toneVoice;
extern s16 playingProg;
extern s16 nextProg;
extern s16 playingTone;
extern s16 nextTone;
extern s16 playingNote;
extern s16 nextNote;
extern s16 nextFine;
extern short SsUtKeyOff(short voice, short vabId, short prog, short tone, short note);
extern short SsUtKeyOn(short vabId, short prog, short tone, short note, short fine, short voll,
                       short volr);

s16 sound_replace_tone(s32 vab, s32 index, s32 voll, s32 volr)
{
    nextProg = soundDefs[index][2];
    nextTone = soundDefs[index][3];
    nextNote = soundDefs[index][4];
    nextFine = soundDefs[index][5];
    SsUtKeyOff(toneVoice, vab, playingProg, playingTone, playingNote);
    toneVoice = SsUtKeyOn(vab, nextProg, nextTone, nextNote, nextFine, voll, volr);
    playingProg = nextProg;
    playingTone = nextTone;
    playingNote = nextNote;
    return toneVoice;
}
typedef struct VoiceSlot {
    s32 vab;
    s32 index;
    s32 voll;
    s32 volr;
    s32 unk10;
} VoiceSlot;
extern VoiceSlot voicePlays[];
extern short SsUtKeyOnV(short voice, short vabId, short prog, short tone, short note, short fine,
                        short voll, short volr);

s32 sound_key_on_voice(s32 vab, s32 index, s32 voll, s32 volr, s32 arg4, s32 voice)
{
    SsUtKeyOnV(voice, vab, soundDefs[index][2], soundDefs[index][3], soundDefs[index][4],
               soundDefs[index][5], voll, volr);
    voicePlays[voice].vab = vab;
    voicePlays[voice].index = index;
    voicePlays[voice].voll = voll;
    voicePlays[voice].volr = volr;
    voicePlays[voice].unk10 = arg4;
    return 0;
}
s32 sound_voice_key_off(s32 vabId, s32 voice, s32 mode)
{
    SsVoKeyOff(voice + (vabId << 8), mode);
    return 0;
}
s32 sound_voice_stop(s32 arg0)
{
    s32 voice = (s16)arg0;
    s32 result;

    SsUtSetVVol(voice, 0, 0);
    result = SsUtKeyOffV(voice);
    if (result < 0) {
        return -1;
    }
    return 0;
}
s32 sound_stub_2(void)
{
    return 0;
}
s32 sound_voice_set_volume(s32 voice, s32 voll, s32 volr)
{
    SsUtSetVVol((s16)voice, (s16)voll, (s16)volr);
    return 0;
}
s32 sound_update(void)
{
    s32 i;

    for (i = 0; i < 6; i++) {
        if (seqData[i] != -1 && seqSlots[i] == -1) {
            seqSlots[i] = SsSeqOpen(seqData[i], seqVabs[i * 2]);
        }
    }
    if (soundSsRunning == 0) {
        SsStart();
    }
    soundSsRunning = 1;
    if (soundStereo != 0) {
        SsSetStereo();
    } else {
        SsSetMono();
    }
    return 0;
}
s32 sound_set_reverb_type(s32 type)
{
    s32 result = SsUtSetReverbType((s16)type);
    SsUtSetReverbDepth(0, 0);
    return result;
}
s32 sound_set_reverb_depth(s32 depth)
{
    return SsUtSetReverbDepth((s16)depth, (s16)depth);
}
s32 sound_set_reverb_volume(s32 volume)
{
    return SsSetRVol((s16)volume, (s16)volume);
}
s32 sound_set_reverb_feedback(s32 feedback)
{
    return SsUtSetReverbFeedback((s16)feedback);
}
s32 sound_set_reverb_delay(s32 delay)
{
    return SsUtSetReverbDelay((s16)delay);
}
s32 sound_reverb_on(void)
{
    return SsUtReverbOn();
}
s32 sound_reverb_off(void)
{
    return SsUtReverbOff();
}
s32 sound_set_mono(void)
{
    SsSetMono();
    soundStereo = 0;
    sound_stereo_mode_hook();
}
void sound_set_stereo(void)
{
    SsSetStereo();
    soundStereo = 1;
    sound_stereo_mode_hook();
}
s32 sound_is_stereo(void)
{
    return soundStereo;
}

s32 sound_get_vab_free_mark(void)
{
    return vabFreeMark;
}

s32 sound_get_write_ptr(void)
{
    return soundWritePtr;
}
s32 sound_release_all(void)
{
    s32 i;
    s32 j;

    SsSetMVol(0, 0);
    for (i = 0; i < 6; i++) {
        for (j = 0; j <= 0; j++) {
            sound_seq_stop(i, j);
        }
    }
    for (i = 0; i < 6; i++) {
        if (seqSlots[i] != -1) {
            SsSeqClose(seqSlots[i]);
            seqSlots[i] = -1;
        }
    }
    SsEnd();
    soundSsRunning = 0;
    for (i = 0; i < 6; i++) {
        if (vabIds[i] >= 0) {
            SsVabClose(vabIds[i]);
            vabIds[i] = -1;
        }
    }
    return 0;
}
extern u32 vabStarts[];
s32 sound_release_from(s32 start, s32 seq)
{
    s32 i, j;
    u32 a, b;
    a = vabBodies[start];
    vabSlot = start;
    seqSlot = seq;
    if (a != -1) {
        vabFreeMark = vabStarts[start];
        b = seqData[seq];
        if (b < a && b != -1)
            soundWritePtr = b;
        else
            soundWritePtr = a;
    }
    for (i = seq; i < 6; i++) {
        for (j = 0; j < 1; j++) {
            SsSeqSetVol(i, 0, 0);
            sound_seq_stop(i, j);
        }
    }
    for (i = seq; i < 6; i++) {
        if (seqSlots[i] != -1) {
            SsSeqClose((s16)seqSlots[i]);
            seqSlots[i] = -1;
        }
        seqData[i] = -1;
        ((s32 *)seqVabs)[i] = -1;
        seqOffsets[i] = -1;
    }
    for (i = *&start; i < 6; i++) {
        if (vabIds[i] >= 0) {
            SsVabClose((s16)vabIds[i]);
            vabIds[i] = -1;
        }
        vabBodies[i] = -1;
        vabEnds[i] = -1;
        for (j = 0; j < 1; j++) {
            seqStates[i + j] = -1;
            seqHandles[i + j] = -1;
        }
    }
    seqSlot = seq;
    currentEntry = 0;
    vabSlot = *&start;
    for (i = 0; i < 6; i++) {
        queuedSounds[i] = -1;
        queuedSoundHasArg[i] = 0;
        storage8006BF88[i] = 0;
    }
    queueCursor = 0;
    return 0;
}
