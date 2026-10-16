/* Included by src/flags.c; shares its private declarations. */
int event_flag_get(u16 index)
{
    int word = index >> 5;

    return (gameState.flags[word] >> (index & 0x1F)) & 1;
}

void event_flag_set(u16 index, u16 value)
{
    int word = index >> 5;
    int bit = index & 0x1F;

    if (value) {
        gameState.flags[word] |= 1 << bit;
    } else {
        gameState.flags[word] &= ~(1 << bit);
    }
}
