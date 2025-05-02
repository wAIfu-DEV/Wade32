#pragma once

/**
 * @brief Halts the CPU until an interrupt is received.
 */
void halt_no_enable_ints(void)
{
    __asm__ volatile("hlt");
}

/**
 * @brief Enables interrupts and halts the CPU until an interrupt is received.
 */
void halt_enable_ints(void)
{
    __asm__ volatile("sti; hlt");
}
