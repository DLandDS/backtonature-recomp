#include "animation.h"
#include <SDL3/SDL_error.h>
#include <stdlib.h>

static unsigned le16(const uint8_t *p) { return (unsigned)p[0] | ((unsigned)p[1] << 8); }
static int signed16(const uint8_t *p)
{
    unsigned value = le16(p);
    return value < 32768u ? (int)value : (int)value-65536;
}
static bool extent(size_t total, size_t offset, size_t count, size_t stride)
{
    return offset <= total && count <= (total-offset)/stride;
}
bool btn_animation_parse(BtnAnimation *out, const uint8_t *data, size_t size)
{
    if (!out || !data || size < 36 || btn_read_le32(data) != 14)
        return SDL_SetError("Expected animation 0E header");
    BtnAnimation a = {0};
    a.data = data; a.size = size;
    a.images=le16(data+4); a.lists=le16(data+6); a.parts=le16(data+8); a.ranges=le16(data+10);
    a.frames=btn_read_le32(data+12);
    size_t counts[5] = {a.images,a.lists,a.parts,a.ranges,a.frames};
    const size_t strides[5] = {8,4,12,4,10};
    size_t end = 36;
    for (unsigned i = 0; i < 5; ++i) {
        a.section[i] = btn_read_le32(data+16+i*4);
        if (a.section[i] < end || !extent(size,a.section[i],counts[i],strides[i]))
            return SDL_SetError("Invalid/overlapping animation table %u",i);
        end = a.section[i]+counts[i]*strides[i];
    }
    a.consumed = end;
    if (!a.ranges || a.ranges > 256) return SDL_SetError("Unsupported animation range count");
    for (unsigned i = 0; i < a.images; ++i) {
        const uint8_t *image=data+a.section[0]+i*8;
        int x=signed16(image),y=signed16(image+2),w=signed16(image+4),h=signed16(image+6);
        if (x<0 || y<0 || w<=0 || h<=0 || x>=1024 || y>=512 || w>1024-x || h>512-y)
            return SDL_SetError("Invalid animation image rectangle %u",i);
    }
    for (unsigned i = 0; i < a.lists; ++i) {
        const uint8_t *p = data+a.section[1]+i*4;
        int count=signed16(p); unsigned offset=le16(p+2);
        if (count<0 || offset%12 || !extent((size_t)a.parts*12,offset,(size_t)count,12))
            return SDL_SetError("Invalid animation part list %u",i);
    }
    for (unsigned i = 0; i < a.parts; ++i) {
        const uint8_t *p=data+a.section[2]+i*12;
        if (le16(p)>=a.images || le16(p+2)>=a.images)
            return SDL_SetError("Animation part references missing image/palette");
    }
    for (unsigned i = 0; i < a.ranges; ++i) {
        const uint8_t *p=data+a.section[3]+i*4;
        unsigned count=le16(p), offset=le16(p+2);
        if (!count || count>256 || offset%10 || !extent((size_t)a.frames*10,offset,count,10))
            return SDL_SetError("Invalid animation frame range %u",i);
        for (unsigned n=0; n<count; ++n) {
            const uint8_t *frame=data+a.section[4]+offset+n*10;
            unsigned parts=le16(frame),control=frame[7];
            if ((parts<=0xFFFDu && parts>=a.lists)
                || (control && control!=255 && (control&127u)>=count))
                return SDL_SetError("Invalid animation frame reference/control");
        }
    }
    *out=a; return true;
}
bool btn_animation_read(BtnArchive *archive, unsigned member, uint32_t offset,
    uint8_t **owned_data, BtnAnimation *out)
{
    uint8_t header[36];
    uint32_t absolute;
    if (!archive || !owned_data || !out
        || !btn_archive_read(archive,member,offset,sizeof(header),header,sizeof(header))) return false;
    if (btn_read_le32(header)!=14) return SDL_SetError("Selected member/offset is not animation 0E");
    uint64_t end=(uint64_t)btn_read_le32(header+32)+(uint64_t)btn_read_le32(header+12)*10;
    if (end<36 || end>16u*1024u*1024u) return SDL_SetError("Animation exceeds viewer's 16 MiB limit");
    if (!btn_archive_range(&archive->index,member,offset,(uint32_t)end,&absolute)) return false;
    uint8_t *data=malloc((size_t)end);
    if (!data) return SDL_SetError("Cannot allocate selected animation");
    BtnAnimation parsed;
    if (!btn_archive_read(archive,member,offset,(uint32_t)end,data,(size_t)end)
        || !btn_animation_parse(&parsed,data,(size_t)end)) { free(data); return false; }
    *owned_data=data; *out=parsed; return true;
}
static const uint8_t *get_frame(const BtnAnimation *a, const BtnAnimationCursor *c, unsigned *count)
{
    if (!a || !c || c->range>=a->ranges) { SDL_SetError("Animation range out of bounds"); return NULL; }
    const uint8_t *range=a->data+a->section[3]+c->range*4;
    *count=le16(range);
    if (c->frame>=*count) { SDL_SetError("Animation frame out of bounds"); return NULL; }
    return a->data+a->section[4]+le16(range+2)+c->frame*10;
}
bool btn_animation_tick(const BtnAnimation *a, BtnAnimationCursor *c)
{
    unsigned count;
    const uint8_t *frame=get_frame(a,c,&count);
    if (!frame) return false;
    if (c->paused) return true;
    c->ended=c->jumped=false;
    unsigned ticks=((unsigned)frame[6]+1u)/2u;
    if (++c->ticks>=ticks) {
        c->ticks=0; ++c->frame;
        unsigned control=frame[7];
        if (c->frame>=count) {
            c->ended=true;
            if (!(control&128u)) { c->frame=control&127u; c->ended=false; c->jumped=true; }
        }
        if (control==255) { c->ticks=ticks; --c->frame; c->ended=true; }
        else if (control&128u) { c->frame=control&127u; c->ended=false; c->jumped=true; }
    }
    return true;
}
bool btn_animation_draw(const BtnAnimation *a, const BtnAnimationCursor *c,
    BtnDrawList *list, unsigned bucket, int x, int y)
{
    unsigned frames;
    const uint8_t *frame=get_frame(a,c,&frames);
    if (!frame) return false;
    unsigned parts=le16(frame);
    if (parts>0xFFFDu) return true;
    const uint8_t *entry=a->data+a->section[1]+parts*4;
    unsigned count=le16(entry), offset=le16(entry+2);
    if (!list || bucket>=BTN_OT_BUCKETS || list->count>BTN_DRAW_CAPACITY
        || count>BTN_DRAW_CAPACITY-list->count) return SDL_SetError("Animation exceeds draw list capacity");
    if (x < -8192 || x > 8192 || y < -8192 || y > 8192)
        return SDL_SetError("Animation anchor exceeds viewer range");
    for (unsigned n=count; n-- > 0;) {
        const uint8_t *part=a->data+a->section[2]+offset+n*12;
        const uint8_t *image=a->data+a->section[0]+le16(part)*8;
        const uint8_t *palette=a->data+a->section[0]+le16(part+2)*8;
        unsigned attr=part[8], depth=attr>>7, scale=depth ? 2u : 4u;
        int ix=signed16(image),iy=signed16(image+2);
        int w=signed16(image+4)*(int)scale,h=signed16(image+6);
        BtnPrimitive p={0};
        p.type=BTN_RECTANGLE; p.w=w; p.h=h;
        p.v[0]=(BtnVertex){x+signed16(frame+2)+signed16(part+4),y+signed16(frame+4)+signed16(part+6),
            (ix&63)*(int)scale,(iy&255),128,128,128};
        p.material.textured=true;
        p.material.semi=(attr&16u)!=0;
        p.material.flip_x=(attr&1u)!=0; p.material.flip_y=(attr&2u)!=0;
        if (p.material.flip_x) p.v[0].u+=w-1;
        if (p.material.flip_y) p.v[0].v+=h-1;
        p.material.page=(uint16_t)((unsigned)(ix/64)|((unsigned)(iy/256)<<4)|(depth<<7));
        p.material.clut=(uint16_t)((unsigned)signed16(palette)/16u|((unsigned)signed16(palette+2)<<6));
        if (!btn_draw_list_add(list,bucket,&p)) return false;
    }
    return true;
}

