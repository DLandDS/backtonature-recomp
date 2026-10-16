/* Included by src/gameplay.c; shares its private declarations. */
void camera_init(void)
{
    camera_set_view(800, 600, 0);
    cameraFocusIndex = -1;
    cameraUnused8005E30C = 0;
    cameraUnused8005E310 = 0;
    cameraUnused8005E314 = 0;
    cameraDirty = 0;
    cameraTarget[2] = 0;
    cameraTarget[1] = 0;
    cameraTarget[0] = 0;
    vec_from_polar((VECTOR *)cameraEye, cameraViewX, cameraViewY, cameraViewZ);
    vec3_copy(cameraEyeCopy, cameraEye);
    cameraReady = 1;
    cameraMode = 2;
}
void camera_set_view(s32 x, s32 y, s32 z)
{
    if (x != cameraViewX || y != cameraViewY || z != cameraViewZ) {
        cameraDirty = 1;
        cameraViewSettled = 0;
    }
    cameraViewX = x;
    cameraViewY = y;
    cameraViewZ = z;
}
void camera_get_view_args(s32 *x, s32 *y, s32 *angle)
{
    s32 vector[3];

    if (x != 0) {
        *x = cameraViewX;
    }
    if (y != 0) {
        *y = cameraViewY;
    }
    if (angle != 0) {
        vec3_copy(vector, cameraEye);
        vec3_sub(vector, cameraTarget);
        *angle = ratan2(vector[2], vector[0]);
    }
}
void camera_refresh_block(s32 *src)
{
    if (cameraMode == 2) {
        if (src[0] != cameraBlock[0] || src[1] != cameraBlock[1] || src[2] != cameraBlock[2]) {
            if (cameraBlockMode != 1) {
                vec3_copy(cameraBlock, src);
                cameraDirty = 1;
                cameraViewSettled = 0;
            }
        }
    }
}
void camera_get_target(s32 *out)
{
    vec3_copy(out, cameraTarget);
}
void camera_deactivate(void)
{
    cameraMode = 0;
}
void camera_set_mode(s32 mode)
{
    if (cameraModeArg != mode) {
        cameraDirty = 1;
    }
    cameraModeArg = mode;
}
extern int rcos(int a);
extern int rsin(int a);
