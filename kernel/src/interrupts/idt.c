#include "../../include/interrupts/idt.h"
#include "../../include/interrupts/isr.h"
#include "../../include/interrupts/irq.h"
#include "../../include/io/serial.h"
#include <stddef.h>
#include <string.h>

#define IDT_MAX_DESCRIPTORS 256
#define GDT_CODE_SEGMENT 0x08

extern void *isr_stub_table[];

static struct idt_entry idt[IDT_MAX_DESCRIPTORS];
static struct idt_ptr idt_ptr;

static interrupt_handler_t interrupt_handlers[IDT_MAX_DESCRIPTORS] = {0};

void idt_init(void) {
    serial_writestring(COM1, "[IDT] Initializing IDT...\n");
    
    // Очищаем IDT
    memset(idt, 0, sizeof(idt));
    
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint64_t)&idt;
    
    for (uint16_t vector = 0; vector < IDT_MAX_DESCRIPTORS; vector++) {
        idt_set_entry(vector, isr_stub_table[vector], GDT_CODE_SEGMENT, 0x8E);
    }

    __asm__ volatile("lidt %0" : : "m"(idt_ptr)); 
    
    isr_init();
    irq_init();
    
    serial_writestring(COM1, "[IDT] IDT initialized successfully\n");
}

void idt_set_entry(uint8_t index, void *base, uint16_t selector, uint8_t flags) {
    idt[index].base_low   = (uint64_t)base & 0xFFFF;
    idt[index].kernel_cs  = selector;
    idt[index].ist        = 0;
    idt[index].attributes = flags;
    idt[index].base_mid   = ((uint64_t)base >> 16) & 0xFFFF;
    idt[index].base_high  = ((uint64_t)base >> 32) & 0xFFFFFFFF;
    idt[index].reserved   = 0;
}

void register_interrupt_handler(uint16_t interrupt, interrupt_handler_t handler) {
    if (interrupt < 256) {
        interrupt_handlers[interrupt] = handler;
    }
}

interrupt_handler_t get_interrupt_handler(uint16_t interrupt) {
    if (interrupt < 256) {
        return interrupt_handlers[interrupt];
    }
    return NULL;
}

void base_trap(void* ctx) {

    serial_printf(COM1, "PANIC! BIG CERVUS PENIS!!!!\n O HOLERO CHITO FREDDY FASBEAR!!!!!");

    for (;;) {
        asm volatile ("hlt");
    }
}

void irq_handler(struct interrupt_frame* frame) {
    interrupt_handler_t handler = get_interrupt_handler(frame->interrupt_number);
    
    if (handler != NULL) {
        handler(frame);
    } else {
        serial_printf(COM1, "[IRQ] Unhandled IRQ %d\n", frame->interrupt_number - IRQ_BASE);
    }
    
    irq_send_eoi(frame->interrupt_number - IRQ_BASE);
}