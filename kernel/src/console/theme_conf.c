#include "../../include/console/console.h"
#include "../../include/fs/vfs.h"
#include "../../include/io/serial.h"
#include <string.h>
#include <stdlib.h>

typedef struct { const char *name; uint32_t pal[16]; uint32_t fg, bg; } builtin_theme_t;

static const builtin_theme_t BUILTIN[] = {
{ "classic", { 0x000000,0xAA0000,0x00AA00,0xAA5500,0x0000AA,0xAA00AA,0x00AAAA,0xAAAAAA,
               0x555555,0xFF5555,0x55FF55,0xFFFF55,0x5555FF,0xFF55FF,0x55FFFF,0xFFFFFF },
  0xFFFFFF, 0x000000 },
{ "cervus",  { 0x2E3440,0xBF616A,0xA3BE8C,0xEBCB8B,0x81A1C1,0xB48EAD,0x88C0D0,0xD8DEE9,
               0x4C566A,0xD08770,0xB9D4A0,0xF0D399,0x9BB8DA,0xC7A4C0,0xA3D4E0,0xECEFF4 },
  0xD8DEE9, 0x21252E },
{ "gruv",    { 0x282828,0xCC241D,0x98971A,0xD79921,0x458588,0xB16286,0x689D6A,0xA89984,
               0x928374,0xFB4934,0xB8BB26,0xFABD2F,0x83A598,0xD3869B,0x8EC07C,0xEBDBB2 },
  0xEBDBB2, 0x282828 },
{ "solar",   { 0x073642,0xDC322F,0x859900,0xB58900,0x268BD2,0xD33682,0x2AA198,0xEEE8D5,
               0x586E75,0xCB4B16,0x93A1A1,0x657B83,0x839496,0x6C71C4,0x93A1A1,0xFDF6E3 },
  0x93A1A1, 0x002B36 },
{ "paper",   { 0xEEEEEC,0xA40000,0x4E9A06,0xC4A000,0x3465A4,0x75507B,0x06989A,0x2E3436,
               0xD3D7CF,0xCC0000,0x73D216,0xEDD400,0x729FCF,0xAD7FA8,0x34E2E2,0x000000 },
  0x2E3436, 0xF2F1EC },
{ "matrix",  { 0x001B00,0x2E7D32,0x00E676,0x00C853,0x1B5E20,0x33691E,0x00BFA5,0x69F0AE,
               0x0A3D0A,0x4CAF50,0x76FF03,0x64DD17,0x2E7D32,0x00E676,0x1DE9B6,0xB9F6CA },
  0x00E676, 0x000A00 },
{ "dracula", { 0x282A36,0xFF5555,0x50FA7B,0xF1FA8C,0xBD93F9,0xFF79C6,0x8BE9FD,0xF8F8F2,
               0x6272A4,0xFF6E6E,0x69FF94,0xFFFFA5,0xD6ACFF,0xFF92DF,0xA4FFFF,0xFFFFFF },
  0xF8F8F2, 0x282A36 },
{ "catppuccin", { 0x45475A,0xF38BA8,0xA6E3A1,0xF9E2AF,0x89B4FA,0xF5C2E7,0x94E2D5,0xBAC2DE,
                  0x585B70,0xF37799,0xB2E8AD,0xFAE7BE,0x9CC0FB,0xF7CEEB,0xA5E7DC,0xA6ADC8 },
  0xCDD6F4, 0x1E1E2E },
{ "latte",   { 0x5C5F77,0xD20F39,0x40A02B,0xDF8E1D,0x1E66F5,0xEA76CB,0x179299,0xACB0BE,
               0x6C6F85,0xD52A44,0x49AF3D,0xE0913C,0x456EFF,0xEC83D0,0x2D9FA8,0xBCC0CC },
  0x4C4F69, 0xEFF1F5 },
{ "nord",    { 0x3B4252,0xBF616A,0xA3BE8C,0xEBCB8B,0x81A1C1,0xB48EAD,0x88C0D0,0xE5E9F0,
               0x4C566A,0xBF616A,0xA3BE8C,0xEBCB8B,0x81A1C1,0xB48EAD,0x8FBCBB,0xECEFF4 },
  0xD8DEE9, 0x2E3440 },
{ "rose",    { 0x26233A,0xEB6F92,0x9CCFD8,0xF6C177,0x31748F,0xC4A7E7,0xEBBCBA,0xE0DEF4,
               0x6E6A86,0xEB6F92,0x9CCFD8,0xF6C177,0x31748F,0xC4A7E7,0xEBBCBA,0xE0DEF4 },
  0xE0DEF4, 0x191724 },
{ "sand",    { 0x5B5147,0xC96F5B,0x7D9C6B,0xD9A05B,0x6B8CA3,0xA88BA3,0x77A6A0,0xD8CFC2,
               0x7A6E62,0xE08A72,0x95B884,0xEFBC78,0x86A6BE,0xC3A5BE,0x93C0BA,0xF2EAE0 },
  0xE8DFD2, 0x2B2622 },
{ "mint",    { 0x2F3B36,0xE8837E,0x8FD9A8,0xEBD292,0x7FB2CE,0xC4A0D6,0x8FD6D2,0xDCE8E2,
               0x46564F,0xF29A95,0xA8E8BE,0xF5E2AC,0x9CC8E0,0xD6B8E5,0xAAE5E2,0xF0F7F3 },
  0xDCE8E2, 0x1F2A26 },
{ "amber",   { 0x2A1B00,0xB33A00,0xC98A00,0xFFB000,0x8A5A00,0xB36A00,0xD9A400,0xE8C070,
               0x4A3200,0xFF6A00,0xFFC947,0xFFD966,0xC98A00,0xFF9E3D,0xFFDD8A,0xFFF0C4 },
  0xFFB000, 0x1A1000 },
{ "mono",    { 0x101010,0x8A8A8A,0xB0B0B0,0xC8C8C8,0x707070,0x989898,0xC0C0C0,0xD8D8D8,
               0x505050,0xA8A8A8,0xCFCFCF,0xE0E0E0,0x909090,0xB8B8B8,0xDCDCDC,0xF4F4F4 },
  0xCFCFCF, 0x141414 },
};
#define N_BUILTIN ((int)(sizeof BUILTIN / sizeof BUILTIN[0]))

