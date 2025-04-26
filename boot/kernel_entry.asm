[bits 32]
; From: https://github.com/cfenollosa/os-tutorial

[extern kernel_main] ; Define calling point. Must have same name as kernel.c 'main' function
;jmp $ ; TEMP FOR DEBUG
call kernel_main ; Calls the C function. The linker will know where it is placed in memory
jmp $

db "KERNEL ENTRY END"
