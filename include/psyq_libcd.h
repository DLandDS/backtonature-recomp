#ifndef PSYQ_LIBCD_H
#define PSYQ_LIBCD_H

/* Minimal interface declarations compatible with the PSY-Q 4.6 CD library.
 * `LIBCD.H:<line>` notes point to the equivalent declaration in the user's own
 * SDK; SDK commentary is not reproduced. The SDK header is deliberately not put
 * on the cpp path: this project declares only the CD ABI it uses. */

/* LIBCD.H:169 */
typedef struct {
    u8 minute;
    u8 second;
    u8 sector;
    u8 track;
} CdlLOC;

/* LIBCD.H:201 */
typedef struct {
    CdlLOC pos;
    u32 size;
    char name[16];
} CdlFILE;

typedef struct {
    u8 val0;
    u8 val1;
    u8 val2;
    u8 val3;
} CdlATV;

/* LIBCD.H:287 */
CdlFILE *CdSearchFile(CdlFILE *fp, char *name);
/* LIBCD.H:291 */
int CdControl(u8 com, u8 *param, u8 *result);
/* LIBCD.H:292 */
int CdControlB(u8 com, u8 *param, u8 *result);
/* LIBCD.H:294 */
int CdGetSector(void *madr, int size);
/* LIBCD.H:299 */
int CdMix(CdlATV *vol);
/* LIBCD.H:300 */
int CdPosToInt(CdlLOC *p);
/* LIBCD.H:302 */
CdlLOC *CdIntToPos(int i, CdlLOC *p);
/* LIBCD.H:301 */
int CdRead(int sectors, u32 *buf, int mode);
/* LIBCD.H:304 */
int CdReadSync(int mode, u8 *result);
/* LIBCD.H:307 */
int CdSync(int mode, u8 *result);
/* LIBCD.H:312 */
int CdInit(void);

#endif
