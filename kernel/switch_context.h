#pragma once

#include "types/proc_context.h"
#include "cpu/interrupts.h"

__attribute__((naked))
void switch_context(Context* old, Context* new)
{
    interrupts_disable();

    __asm__ volatile(
        // Save caller-saved registers to old context
        "pusha\n\t"
        "pushf\n\t"

        // Save ESP and EIP manually
        "movl 4(%%ebp), %%eax\n\t"    // old context ptr in eax
        "movl %%esp, (%%eax)\n\t"     // save esp to old->esp

        "lea 1f, %%ecx\n\t"           // address of label 1 (resume point)
        "movl %%ecx, 36(%%eax)\n\t"   // save eip to old->eip

        // Save EFLAGS
        "popf\n\t"
        "pushf\n\t"
        "pop %%edx\n\t"
        "movl %%edx, 40(%%eax)\n\t"

        // Restore new context
        "movl 8(%%ebp), %%ebx\n\t"    // new context ptr in ebx
        "movl (%%ebx), %%esp\n\t"     // load esp from new->esp

        // Push new eip and flags
        "movl 36(%%ebx), %%ecx\n\t"   // eip
        "pushl %%ecx\n\t"
        "movl 40(%%ebx), %%edx\n\t"   // eflags
        "pushl %%edx\n\t"
        "popf\n\t"                    // restore eflags

        "pop %%ecx\n\t"               // pop eip
        "jmp *%%ecx\n\t"              // jump to new->eip

        "1:\n\t" // Resume point
        "popa\n\t"
        "ret\n\t"
    );

    interrupts_enable();
}
