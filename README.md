# Wade32 Operating System

Wade32 is a minimal 32-bit x86 operating system developed from scratch. It offers a foundational platform for exploring low-level systems programming, kernel design, and operating system mechanics.

---

## 📐 Architecture Overview

### 🧠 Core Design Philosophy

Wade32 was designed with the following principles in mind:

- **Safety**: Error handling is explicit and required by design.
- **Readability**: All functions and names are clear and explicit.

---

## 🏗️ System Layers Overview

### 1. Bootloader (Assembly)

The system boots using BIOS interrupts in 16-bit real mode. A custom bootloader performs the following:

- Loads the kernel image from disk
- Sets up a temporary stack
- Switches to Protected Mode via a GDT
- Transfers execution to the 32-bit kernel

### 2. Kernel (C + Assembly Interface)

The kernel takes over in 32-bit Protected Mode and provides several subsystems to abstract hardware and execution. It does so without dynamic memory allocation or third-party dependencies.

✅ Major Components:

| Subsystem       | Description |
|----------------|-------------|
| **VGA Driver**         | Low-level text-based output on VGA-compatible memory. Easy styling and screen manipulation. |
| **Timer & Scheduler**  | Tick-based cooperative scheduler with event and routine support. |
| **Input Handling**     | Interrupt-driven keyboard listener system with registrable buffers. |
| **Shell Interface**    | In-kernel command system supporting user commands and kernel apps. |
| **Heap Management**    | Simple heap allocator with debug tracking support. Allocator-agnostic memory APIs. |
| **Kernel Applications**| Modular "user" applications compiled into the kernel image and invoked via the shell. |
| **Time Subsystem**     | UTC time retrieval via BIOS RTC and formatting to human-readable strings. |

---

## 🖥️ Execution Overview

1. BIOS loads and runs the custom bootloader at 0x7C00.
2. Bootloader loads the kernel into memory and switches to 32-bit Protected Mode.
3. Entry point (`kernel_main`):
   - Initializes core services: VGA, heap, interrupts, scheduler.
   - Installs ISRs & IRQ handlers (keyboard, timers, etc).
   - Loads the kernel shell from registry.
4. Kernel shell displays interface, listens for keyboard input.
5. User may launch compiled-in kernel apps via the shell (only `sysinfo` is available for now)
6. Scheduler and timer subsystems allow apps to yield and sleep cooperatively.

---

## ⚙️ Unique Architectural Features

- **Own Standard Library**: Makes use of [**XSTD**](https://github.com/wAIfu-DEV/XSTD) a C standard library made from scratch.

---

## 🚫 Current Limitations

- No page tables or virtual memory
- No disk filesystem or I/O abstraction
- All apps are statically compiled into the kernel
- All code runs in kernel level privilege
- No multithreading — only cooperative multitasking via scheduler/yield
- Screen rendering locked to VGA text mode (80x25) (no pixel buffers)

---

## 🛠️ How to Build & Boot

Refer to the build.bat script

Requirements:
- nasm
- i386_elf gcc and ld
- QEMU for running the floppy image

---

## 🤝 Inspirations

Wade32 draws influence from:

- [cfenollosa/os-tutorial](https://github.com/cfenollosa/os-tutorial)

## 🚀 Future Directions (Ideas)

- Shell command parsing with arguments
- Add FAT12 file system support
- Introduce dynamic module loading
- Implement context switching and thread primitives
- Move toward ring-3 execution via privilege levels
