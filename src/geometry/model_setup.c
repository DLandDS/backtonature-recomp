/* Included by src/state_core.c; shares its private declarations. */
void camera_init_projection(void)
{
    SetGeomOffset(160, 120);
    SetGeomScreen(1000);
    camera_set_eye_roll(0, 0, 0x1000, 0x800);
    camera_set_target(0, 0, -0x1000);
    memset(&lightMatrix, 0, sizeof(MATRIX));
    memset(&colorMatrix, 0, sizeof(MATRIX));
    memset(&baseMatrix, 0, sizeof(MATRIX));
    baseMatrix.m[2][2] = 0x1000;
    baseMatrix.m[1][1] = 0x1000;
    baseMatrix.m[0][0] = 0x1000;
}
extern void camera_build_view(void);

void model_load_lighting(void)
{
    camera_build_view();
    SetFogNear(fogNear, 1000);
    gte_SetBackColor(backColor, storage8005E2DD, storage8005E2DE);
    SetFarColor(farColor, storage8005E2E1, storage8005E2E2);
    gte_SetColorMatrix(&colorMatrix);
    gte_SetLightMatrix(&lightMatrix);
}
extern MATRIX storage80061E08m __asm__("D_80061E08");
extern MATRIX GsIDMATRIX;
extern void VectorNormalS(VECTOR *v0, SVECTOR *v1);
extern MATRIX *RotMatrixZ(s32 r, MATRIX *m);
extern VECTOR *ApplyMatrixLV(MATRIX *m, VECTOR *v0, VECTOR *v1);

void camera_build_view(void)
{
    VECTOR v;
    s32 a[4];
    s32 b[4];
    MATRIX rm;
    s32 i;

    if (modelCamera.unk1C == 0) {
        v.vx = modelCamera.unk0C - modelCamera.unk00;
        v.vy = modelCamera.unk10 - modelCamera.unk04;
        v.vz = modelCamera.unk14 - modelCamera.unk08;
        VectorNormalS(&v, (SVECTOR *)storage80061E08m.m[2]);
        v.vy = 0;
        if (storage80061E08m.m[2][0] == 0 && storage80061E08m.m[2][2] == 0) {
            v.vx = 0x1000;
            v.vz = 0;
        } else {
            v.vx = storage80061E08m.m[2][2];
            v.vz = -storage80061E08m.m[2][0];
        }
        VectorNormalS(&v, (SVECTOR *)storage80061E08m.m[0]);
        for (i = 0; i < 3; i++) {
            a[i] = storage80061E08m.m[2][i];
            b[i] = storage80061E08m.m[0][i];
        }
        gte_OuterProduct12(a, b, &v);
        VectorNormalS(&v, (SVECTOR *)storage80061E08m.m[1]);
        if (modelCamera.unk18 != 0) {
            rm = GsIDMATRIX;
            RotMatrixZ(modelCamera.unk18, &rm);
            gte_MulMatrix0(&rm, &storage80061E08m, &storage80061E08m);
        }
        v.vx = -modelCamera.unk00;
        v.vy = -modelCamera.unk04;
        v.vz = -modelCamera.unk08;
        ApplyMatrixLV(&storage80061E08m, &v, (VECTOR *)storage80061E08m.t);
    }
    gte_SetRotMatrix(&storage80061E08m);
    gte_SetTransMatrix(&storage80061E08m);
}
void camera_set_eye_roll(s32 a, s32 b, s32 c, s32 d)
{
    modelCamera.unk00 = a;
    modelCamera.unk04 = b;
    modelCamera.unk08 = c;
    modelCamera.unk18 = d;
    modelCamera.unk1C = 0;
}

void camera_get_eye(s32 *a, s32 *b, s32 *c)
{
    *a = modelCamera.unk00;
    *b = modelCamera.unk04;
    *c = modelCamera.unk08;
}

void camera_set_target(s32 a, s32 b, s32 c)
{
    modelCamera.unk0C = a;
    modelCamera.unk10 = b;
    modelCamera.unk14 = c;
    modelCamera.unk1C = 0;
}
void camera_lock_view(Block20 *src)
{
    viewMatrix = *src;
    storage80061E04[0] = src;
}