bool btn_text_draw(BtnDrawList *list, unsigned bucket, const uint16_t *words,
    size_t count, int x, int y, size_t *consumed)
{
    if (!list || !words || x < -8192 || x > 8192 || y < -8192 || y > 8192)
        return SDL_SetError("Invalid text input/anchor");
    int start=x;
    for (size_t i=0; i<count; ++i) {
        uint16_t code=words[i];
        if (code==0xFFFFu) { if (consumed) *consumed=i+1; return true; }
        if (code==0xFFFEu) { x=start; y+=12; }
        else if (code==0xFFDu) x+=8;
        else if (code<=255u) {
            BtnPrimitive p={0};
            p.type=BTN_RECTANGLE; p.w=8; p.h=12;
            p.v[0]=(BtnVertex){x,y,(code&15)*8,((code>>4)&15)*12,128,128,128};
            p.material.textured=true; p.material.page=14; p.material.clut=(uint16_t)((254u<<6)|56u);
            if (!btn_draw_list_add(list,bucket,&p)) return false;
            x+=8;
        } else return SDL_SetError("Unsupported text control 0x%04X at word %zu",(unsigned)code,i);
        if (x>16384 || y>16384) return SDL_SetError("Text exceeds viewer coordinate range");
    }
    return SDL_SetError("Text stream has no END within its supplied length");
}
