#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../include/io/serial.h"
#include "../include/memory/pmm.h"
#include "../include/smp/smp.h"
#include "../include/graphics/fb/fb.h"
extern void sched_print_stats(void);
extern void acpi_shutdown(void);
void cervus_fetch(void) {
    serial_writestring("\n");
    serial_writestring("      /|       /|\n");
    serial_writestring("   `__\\\\       
    serial_printf("      \\ \\     / /      \033[93mOS:\033[0m Cervus x86_64\n");
    serial_printf("       \\ \\___/ /       \033[93mVersion:\033[0m v0.0.1-alpha\n");
    serial_printf("       /  6 6  \\       \033[93mCores:\033[0m %u CPU cores\n", smp_get_cpu_count());
    serial_printf("      (    v    )      \033[93mMemory:\033[0m %zu / %zu MB\n", 
           (pmm_get_used_pages() * 4096) / 1024 / 1024, 
           (pmm_get_total_pages() * 4096) / 1024 / 1024);
    serial_printf("       \\  -=-  /       \033[93mStage:\033[0m Alpha (Ring 0)\n");
    serial_printf("       /       \\       \033[93mBuilder:\033[0m C-Builder\n");
    serial_writestring("      /         \\\n\n");
}
void kgets(char* buf, size_t max) {
    size_t i = 0;
    while (i < max - 1) {
        char c = serial_read();
        if (c == '\r' || c == '\n') {
            serial_write('\n');
            break;
        } else if (c == '\b' || c == 127) {
            if (i > 0) {
                i--;
                serial_writestring("\b \b");
            }
        } else {
            buf[i++] = c;
            serial_write(c);
        }
    }
    buf[i] = '\0';
}
void shell_main(void) {
    char cmd[128];
    serial_writestring("\n\033[92m--- Cervus Interactive Shell v0.2 ---\033[0m\n");
    cervus_fetch(); 
    while (1) {
        serial_writestring("\033[96mcervus>\033[0m ");
        kgets(cmd, 128);
        if (strcmp(cmd, "help") == 0) {
            serial_writestring("Available commands:\n");
            serial_writestring("  fetch    - Show system info & logo\n");
            serial_writestring("  tasks    - List running kernel tasks\n");
            serial_writestring("  mem      - Physical memory statistics\n");
            serial_writestring("  clear    - Clear terminal screen\n");
            serial_writestring("  poweroff - Shut down the system\n");
            serial_writestring("  panic    - Trigger a kernel exception\n");
        } 
        else if (strcmp(cmd, "fetch") == 0) {
            cervus_fetch();
        }
        else if (strcmp(cmd, "tasks") == 0) {
            sched_print_stats();
        }
        else if (strcmp(cmd, "mem") == 0) {
            pmm_print_stats();
        }
        else if (strcmp(cmd, "clear") == 0) {
            serial_writestring("\033[H\033[J");
        }
        else if (strcmp(cmd, "poweroff") == 0) {
            serial_writestring("Shutting down Cervus OS...\n");
            acpi_shutdown();
        }
        else if (strcmp(cmd, "panic") == 0) {
            serial_writestring("Forcing Kernel Panic...\n");
            asm volatile("ud2");
        }
        else if (strlen(cmd) > 0) {
            serial_printf("Unknown command: %s (try 'help')\n", cmd);
        }
    }
}
