/* Included by src/gameplay.c; shares its private declarations. */
void ground_set_tolerance(s32 tolerance)
{
    groundTolerance = tolerance;
}
/* TENTATIVE: layout inferred only from ground_enable_objects's field offsets. */
typedef struct {
    u8 pad00[0x18];
    s32 flag;
} E340Entry;

typedef struct {
    u8 pad00[8];
    s32 count;
    E340Entry entries[1];
} E340List;

typedef struct {
    E340List *list;
} E340Ctx;

void ground_enable_objects(E340Ctx *model)
{
    s32 i;
    E340Entry *entry;

    for (i = 0; i < model->list->count; i++) {
        entry = &model->list->entries[i];
        entry->flag = 1;
    }
}
extern SVECTOR *groundTriangle[3];
extern s32 groundFound;
extern s32 groundPos;
extern void OuterProduct0(VECTOR *v0, VECTOR *v1, VECTOR *v2);

s32 ground_height_at(void)
{
    VECTOR normal;
    VECTOR origin;
    VECTOR term;
    VECTOR edge0;
    VECTOR edge1;
    SVECTOR **tri;
    VECTOR *pos;

    if (groundFound <= 0) {
        return 0;
    }
    tri = groundTriangle;
    origin.vx = tri[1]->vx;
    origin.vy = tri[1]->vy;
    origin.vz = tri[1]->vz;
    edge0.vx = origin.vx - tri[0]->vx;
    edge0.vy = origin.vy - tri[0]->vy;
    edge0.vz = origin.vz - tri[0]->vz;
    edge1.vx = tri[2]->vx - origin.vx;
    edge1.vy = tri[2]->vy - origin.vy;
    edge1.vz = tri[2]->vz - origin.vz;
    OuterProduct0(&edge0, &edge1, &normal);
    pos = (VECTOR *)groundPos;
    term.vx = (pos->vx - origin.vx) * normal.vx;
    term.vz = (pos->vz - origin.vz) * normal.vz;
    term.vy = -(term.vx + term.vz);
    if (normal.vy == 0) {
        return pos->vy;
    }
    return term.vy / normal.vy + origin.vy;
}
/* Ported from hmbtn-decomp (CC0), ground_find_object. */
extern s32 groundObjects[];
s32 ground_primitive_under(TmdObject *a, TmdObject *b);
s32 ground_xz_in_clip(s32 x, s32 z, s32 px, s32 pz);
s32 ground_xyz_in_clip(s32 *a, s32 *b);
s32 ground_find_object(Model *model, VECTOR *pos)
{
    TmdHeader *tmd;
    ModelCenters *centers;
    s32 count;
    s32 found;
    s32 hits;
    s32 i;
    s32 slot;
    s32 k;
    s32 coord;
    VECTOR here;
    VECTOR center;

    groundHitPrimPtr = 0;
    groundFound = 0;
    for (k = 0; k < 8; k++) {
        groundObjects[k] = -1;
    }
    found = -1;
    centers = model->centers;
    tmd = model->tmd;
    groundPos = (s32)pos;
    groundClip = centers->clip;
    count = tmd->nObj;
    for (i = 0; i < count; i++) {
        if (tmd->objects[i].scale != 0) {
            if (ground_xz_in_clip(centers->coords[i * 4], centers->coords[i * 4 + 2], pos->vx,
                                  pos->vz) != 0 &&
                ground_primitive_under(tmd->objects, &tmd->objects[i]) == 0) {
                found = i;
                break;
            }
        }
    }
    if (found < 0) {
        return 0;
    }
    hits = 0;
    here.vx = pos->vx;
    here.vy = ground_height_at();
    here.vz = pos->vz;
    for (i = 0, coord = 0; i < count; i++, coord += 4) {
        if (tmd->objects[i].scale != 0) {
            center.vx = centers->coords[coord];
            center.vy = centers->coords[coord + 1];
            center.vz = centers->coords[coord + 2];
            if (ground_xyz_in_clip(&here.vx, &center.vx) != 0) {
                groundObjects[hits] = i;
                hits++;
                if (hits >= 8) {
                    break;
                }
            }
        }
    }
    for (slot = 0; slot < 8; slot++) {
        if (found == groundObjects[slot]) {
            groundObjects[slot] = groundObjects[0];
            groundObjects[0] = found;
            break;
        }
    }
    return 1;
}

