#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/syscall.h>

#define THEME_CONF "/etc/console.conf"

typedef struct {
    uint32_t palette[16];
    uint32_t fg;
    uint32_t bg;
} theme_t;

typedef struct {
    const char *name;
    const char *about;
    theme_t     t;
} named_theme_t;

static const named_theme_t THEMES[] = {
{ "classic", "the original VGA palette", {{
    0x000000, 0xAA0000, 0x00AA00, 0xAA5500, 0x0000AA, 0xAA00AA, 0x00AAAA, 0xAAAAAA,
    0x555555, 0xFF5555, 0x55FF55, 0xFFFF55, 0x5555FF, 0xFF55FF, 0x55FFFF, 0xFFFFFF },
    0xFFFFFF, 0x000000 }},

{ "cervus", "soft slate, easy on the eyes", {{
    0x2E3440, 0xBF616A, 0xA3BE8C, 0xEBCB8B, 0x81A1C1, 0xB48EAD, 0x88C0D0, 0xD8DEE9,
    0x4C566A, 0xD08770, 0xB9D4A0, 0xF0D399, 0x9BB8DA, 0xC7A4C0, 0xA3D4E0, 0xECEFF4 },
    0xD8DEE9, 0x21252E }},

{ "gruv", "warm and low contrast", {{
    0x282828, 0xCC241D, 0x98971A, 0xD79921, 0x458588, 0xB16286, 0x689D6A, 0xA89984,
    0x928374, 0xFB4934, 0xB8BB26, 0xFABD2F, 0x83A598, 0xD3869B, 0x8EC07C, 0xEBDBB2 },
    0xEBDBB2, 0x282828 }},

{ "solar", "muted blue-grey, low glare", {{
    0x073642, 0xDC322F, 0x859900, 0xB58900, 0x268BD2, 0xD33682, 0x2AA198, 0xEEE8D5,
    0x586E75, 0xCB4B16, 0x93A1A1, 0x657B83, 0x839496, 0x6C71C4, 0x93A1A1, 0xFDF6E3 },
    0x93A1A1, 0x002B36 }},

{ "paper", "dark text on a light page", {{
    0xEEEEEC, 0xA40000, 0x4E9A06, 0xC4A000, 0x3465A4, 0x75507B, 0x06989A, 0x2E3436,
    0xD3D7CF, 0xCC0000, 0x73D216, 0xEDD400, 0x729FCF, 0xAD7FA8, 0x34E2E2, 0x000000 },
    0x2E3436, 0xF2F1EC }},

{ "matrix", "green on black, nothing else", {{
    0x001B00, 0x2E7D32, 0x00E676, 0x00C853, 0x1B5E20, 0x33691E, 0x00BFA5, 0x69F0AE,
    0x0A3D0A, 0x4CAF50, 0x76FF03, 0x64DD17, 0x2E7D32, 0x00E676, 0x1DE9B6, 0xB9F6CA },
    0x00E676, 0x000A00 }},

{ "dracula", "the usual purple night", {{
    0x282A36, 0xFF5555, 0x50FA7B, 0xF1FA8C, 0xBD93F9, 0xFF79C6, 0x8BE9FD, 0xF8F8F2,
    0x6272A4, 0xFF6E6E, 0x69FF94, 0xFFFFA5, 0xD6ACFF, 0xFF92DF, 0xA4FFFF, 0xFFFFFF },
    0xF8F8F2, 0x282A36 }},

{ "catppuccin", "mocha, warm and muted", {{
    0x45475A, 0xF38BA8, 0xA6E3A1, 0xF9E2AF, 0x89B4FA, 0xF5C2E7, 0x94E2D5, 0xBAC2DE,
    0x585B70, 0xF37799, 0xB2E8AD, 0xFAE7BE, 0x9CC0FB, 0xF7CEEB, 0xA5E7DC, 0xA6ADC8 },
    0xCDD6F4, 0x1E1E2E }},

{ "latte", "catppuccin in daylight", {{
    0x5C5F77, 0xD20F39, 0x40A02B, 0xDF8E1D, 0x1E66F5, 0xEA76CB, 0x179299, 0xACB0BE,
    0x6C6F85, 0xD52A44, 0x49AF3D, 0xE0913C, 0x456EFF, 0xEC83D0, 0x2D9FA8, 0xBCC0CC },
    0x4C4F69, 0xEFF1F5 }},

{ "nord", "cold blue, low contrast", {{
    0x3B4252, 0xBF616A, 0xA3BE8C, 0xEBCB8B, 0x81A1C1, 0xB48EAD, 0x88C0D0, 0xE5E9F0,
    0x4C566A, 0xBF616A, 0xA3BE8C, 0xEBCB8B, 0x81A1C1, 0xB48EAD, 0x8FBCBB, 0xECEFF4 },
    0xD8DEE9, 0x2E3440 }},

{ "rose", "dusty pink and mauve", {{
    0x26233A, 0xEB6F92, 0x9CCFD8, 0xF6C177, 0x31748F, 0xC4A7E7, 0xEBBCBA, 0xE0DEF4,
    0x6E6A86, 0xEB6F92, 0x9CCFD8, 0xF6C177, 0x31748F, 0xC4A7E7, 0xEBBCBA, 0xE0DEF4 },
    0xE0DEF4, 0x191724 }},

{ "sand", "pastel, paper and clay", {{
    0x5B5147, 0xC96F5B, 0x7D9C6B, 0xD9A05B, 0x6B8CA3, 0xA88BA3, 0x77A6A0, 0xD8CFC2,
    0x7A6E62, 0xE08A72, 0x95B884, 0xEFBC78, 0x86A6BE, 0xC3A5BE, 0x93C0BA, 0xF2EAE0 },
    0xE8DFD2, 0x2B2622 }},

{ "mint", "pastel green and teal", {{
    0x2F3B36, 0xE8837E, 0x8FD9A8, 0xEBD292, 0x7FB2CE, 0xC4A0D6, 0x8FD6D2, 0xDCE8E2,
    0x46564F, 0xF29A95, 0xA8E8BE, 0xF5E2AC, 0x9CC8E0, 0xD6B8E5, 0xAAE5E2, 0xF0F7F3 },
    0xDCE8E2, 0x1F2A26 }},

{ "amber", "old amber terminal", {{
    0x2A1B00, 0xB33A00, 0xC98A00, 0xFFB000, 0x8A5A00, 0xB36A00, 0xD9A400, 0xE8C070,
    0x4A3200, 0xFF6A00, 0xFFC947, 0xFFD966, 0xC98A00, 0xFF9E3D, 0xFFDD8A, 0xFFF0C4 },
    0xFFB000, 0x1A1000 }},

{ "mono", "grey scale only", {{
    0x101010, 0x8A8A8A, 0xB0B0B0, 0xC8C8C8, 0x707070, 0x989898, 0xC0C0C0, 0xD8D8D8,
    0x505050, 0xA8A8A8, 0xCFCFCF, 0xE0E0E0, 0x909090, 0xB8B8B8, 0xDCDCDC, 0xF4F4F4 },
    0xCFCFCF, 0x141414 }},
};

