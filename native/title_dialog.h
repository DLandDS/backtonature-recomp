#ifndef BTN_NATIVE_TITLE_DIALOG_H
#define BTN_NATIVE_TITLE_DIALOG_H

#include "graphics.h"
#include "title_setup.h"

typedef enum {
    BTN_DIALOG_NONE, BTN_DIALOG_NAME_HELP, BTN_DIALOG_BIRTHDAY_HELP,
    BTN_DIALOG_NAME_CONFIRM, BTN_DIALOG_BIRTHDAY_CONFIRM, BTN_DIALOG_REVIEW, BTN_DIALOG_SPEECH
} BtnDialogKind;
typedef enum {
    BTN_DIALOG_HIDDEN, BTN_DIALOG_OPENING, BTN_DIALOG_READY, BTN_DIALOG_CLOSING
} BtnDialogPhase;
typedef struct {
    BtnDialogKind kind;
    BtnDialogPhase phase;
    unsigned frame;
    BtnRect box;
    bool visible;
} BtnTitleDialog;

void btn_dialog_reset(BtnTitleDialog *dialog);
BtnDialogKind btn_dialog_for_setup(const BtnTitleSetup *setup);
bool btn_dialog_ready(const BtnTitleDialog *dialog, BtnDialogKind kind);
/* menu_box_animate geometry: duration 12 opening, 6 closing, six width steps.
 * The source NO_SCROLL setup messages appear together once the box is ready.
 * Resource I/O latency and hardware raster parity are not modelled here. */
void btn_dialog_tick(BtnTitleDialog *dialog, BtnDialogKind requested);
int btn_dialog_setup_step(BtnTitleDialog *dialog, BtnTitleSetup *setup,
    const BtnNameLayout *layout, const BtnInput *input);

#endif