extern s32 groundObjects[];
extern s32 groundPos;
s32 ground_primitive_under(TmdObject *a, TmdObject *b);

s32 ground_listed_object_hit(u8 **obj, s32 pos)
{
    u8 *base = *obj;
    s32 i;

    groundHitPrimPtr = 0;
    groundPos = pos;
    for (i = 0; i < 8; i++) {
        if (groundObjects[i] >= 0) {
            if (ground_primitive_under(base + 0xC, base + (groundObjects[i] * 28 + 0xC)) == 0) {
                return 1;
            }
        }
    }
    return 0;
}
extern s32 groundClip;
s32 ground_xz_in_clip(s32 x, s32 z, s32 px, s32 pz)
{
    s32 savedZ = z;
    s32 negative;
    z = (groundClip >> 1) + 600;
    negative = -z;
    x -= px;
    if (negative < x && x < z)
        goto check_z;
zero:
    return 0;
check_z:
    savedZ -= pz;
    if (negative >= savedZ)
        goto zero;
    if (savedZ >= z)
        goto zero;
    return 1;
}
s32 ground_xyz_in_clip(s32 *a, s32 *b)
{
    s32 range = (groundClip >> 1) + 600;
    s32 d;

    d = *a++ - *b++;
    if (d < -range || range < d) {
        return 0;
    }
    d = *a++ - *b++;
    if (d < -range || range < d) {
        return 0;
    }
    d = *a - *b;
    if (d < -range || range < d) {
        return 0;
    }
    return 1;
}
/* Ported from hmbtn-decomp (CC0), ground_primitive_under. ground_triangle_under and
 * ground_quad_under are its GNU C nested functions (ground_triangle_under and
 * ground_quad_under): their static chain arrives in $v0, which is the "private
 * register convention" earlier rounds took for hand-written code. */
