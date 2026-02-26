#include <stdio.h>
#include <stdint.h>
#include "../kernel/include/memory/pmm.h"
#include "../kernel/include/smp/smp.h"

// ASCII Арт Оленя (Cervus)
const char* deer_ascii[] = {
    "      /|       /|",
    "   `__\\\\       //__'",
    "      \\ \\     / /",
    "       \\ \\___/ /",
    "       /  6 6  \\",
    "      (    v    )",
    "       \\  -=-  /",
    "       /       \\",
    "      /         \\"
};

void cervus_fetch() {
    // Получаем данные о памяти
    size_t total_mb = (pmm_get_total_pages() * 4096) / 1024 / 1024;
    size_t used_mb = (pmm_get_used_pages() * 4096) / 1024 / 1024;
    
    // Получаем количество ядер
    uint32_t cores = smp_get_cpu_count();

    printf("\n");
    // Выводим арт и инфо в две колонки (имитация)
    for (int i = 0; i < 9; i++) {
        printf("  %s  ", deer_ascii[i]);
        
        // Добавляем текст рядом с артом
        switch(i) {
            case 1: printf("  \033[93mOS:\033[0m Cervus x86_64"); break;
            case 2: printf("  \033[93mVersion:\033[0m v0.0.1-alpha"); break;
            case 3: printf("  \033[93mCores:\033[0m %u CPU detected", cores); break;
            case 4: printf("  \033[93mMemory:\033[0m %zu MB / %zu MB", used_mb, total_mb); break;
            case 5: printf("  \033[93mStage:\033[0m Kernel Mode (Ring 0)"); break;
            case 6: printf("  \033[93mBootloader:\033[0m Limine Barebone"); break;
        }
        printf("\n");
    }
    printf("\n   \033[92m[#] System Scan Complete.\033[0m\n\n");
}

// Если запускаем как отдельный ELF
void _start() {
    cervus_fetch();
    while(1) { __asm__("pause"); }
}
