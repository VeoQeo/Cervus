#include "../../../include/syscall/syscall_internal.h"
#include "../../../include/drivers/video/bga.h"
#include "../../../include/graphics/fb/fb.h"
#include "../../../include/console/console.h"
#include <string.h>

extern fb_info_t *global_framebuffer;

#define MODE_OP_LIST 0
#define MODE_OP_SET  1

typedef struct {
    uint32_t count;
    struct { uint32_t w, h; } modes[24];
    uint32_t cur_w, cur_h;
} mode_list_t;

int64_t sys_fb_setmode(uint64_t op, uint64_t a, uint64_t b) {
    if (op == MODE_OP_LIST) {
        if (!a) return -EINVAL;
        if (!syscall_uptr_validate((void *)a, sizeof(mode_list_t))) return -EFAULT;

        mode_list_t out;
        memset(&out, 0, sizeof out);
        out.cur_w = global_framebuffer ? (uint32_t)global_framebuffer->width : 0;
        out.cur_h = global_framebuffer ? (uint32_t)global_framebuffer->height : 0;

        bga_mode_t modes[24];
        int n = bga_list_modes(modes, 24);
        if (n > 24) n = 24;
        for (int i = 0; i < n; i++) {
            out.modes[i].w = modes[i].width;
            out.modes[i].h = modes[i].height;
        }
        out.count = (uint32_t)n;
        memcpy((void *)a, &out, sizeof out);
        return bga_present() ? 0 : -ENODEV;
    }

    if (op != MODE_OP_SET) return -EINVAL;
    if (!bga_present()) return -ENODEV;
    if (bga_set_mode((uint32_t)a, (uint32_t)b) != 0) return -EINVAL;

    fb_resize_backbuffer(global_framebuffer);
    vt_font_changed();
    return 0;
}
