#ifndef KERNEL_DRIVERS_VIDEO_BGA_H
#define KERNEL_DRIVERS_VIDEO_BGA_H

#include <stdint.h>

typedef struct {
    uint16_t width;
    uint16_t height;
} bga_mode_t;

void bga_init(void);
int  bga_present(void);
int  bga_list_modes(bga_mode_t *out, int max);
int  bga_set_mode(uint32_t width, uint32_t height);

#endif
