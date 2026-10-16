/* Included by src/ui.c; shares its private declarations. */
/* CONFIDENCE: exact leaf bodies; unused parameters, if any, are not observable. */
void sound_stereo_mode_hook(void) {}
extern CdlFILE xaSearchResults[];
extern char *xaFileNames[];

void xa_search_files(void)
{
    s32 i;

    for (i = 0; i < 4; i++) {
        while (CdSearchFile(&xaSearchResults[i], xaFileNames[i]) == 0) {
        }
    }
}
extern s16 cdAudioVolume;
extern s32 cdAudioStart;
extern s32 cdAudioPos;
extern s32 cdAudioStep;
extern u8 main_get_speed_mode(void);
extern s32 func_80035A44(s32 mode, u8 *result);
extern s32 CdLastCom(void);
extern int CdControlF(u8 com, u8 *param);
extern void SsSetSerialVol(s16 sNum, s16 voll, s16 volr);
void cd_audio_stop(void);
s32 cd_audio_restart(u8 com);

void cd_audio_fade_tick(void)
{
    u8 result[8];
    s32 step;
    s32 pos;
    s32 vol;
    s32 r;

    if (cdAudioRequest == 0) {
        return;
    }
    switch (main_get_speed_mode()) {
    case 0:
    case 1:
        step = 1;
        break;
    case 2:
        step = 2;
        break;
    case 3:
        step = 3;
        break;
    default:
        step = 1;
        break;
    }
    switch (cdAudioStep) {
    case 0:
    case 1:
    case 2:
        cdAudioStep++;
        break;
    case 3:
        if (func_80035A44(1, result) == 2 && cd_audio_restart(0x1B) == 0) {
            cdAudioStep++;
        }
        break;
    case 4:
        r = func_80035A44(1, result);
        if (r == 5) {
            goto reseek;
        }
        if (r == 2) {
            if (cdAudioVolume <= 0) {
                cd_audio_stop();
            } else if (cdAudioPos < cdAudioStart - 300) {
            reseek:
                cd_audio_restart(0x1B);
            } else {
                if (CdLastCom() == 0x11) {
                    pos = CdPosToInt((CdlLOC *)&result[5]);
                    if (pos > 0) {
                        cdAudioPos = pos;
                    }
                }
                cdAudioRequest = 2;
                CdControlF(0x11, 0);
                vol = cdAudioVolume - step;
                if (vol < 0) {
                    vol = 0;
                }
                cdAudioVolume = vol;
                if (cdAudioVolume <= 0x20) {
                    SsSetSerialVol(0, cdAudioVolume * 4, cdAudioVolume * 4);
                } else {
                    SsSetSerialVol(0, 0x2D, 0x2D);
                }
            }
        }
        break;
    }
}
extern void SsSetSerialVol(s16 sNum, s16 voll, s16 volr);
void cd_audio_stop(void)
{
    if (cdAudioRequest != 0) {
        SsSetSerialVol(0, 0, 0);
        CdControl(7, 0, 0);
        cdAudioRequest = 0;
    }
}
s32 cd_audio_get_state(void)
{
    if (cdAudioRequest == 0) {
        return 0;
    }
    if (cdAudioRequest == 2) {
        return 2;
    }
    return 1;
}
extern u16 cdAudioTracks[][3];
extern s32 cdAudioRestarts;
extern s32 cdAudioTrack;
extern s32 cdAudioFile;
extern s32 cdAudioChannel;
int event_flag_get(u16 index);
extern void CdFlush(void);

void cd_audio_play_track(s32 idx)
{
    s16 *track;
    u8 mode[8];
    u8 filter[8];

    if (gameState[0xCC] != 0) {
        return;
    }
    if (event_flag_get(2) == 1 && *(s32 *)(gameState + 0x7A74) != 1) {
        return;
    }
    track = (s16 *)cdAudioTracks[idx];
    CdFlush();
    cd_audio_stop();
    SsSetSerialVol(0, 0, 0);
    cdAudioRequest = 1;
    cdAudioStep = 0;
    cdAudioTrack = idx;
    cdAudioChannel = track[2];
    cdAudioFile = track[1];
    cdAudioVolume = track[0];
    filter[0] = 1;
    filter[1] = cdAudioChannel;
    while (CdControlB(0xD, filter, 0) != 1) {
    }
    cdAudioStart = CdPosToInt(&xaSearchResults[cdAudioFile].pos);
    cdAudioPos = cdAudioStart;
    cdAudioRestarts = 0;
    mode[0] = 0xC8;
    while (CdControlB(0xE, mode, 0) != 1) {
    }
}
extern u16 cdAudioTracks[][3];
extern s16 cdAudioVolume;
extern s32 cdAudioStart;
extern s32 cdAudioRestarts;
extern s32 cdAudioPos;
extern s32 cdAudioTrack;

s32 cd_audio_restart(u8 com)
{
    u16 *track = cdAudioTracks[cdAudioTrack];
    CdlLOC pos;

    cdAudioRequest = 1;
    cdAudioVolume = track[0];
    CdIntToPos(cdAudioStart, &pos);
    if (CdControl(com, (u8 *)&pos, 0) != 1) {
        return -1;
    }
    cdAudioPos = cdAudioStart;
    cdAudioRestarts++;
    return 0;
}
