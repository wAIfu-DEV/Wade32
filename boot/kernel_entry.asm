[bits 32]
; From: https://github.com/cfenollosa/os-tutorial

[section .text.kernel_entry] ; << mark into special section
global _start
global stack_canary
extern kernel_main

_start:
    ;jmp $ ; TEMP FOR DEBUG
    call kernel_main
    jmp $

stack_canary db "STACK CANARY", 0