#define NTHEMES ((int)(sizeof THEMES / sizeof THEMES[0]))

static const char USAGE[] =
    "Usage: theme                 show the current theme and the rest\n"
    "       theme <name>          switch to a theme and remember it\n"
    "       theme <name> --once   switch without saving\n"
    "       theme bg #RRGGBB      set just the background\n"
    "       theme fg #RRGGBB      set just the text colour\n"
    "       theme -r              back to classic\n";

static theme_t g_cur;
static char    g_cur_name[32] = "classic";

static int parse_hex(const char *s, uint32_t *out) {
    if (*s == '#') s++;
    uint32_t v = 0;
    int n = 0;
    for (; *s; s++, n++) {
        char c = *s;
        int d;
        if      (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
        else return -1;
        v = (v << 4) | (uint32_t)d;
    }
    if (n != 6) return -1;
    *out = v;
    return 0;
}

static const named_theme_t *find_theme(const char *name) {
    for (int i = 0; i < NTHEMES; i++)
        if (!strcmp(THEMES[i].name, name)) return &THEMES[i];
    return NULL;
}

static int apply(const theme_t *t) {
    if (syscall1(SYS_CONSOLE_THEME, t) != 0) {
        fprintf(stderr, "theme: the console rejected it\n");
        return -1;
    }
    return 0;
}

static void load_conf(void) {
    g_cur = THEMES[0].t;
    FILE *f = fopen("/mnt" THEME_CONF, "r");
    if (!f) f = fopen(THEME_CONF, "r");
    if (!f) return;
    char line[128];
    while (fgets(line, sizeof line, f)) {
        char *nl = strchr(line, '\n'); if (nl) *nl = 0;
        char *eq = strchr(line, '='); if (!eq) continue;
        *eq = 0;
        const char *v = eq + 1;
        if (!strcmp(line, "theme")) {
            const named_theme_t *nt = find_theme(v);
            if (nt) { g_cur = nt->t; snprintf(g_cur_name, sizeof g_cur_name, "%s", v); }
        } else if (!strcmp(line, "bg")) {
            uint32_t c; if (parse_hex(v, &c) == 0) g_cur.bg = c;
        } else if (!strcmp(line, "fg")) {
            uint32_t c; if (parse_hex(v, &c) == 0) g_cur.fg = c;
        }
    }
    fclose(f);
}

static int save_conf(void) {
    FILE *f = fopen(THEME_CONF, "w");
    if (!f) { fprintf(stderr, "theme: cannot write %s\n", THEME_CONF); return -1; }
    fprintf(f, "theme=%s\n", g_cur_name);
    fprintf(f, "fg=%06X\n", g_cur.fg);
    fprintf(f, "bg=%06X\n", g_cur.bg);
    fclose(f);
    return 0;
}

static void show(void) {
    printf("current: \x1b[1m%s\x1b[0m   text #%06X on #%06X\n\n",
           g_cur_name, g_cur.fg, g_cur.bg);
    printf("available:\n");
    for (int i = 0; i < NTHEMES; i++) {
        const theme_t *t = &THEMES[i].t;
        printf("  %-11s %-31s ", THEMES[i].name, THEMES[i].about);
        for (int c = 1; c < 8; c++) {
            uint32_t v = t->palette[c];
            printf("\x1b[48;2;%u;%u;%um  \x1b[0m",
                   (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
        }
        for (int c = 9; c < 15; c++) {
            uint32_t v = t->palette[c];
            printf("\x1b[48;2;%u;%u;%um  \x1b[0m",
                   (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
        }
        printf(" \x1b[48;2;%u;%u;%um\x1b[38;2;%u;%u;%um Aa \x1b[0m",
               (t->bg >> 16) & 0xFF, (t->bg >> 8) & 0xFF, t->bg & 0xFF,
               (t->fg >> 16) & 0xFF, (t->fg >> 8) & 0xFF, t->fg & 0xFF);
        printf("\n");
    }
    printf("\nrun 'theme <name>' to switch\n");
}

int main(int argc, char **argv) {
    load_conf();

    if (argc < 2) { show(); return 0; }
    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) { fputs(USAGE, stdout); return 0; }

    if (!strcmp(argv[1], "--restore")) {
        return apply(&g_cur) == 0 ? 0 : 1;
    }
    if (!strcmp(argv[1], "-r")) {
        g_cur = THEMES[0].t;
        snprintf(g_cur_name, sizeof g_cur_name, "classic");
        if (apply(&g_cur) != 0) return 1;
        unlink(THEME_CONF);
        return 0;
    }

    if (!strcmp(argv[1], "bg") || !strcmp(argv[1], "fg")) {
        if (argc < 3) { fputs(USAGE, stderr); return 1; }
        uint32_t c;
        if (parse_hex(argv[2], &c) != 0) {
            fprintf(stderr, "theme: %s is not a #RRGGBB colour\n", argv[2]);
            return 1;
        }
        if (argv[1][0] == 'b') g_cur.bg = c; else g_cur.fg = c;
        if (apply(&g_cur) != 0) return 1;
        save_conf();
        return 0;
    }

    const named_theme_t *nt = find_theme(argv[1]);
    if (!nt) {
        fprintf(stderr, "theme: no theme called %s\n", argv[1]);
        show();
        return 1;
    }
    g_cur = nt->t;
    snprintf(g_cur_name, sizeof g_cur_name, "%s", nt->name);
    if (apply(&g_cur) != 0) return 1;
    if (argc < 3 || strcmp(argv[2], "--once") != 0) save_conf();
    return 0;
}
