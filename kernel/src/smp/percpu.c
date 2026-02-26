#include "../../include/smp/percpu.h"
#include "../../include/smp/smp.h"
#include "../../include/memory/pmm.h"
#include "../../include/io/serial.h"
#include <string.h>
#include <stdlib.h>
extern uintptr_t __percpu_start;
extern uintptr_t __percpu_end;
PERCPU_SECTION percpu_t percpu = {0};
percpu_t* percpu_regions[MAX_CPUS] = {0};
void init_percpu_regions(void) {
    size_t percpu_size = (uintptr_t)&__percpu_end - (uintptr_t)&__percpu_start;
    smp_info_t* info = smp_get_info();
    for (uint32_t i = 0; i < info->cpu_count; i++) {
        void* region = malloc(percpu_size);
        if (!region) continue;
        memcpy(region, &__percpu_start, percpu_size);
        percpu_regions[i] = (percpu_t*)region;
        percpu_regions[i]->cpu_id = info->cpus[i].lapic_id;
    }
}
percpu_t* get_percpu(void) {
    uint64_t base;
    uint32_t low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(0xC0000101));
    base = ((uint64_t)high << 32) | low;
    return (percpu_t*)base;
}
void set_percpu_base(percpu_t* base) {
    uintptr_t addr = (uintptr_t)base;
    uint32_t low = addr & 0xFFFFFFFF;
    uint32_t high = addr >> 32;
    __asm__ volatile ("wrmsr" : : "c"(0xC0000101), "a"(low), "d"(high) : "memory");
}
