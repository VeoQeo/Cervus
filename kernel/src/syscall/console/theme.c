#include "../../../include/syscall/syscall_internal.h"
#include "../../../include/console/console.h"
#include <string.h>

int64_t sys_console_theme(uint64_t uptr) {
    if (!uptr) return -EINVAL;
    if (!syscall_uptr_validate((void *)uptr, sizeof(console_theme_t))) return -EFAULT;

    console_theme_t t;
    memcpy(&t, (const void *)uptr, sizeof t);

    uint32_t old_pal[16], old_fg, old_bg;
    console_get_theme(old_pal, &old_fg, &old_bg);

    console_set_theme(t.palette, t.fg, t.bg);
    vt_theme_changed(old_pal, old_fg, old_bg);
    return 0;
}
