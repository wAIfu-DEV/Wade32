#pragma once

// From: https://github.com/cfenollosa/os-tutorial

#include "../../xstd/xstd_core.h"
#include "../kernel_panic.h"
#include "../drivers/bios_io.h"

/* Struct which aggregates many registers */
typedef struct {
    u32 ds; /* Data segment selector */
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha. */
    u32 int_no, err_code; /* Interrupt number and error code (if applicable) */
    u32 eip, cs, eflags, useresp, ss; /* Pushed by the processor automatically */
} registers_t;

typedef void (*isr_t)(registers_t);
isr_t interrupt_handlers[256];

/* ISRs reserved for CPU exceptions */
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
/* IRQ definitions */
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47
 
/* Can't do this with a loop because we need the address
 * of the function names */
void isr_install() {
    set_idt_gate(0, (u32)isr0);
    set_idt_gate(1, (u32)isr1);
    set_idt_gate(2, (u32)isr2);
    set_idt_gate(3, (u32)isr3);
    set_idt_gate(4, (u32)isr4);
    set_idt_gate(5, (u32)isr5);
    set_idt_gate(6, (u32)isr6);
    set_idt_gate(7, (u32)isr7);
    set_idt_gate(8, (u32)isr8);
    set_idt_gate(9, (u32)isr9);
    set_idt_gate(10, (u32)isr10);
    set_idt_gate(11, (u32)isr11);
    set_idt_gate(12, (u32)isr12);
    set_idt_gate(13, (u32)isr13);
    set_idt_gate(14, (u32)isr14);
    set_idt_gate(15, (u32)isr15);
    set_idt_gate(16, (u32)isr16);
    set_idt_gate(17, (u32)isr17);
    set_idt_gate(18, (u32)isr18);
    set_idt_gate(19, (u32)isr19);
    set_idt_gate(20, (u32)isr20);
    set_idt_gate(21, (u32)isr21);
    set_idt_gate(22, (u32)isr22);
    set_idt_gate(23, (u32)isr23);
    set_idt_gate(24, (u32)isr24);
    set_idt_gate(25, (u32)isr25);
    set_idt_gate(26, (u32)isr26);
    set_idt_gate(27, (u32)isr27);
    set_idt_gate(28, (u32)isr28);
    set_idt_gate(29, (u32)isr29);
    set_idt_gate(30, (u32)isr30);
    set_idt_gate(31, (u32)isr31);

     // Remap the PIC
     bios_outb(0x20, 0x11);
     bios_outb(0xA0, 0x11);
     bios_outb(0x21, 0x20);
     bios_outb(0xA1, 0x28);
     bios_outb(0x21, 0x04);
     bios_outb(0xA1, 0x02);
     bios_outb(0x21, 0x01);
     bios_outb(0xA1, 0x01);
     bios_outb(0x21, 0x0);
     bios_outb(0xA1, 0x0); 
 
     // Install the IRQs
     set_idt_gate(32, (u32)irq0);
     set_idt_gate(33, (u32)irq1);
     set_idt_gate(34, (u32)irq2);
     set_idt_gate(35, (u32)irq3);
     set_idt_gate(36, (u32)irq4);
     set_idt_gate(37, (u32)irq5);
     set_idt_gate(38, (u32)irq6);
     set_idt_gate(39, (u32)irq7);
     set_idt_gate(40, (u32)irq8);
     set_idt_gate(41, (u32)irq9);
     set_idt_gate(42, (u32)irq10);
     set_idt_gate(43, (u32)irq11);
     set_idt_gate(44, (u32)irq12);
     set_idt_gate(45, (u32)irq13);
     set_idt_gate(46, (u32)irq14);
     set_idt_gate(47, (u32)irq15);
 
     set_idt(); // Load with ASM
}

/* To print the message which defines every exception */
ConstStr exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_handler(registers_t r)
{
    kernel_panic(r.int_no + 2048, exception_messages[r.int_no]);
}

void register_interrupt_handler(u8 n, isr_t handler)
{
    interrupt_handlers[n] = handler;
}

void irq_handler(registers_t r)
{
    /* After every interrupt we need to send an EOI to the PICs
     * or they will not send another interrupt again */
    if (r.int_no >= 40) bios_outb(0xA0, 0x20); /* slave */
    bios_outb(0x20, 0x20); /* master */

    /* Handle the interrupt in a more modular way */
    if (interrupt_handlers[r.int_no] != 0) {
        isr_t handler = interrupt_handlers[r.int_no];
        handler(r);
    }
}
