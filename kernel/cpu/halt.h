#pragma once

/**
 * @brief Halts the CPU until an interrupt is received.
 */
void halt(void)
{
    __asm__ volatile("hlt");
}
