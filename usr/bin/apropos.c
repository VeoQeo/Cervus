#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAN_ROOT "/usr/share/man"

static int ci_contains(const char *hay, const char *needle) {
    size_t nl = strlen(needle);
    if (nl == 0) return 1;
    for (const char *p = hay; *p; p++) {
        size_t i = 0;
        while (i < nl) {
            char a = p[i], b = needle[i];
            if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
            if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
            if (a != b) break;
            i++;
        }
        if (i == nl) return 1;
    }
    return 0;
}

static int scan_section(int sec, const char *word, int *found) {
    char dir[64];
    snprintf(dir, sizeof dir, MAN_ROOT "/man%d", sec);
    DIR *d = opendir(dir);
    if (!d) return 0;

    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.') continue;

        char path[320];
        snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;

        char line[256], name[128] = "", desc[200] = "";
        int in_name = 0;
        while (fgets(line, sizeof line, f)) {
            char *nl = strchr(line, '\n');
            if (nl) *nl = 0;
            if (!strncmp(line, "NAME", 4)) { in_name = 1; continue; }
            if (in_name) {
                char *p = line;
                while (*p == ' ' || *p == '\t') p++;
                if (!*p) continue;
                char *dash = strstr(p, " - ");
                if (dash) {
                    size_t n = (size_t)(dash - p);
                    if (n > sizeof name - 1) n = sizeof name - 1;
                    memcpy(name, p, n);
                    name[n] = 0;
                    snprintf(desc, sizeof desc, "%s", dash + 3);
                } else {
                    snprintf(name, sizeof name, "%s", p);
                }
                break;
            }
        }
        fclose(f);
        if (!name[0]) continue;

        if (ci_contains(name, word) || ci_contains(desc, word)) {
            printf("%-18s \x1b[90m(%d)\x1b[0m  %s\n", name, sec, desc);
            (*found)++;
        }
    }
    closedir(d);
    return 0;
}

static const char USAGE[] =
    "Usage: apropos <word> [...]\n"
    "Search the manual pages by name and description.\n"
    "\n"
    "  apropos disk       everything that mentions disks\n"
    "  apropos -s 2 time  only section 2\n";

int main(int argc, char **argv) {
    int only = 0, a = 1;
    if (argc >= 3 && !strcmp(argv[1], "-s")) { only = atoi(argv[2]); a = 3; }
    if (a >= argc || !strcmp(argv[a], "-h") || !strcmp(argv[a], "--help")) {
        fputs(USAGE, a >= argc ? stderr : stdout);
        return a >= argc ? 1 : 0;
    }

    int total = 0;
    for (; a < argc; a++) {
        int found = 0;
        if (only) scan_section(only, argv[a], &found);
        else for (int s = 1; s <= 3; s++) scan_section(s, argv[a], &found);
        total += found;
        if (!found) fprintf(stderr, "apropos: nothing matches '%s'\n", argv[a]);
    }
    return total ? 0 : 1;
}