#define PRIM_HEADER(p)        (*(u32 *)(p))
#define PRIM_INDEX(p, offset) (*(u16 *)((p) + (offset)))
#define TMD_PRIM_CODE_MASK    0xFDFF0000
extern SVECTOR *groundVertices;
extern s32 groundCount;
extern u8 *groundPrimitives;
extern s32 groundMask;
extern s32 groundEdge0;
extern s32 groundEdge1;
extern s32 groundEdge2;
extern s32 groundPosX;
extern s32 groundPosZ;
extern s32 groundDy;
s32 ground_primitive_under(TmdObject *table, TmdObject *object)
{
    s32 count;
    SVECTOR *vertices;
    u8 *primitives;

    s32 ground_triangle_under(SVECTOR * a, SVECTOR * b, SVECTOR * c)
    {
        VECTOR *pos;

        pos = (VECTOR *)groundPos;
        groundDy = (a->vy + b->vy + c->vy) / 3;
        groundDy = pos->vy - groundDy;
        if (groundDy > groundTolerance || groundDy < -groundTolerance) {
            return 1;
        }
        groundPosX = pos->vx;
        groundPosZ = pos->vz;
        groundEdge0 =
            (b->vx - a->vx) * (groundPosZ - a->vz) - (b->vz - a->vz) * (groundPosX - a->vx);
        groundEdge1 =
            (c->vx - b->vx) * (groundPosZ - b->vz) - (c->vz - b->vz) * (groundPosX - b->vx);
        groundEdge2 =
            (a->vx - c->vx) * (groundPosZ - c->vz) - (a->vz - c->vz) * (groundPosX - c->vx);
        if ((groundEdge0 > 0 || groundEdge1 > 0 || groundEdge2 > 0) &&
            (groundEdge0 < 0 || groundEdge1 < 0 || groundEdge2 < 0)) {
            return 1;
        }
        groundFound = 1;
        groundTriangle[0] = a;
        groundTriangle[1] = b;
        groundTriangle[2] = c;
        return 0;
    }

    s32 ground_quad_under(s32 a, s32 b, s32 c, s32 d)
    {
        if (a == b || a == c) {
            return ground_triangle_under(&groundVertices[c], &groundVertices[b],
                                         &groundVertices[d]);
        }
        if (d == b || d == c) {
            return ground_triangle_under(&groundVertices[a], &groundVertices[b],
                                         &groundVertices[c]);
        }
        if (ground_triangle_under(&groundVertices[a], &groundVertices[b], &groundVertices[c]) ==
            0) {
            return 0;
        }
        return ground_triangle_under(&groundVertices[d], &groundVertices[c], &groundVertices[b]);
    }

    count = object->nPrimitive;
    /* TMD tables hold offsets from the model base. */
    vertices = (SVECTOR *)((u8 *)table + object->vertTop);
    primitives = (u8 *)table + object->primitiveTop;
    /* Walk the object's primitives, grouped by type in this fixed order, until
     * one under *groundPos is found (return 0, groundHitPrimPtr = that primitive).
     * Primitives whose header has a groundFlagBits bit set are skipped. */
    groundHitPrimPtr = 0;
    groundCount = count;
    groundVertices = vertices;
    groundPrimitives = primitives;
    groundMask = groundFlagBits;

#define TRIANGLES(label, code, size, first)                                                        \
    label:                                                                                         \
    if (groundCount == 0) {                                                                        \
        return 1;                                                                                  \
    }                                                                                              \
    if ((PRIM_HEADER(groundPrimitives) & TMD_PRIM_CODE_MASK) != code) {                            \
        goto label##_done;                                                                         \
    }                                                                                              \
    if (!(PRIM_HEADER(groundPrimitives) & groundMask) &&                                           \
        ground_triangle_under(&groundVertices[PRIM_INDEX(groundPrimitives, first)],                \
                              &groundVertices[PRIM_INDEX(groundPrimitives, first + 2)],            \
                              &groundVertices[PRIM_INDEX(groundPrimitives, first + 4)]) == 0) {    \
        goto hit;                                                                                  \
    }                                                                                              \
    groundPrimitives += size;                                                                      \
    groundCount--;                                                                                 \
    goto label;                                                                                    \
    label##_done:

#define QUADS(label, code, size, first)                                                            \
    label:                                                                                         \
    if (groundCount == 0) {                                                                        \
        return 1;                                                                                  \
    }                                                                                              \
    if ((PRIM_HEADER(groundPrimitives) & TMD_PRIM_CODE_MASK) != code) {                            \
        goto label##_done;                                                                         \
    }                                                                                              \
    if (!(PRIM_HEADER(groundPrimitives) & groundMask) &&                                           \
        ground_quad_under(PRIM_INDEX(groundPrimitives, first),                                     \
                          PRIM_INDEX(groundPrimitives, first + 2),                                 \
                          PRIM_INDEX(groundPrimitives, first + 4),                                 \
                          PRIM_INDEX(groundPrimitives, first + 6)) == 0) {                         \
        goto hit;                                                                                  \
    }                                                                                              \
    groundPrimitives += size;                                                                      \
    groundCount--;                                                                                 \
    goto label;                                                                                    \
    label##_done:

    TRIANGLES(f3, 0x2101 << 16, 0x10, 0x08)
    TRIANGLES(g3, 0x3101 << 16, 0x18, 0x10)
    TRIANGLES(ft3, 0x2501 << 16, 0x1C, 0x14)
    TRIANGLES(gt3, 0x3501 << 16, 0x24, 0x1C)
    QUADS(f4, 0x2901 << 16, 0x10, 0x08)
    QUADS(g4, 0x3901 << 16, 0x1C, 0x14)
    QUADS(ft4, 0x2D01 << 16, 0x20, 0x18)
    QUADS(gt4, 0x3D01 << 16, 0x2C, 0x24)
    return 1;

hit:
    groundHitPrimPtr = (s32)groundPrimitives;
    return 0;
}

void ground_set_skip_mask(s32 flags)
{
    groundFlagBits = flags;
}

s32 ground_hit_primitive(void)
{
    return groundHitPrimPtr;
}
extern s32 cameraFocusIndex, cameraUnused8005E30C, cameraUnused8005E310, cameraUnused8005E314;
extern u8 cameraReady;
extern s32 cameraEyeCopy[];
void vec_from_polar(VECTOR *arg0, s32 arg1, s32 arg2, s32 arg3);
