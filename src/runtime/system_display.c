/* Included by src/display.c; shares its private declarations. */
void display_on(void)
{
    SetDispMask(1);
}

void display_off(void)
{
    SetDispMask(0);
}
