[bits 32]
; From: https://github.com/cfenollosa/os-tutorial

[section .text.kernel_entry] ; << mark into special section
global _start
extern kernel_main

_start:
    ;jmp $ ; TEMP FOR DEBUG
    call kernel_main
    jmp $
