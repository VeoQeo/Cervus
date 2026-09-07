#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/syscall.h>

#define MODE_OP_LIST 0
#define MODE_OP_SET  1

typedef struct {
    uint32_t count;
    struct { uint32_t w, h; } modes[24];
    uint32_t cur_w, cur_h;
} mode_list_t;

static const char USAGE[] =
    "Usage: mode                 list the modes this adapter offers\n"
    "       mode <W>x<H>         switch to that resolution\n"
    "       mode <W> <H>         same, written apart\n"
    "\nThe console reflows to the new size; open full-screen programs\n"
    "should be restarted.\n";

static void explain_no_adapter(void) {
    fprintf(stderr,
        "mode: this display has no runtime mode setting.\n"
        "\n"
        "Resolution is fixed by the bootloader at startup. Changing it\n"
        "afterwards needs a driver for the graphics chip, and only the\n"
        "Bochs adapter used by QEMU, Bochs and VirtualBox is supported.\n"
        "\n"
        "On real hardware, set the resolution in the bootloader instead.\n");
}

int main(int argc, char **argv) {
    mode_list_t ml;
    long rc = syscall3(SYS_FB_SETMODE, MODE_OP_LIST, (long)&ml, 0);

    if (argc < 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        if (argc >= 2) { fputs(USAGE, stdout); return 0; }
        if (rc != 0) { explain_no_adapter(); return 1; }
        printf("current: %ux%u\n\navailable:\n", ml.cur_w, ml.cur_h);
        for (uint32_t i = 0; i < ml.count; i++) {
            int cur = (ml.modes[i].w == ml.cur_w && ml.modes[i].h == ml.cur_h);
            printf("  %c %ux%u\n", cur ? '*' : ' ', ml.modes[i].w, ml.modes[i].h);
        }
        printf("\nrun 'mode 1280x800' to switch\n");
        return 0;
    }

    if (rc != 0) { explain_no_adapter(); return 1; }

    unsigned w = 0, h = 0;
    if (argc >= 3) {
        w = (unsigned)atoi(argv[1]);
        h = (unsigned)atoi(argv[2]);
    } else {
        const char *x = strchr(argv[1], 'x');
        if (!x) x = strchr(argv[1], 'X');
        if (!x) { fputs(USAGE, stderr); return 1; }
        w = (unsigned)atoi(argv[1]);
        h = (unsigned)atoi(x + 1);
    }
    if (w == 0 || h == 0) { fputs(USAGE, stderr); return 1; }

    if (syscall3(SYS_FB_SETMODE, MODE_OP_SET, w, h) != 0) {
        fprintf(stderr, "mode: %ux%u was refused; run 'mode' for the list\n", w, h);
        return 1;
    }
    return 0;
}