static uint32_t parse_hex(const char *s) {
    uint32_t v = 0;
    if (*s == '#') s++;
    for (; *s; s++) {
        int d;
        if      (*s >= '0' && *s <= '9') d = *s - '0';
        else if (*s >= 'a' && *s <= 'f') d = *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'F') d = *s - 'A' + 10;
        else break;
        v = (v << 4) | (uint32_t)d;
    }
    return v;
}

void console_theme_load_config(void) {
    static const char *paths[] = { "/mnt/etc/console.conf", "/etc/console.conf" };
    char buf[256];
    int got = 0;

    for (int p = 0; p < 2 && !got; p++) {
        vfs_file_t *f = NULL;
        if (vfs_open(paths[p], O_RDONLY, 0, &f) < 0 || !f) continue;
        int64_t n = vfs_read(f, buf, sizeof buf - 1);
        vfs_close(f);
        if (n <= 0) continue;
        buf[n] = 0;
        got = 1;
    }
    if (!got) return;

    const builtin_theme_t *pick = NULL;
    uint32_t fg = 0, bg = 0;
    int have_fg = 0, have_bg = 0;

    char *line = buf;
    while (line && *line) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = 0;
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = 0;
            const char *v = eq + 1;
            if (!strcmp(line, "theme")) {
                for (int i = 0; i < N_BUILTIN; i++)
                    if (!strcmp(BUILTIN[i].name, v)) { pick = &BUILTIN[i]; break; }
            } else if (!strcmp(line, "fg")) { fg = parse_hex(v); have_fg = 1; }
            else if (!strcmp(line, "bg")) { bg = parse_hex(v); have_bg = 1; }
        }
        line = nl ? nl + 1 : NULL;
    }

    if (!pick && !have_fg && !have_bg) return;

    uint32_t pal[16], cur_fg, cur_bg;
    console_get_theme(pal, &cur_fg, &cur_bg);
    if (pick) {
        for (int i = 0; i < 16; i++) pal[i] = pick->pal[i];
        cur_fg = pick->fg;
        cur_bg = pick->bg;
    }
    if (have_fg) cur_fg = fg;
    if (have_bg) cur_bg = bg;

    uint32_t old_pal[16], old_fg, old_bg;
    console_get_theme(old_pal, &old_fg, &old_bg);
    console_set_theme(pal, cur_fg, cur_bg);
    vt_theme_changed(old_pal, old_fg, old_bg);
    serial_printf("[console] theme from %s applied\n",
                  pick ? pick->name : "console.conf");
}
