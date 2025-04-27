[org 0x7c00]

; This bootloader is a bastard between the one you can find here:
; https://github.com/cfenollosa/os-tutorial <- Highly recommend it btw
; and the one from:
; https://stackoverflow.com/questions/54894585/legacy-bios-bootloader-to-bootstrap-real-mode-code-in-second-stage/54894586#54894586
; Had to merge the two of them since the first one was limited to 63 max disk sectors
; which is not enough for Wade32.

KERNEL_OFFSET    equ 0x07e00 ; The same one we used when linking the kernel
KERNEL_ABS_ADDR  equ 0x07e00
KERNEL_RUN_SEG   equ 0x0000
KERNEL_RUN_OFS   equ KERNEL_ABS_ADDR
                                ; Run stage2 with segment of 0x0000 and offset of 0x7e00

KERNEL_LOAD_SEG  equ KERNEL_ABS_ADDR>>4
                                ; Segment to start reading Stage2 into
                                ;     right after bootloader

KERNEL_LBA_START equ 1          ; Logical Block Address(LBA) Stage2 starts on
                                ;     LBA 1 = sector after boot sector
KERNEL_LBA_END   equ KERNEL_LBA_START + KERNEL_SECTORS
                                ; Logical Block Address(LBA) Stage2 ends at
DISK_RETRIES     equ 3          ; Number of times to retry on disk error

; auto-generated kernel size information
;%include "./kernel_size.inc"

    mov [BOOT_DRIVE], dl ; Remember that the BIOS sets us the boot drive in 'dl' on boot
    mov bp, 0x9000
    mov sp, bp

    mov bx, MSG_REAL_MODE 
    call print
    call print_nl

    call load_kernel ; read the kernel from disk
finished_loading_kernel:
    call switch_to_pm ; disable interrupts, load GDT,  etc. Finally jumps to 'BEGIN_PM'
    jmp $ ; Never executed

%include "./print.asm"
%include "./print_hex.asm"
;%include "./disk.asm"
%include "./32b_gdt.asm"
%include "./32b_print.asm"
%include "./32b_switch.asm"

[bits 16]
load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print
    call print_nl

    pusha
    xor ax, ax                  ; DS=SS=0 for stage2 loading
    mov ds, ax
    mov ss, ax                  ; Stack at 0x0000:0x7c00
    mov sp, 0x7c00
    cld                         ; Set string instructions to use forward movement

    mov [bootDevice], dl        ; Save boot drive
    mov di, KERNEL_LOAD_SEG     ; DI = Current segment to read into
    mov si, KERNEL_LBA_START    ; SI = LBA that stage2 starts at
    jmp .chk_for_last_lba       ; Check to see if we are last sector in stage2

.read_sector_loop:
    mov bp, DISK_RETRIES        ; Set disk retry count

    call lba_to_chs             ; Convert current LBA to CHS
    mov es, di                  ; Set ES to current segment number to read into
    xor bx, bx                  ; Offset zero in segment

.retry:
    mov ax, 0x0201              ; Call function 0x02 of int 13h (read sectors)
                                ;     AL = 1 = Sectors to read
    int 0x13                    ; BIOS Disk interrupt call
    jc .disk_error              ; If CF set then disk error

.success:
    add di, 512>>4              ; Advance to next 512 byte segment (0x20*16=512)
    inc si                      ; Next LBA

.chk_for_last_lba:
    cmp si, KERNEL_LBA_END      ; Have we reached the last stage2 sector?
    jl .read_sector_loop        ;     If we haven't then read next sector

.stage2_loaded:
    popa

    jmp finished_loading_kernel
    ret ; ^^^ had to do this since ret doesn't seem to work here
        ; I don't really like assembly

.disk_error:
    xor ah, ah                  ; Int13h/AH=0 is drive reset
    int 0x13
    dec bp                      ; Decrease retry count
    jge .retry                  ; If retry count not exceeded then try again

error_end:
    ; Unrecoverable error; print drive error; enter infinite loop
    mov si, diskErrorMsg        ; Display disk error message
    call print_string
    cli
.error_loop:
    hlt
    jmp .error_loop

print_string:
    mov ah, 0x0e                ; BIOS tty Print
    xor bx, bx                  ; Set display page to 0 (BL)
    jmp .getch
.repeat:
    int 0x10                    ; print character
.getch:
    lodsb                       ; Get character from string
    test al,al                  ; Have we reached end of string?
    jnz .repeat                 ;     if not process next character
.end:
    ret

lba_to_chs:
    push ax                    ; Preserve AX
    mov ax, si                 ; Copy LBA to AX
    xor dx, dx                 ; Upper 16-bit of 32-bit value set to 0 for DIV
    div word [sectorsPerTrack] ; 32-bit by 16-bit DIV : LBA / SPT
    mov cl, dl                 ; CL = S = LBA mod SPT
    inc cl                     ; CL = S = (LBA mod SPT) + 1
    xor dx, dx                 ; Upper 16-bit of 32-bit value set to 0 for DIV
    div word [numHeads]        ; 32-bit by 16-bit DIV : (LBA / SPT) / HEADS
    mov dh, dl                 ; DH = H = (LBA / SPT) mod HEADS
    mov dl, [bootDevice]       ; boot device, not necessary to set but convenient
    mov ch, al                 ; CH = C(lower 8 bits) = (LBA / SPT) / HEADS
    shl ah, 6                  ; Store upper 2 bits of 10-bit Cylinder into
    or  cl, ah                 ;     upper 2 bits of Sector (CL)
    pop ax                     ; Restore scratch registers
    ret

[bits 32]
BEGIN_PM:
    mov ebx, MSG_PROT_MODE
    call print_string_pm
    ;jmp $ ; TEMP FOR DEBUGGING
    call KERNEL_OFFSET ; Give control to the kernel
    jmp $ ; Stay here when the kernel returns control to us (if ever)


BOOT_DRIVE db 0 ; It is a good idea to store it in memory because 'dl' may get overwritten
MSG_DEBUG db "DB", 0
MSG_REAL_MODE db "Started in 16-bit Real Mode", 0
MSG_PROT_MODE db "Landed in 32-bit Protected Mode", 0
MSG_LOAD_KERNEL db "Loading kernel into memory", 0

numHeads:        dw 2          ; 1.44MB Floppy has 2 heads & 18 sector per track
sectorsPerTrack: dw 18
bootDevice:      db 0x00
diskErrorMsg:    db "Unrecoverable disk error!", 0

; padding
times 510 - ($-$$) db 0
dw 0xaa55

KERNEL_SECTORS equ (kernel_end - kernel_start + 511) / 512
kernel_start:
    incbin "../out/kernel.bin"
kernel_end:

; Fill out this file to produce a 1.44MB floppy image
; If this ever fails (time negative error) then it mean kernel has become
; too large for floppy.
times 1024 * 1440 - ($-$$) db 0
