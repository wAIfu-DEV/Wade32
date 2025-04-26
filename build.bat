@ECHO OFF

SET BINDIR=~/opt/cross/bin
SET ARCH=i386

WSL rm -rf out
WSL mkdir out

:: Compile kernel entry asm
WSL nasm -f elf32 boot/kernel_entry.asm -o out/kernel_entry.o

IF NOT EXIST "out/kernel_entry.o" (
    ECHO failed to build kernel_entry.asm
    GOTO :Ex
)
ECHO # built kernel_entry.asm

:: Compile kernel entry asm
WSL nasm -f elf32 kernel/cpu/interrupt.asm -o out/interrupt.o

IF NOT EXIST "out/interrupt.o" (
    ECHO failed to build kernel/cpu/interrupt.asm
    GOTO :Ex
)
ECHO # built kernel/cpu/interrupt.asm

:: Compile kernel C files
:: -mno-red-zone
:: -O3 kills it
WSL %BINDIR%/%ARCH%-elf-gcc -ffreestanding -c kernel/kernel.c -o out/kernel.o -Wall -Werror -pedantic

IF NOT EXIST "out/kernel.o" (
    ECHO # failed to build kernel.c
    GOTO :Ex
)
ECHO # built kernel.c

:: Link kernel to kernel entry
WSL %BINDIR%/%ARCH%-elf-ld -o out/kernel.bin -Ttext 0x07e00 out/kernel_entry.o out/interrupt.o out/kernel.o --oformat binary

IF NOT EXIST "out/kernel.bin" (
    ECHO # failed to link kernel.o
    GOTO :Ex
)
ECHO # linked kernel.o

:: Compile boot sector
CD boot
    WSL nasm -f bin boot.asm -o ../out/Wade32.bin
CD ..

IF NOT EXIST "out/Wade32.bin" (
    ECHO # failed to build Wade32.asm
    GOTO :Ex
)
ECHO # built Wade32.bin
ECHO # done.

:: Run QEMU
::C:\QEMU\qemu-system-%ARCH%.exe -s -drive file=out/Wade32.bin,format=raw,if=floppy
C:\QEMU\qemu-system-%ARCH%.exe -s -fda out/Wade32.bin

:Ex
PAUSE
