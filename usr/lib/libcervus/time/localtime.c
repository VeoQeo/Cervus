#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TZ_CONF "/etc/timezone"

static int  g_tz_loaded = 0;
static long g_tz_offset = 0;
static char g_tz_name[64] = "UTC";

static void tz_load(void) {
    if (g_tz_loaded) return;
    g_tz_loaded = 1;

    const char *env = getenv("TZ");
    char line[128];

    if (env && *env) {
        snprintf(line, sizeof line, "%s", env);
    } else {
        FILE *f = fopen("/mnt" TZ_CONF, "r");
        if (!f) f = fopen(TZ_CONF, "r");
        if (!f) return;
        if (!fgets(line, sizeof line, f)) { fclose(f); return; }
        fclose(f);
    }

    char *nl = strchr(line, '\n');
    if (nl) *nl = 0;

    char *sp = strchr(line, ' ');
    if (sp) {
        *sp = 0;
        g_tz_offset = strtol(sp + 1, NULL, 10) * 60;
    }
    snprintf(g_tz_name, sizeof g_tz_name, "%s", line);
}

long timezone_offset(void) { tz_load(); return g_tz_offset; }
const char *timezone_name(void) { tz_load(); return g_tz_name; }

struct tm *localtime(const time_t *t) {
    tz_load();
    if (!t) return gmtime(t);
    time_t local = *t + g_tz_offset;
    return gmtime(&local);
}
