org 0x7C00
bits 16

;segment 0x0000:0x7C00
;linear 0x7C00

;bootloader still have to:
;gather ACPI informations from bios/uefi internal memory
;check PCI access method

;free memory from 0x00007E00 to 0x0007FFFF

jmp init

%include "include/lba_chs.asm"
%include "include/print_r.asm"
%include "include/print_p.asm"

bits 16

;static vars
bootloader_st2_linear equ 0x7E00 ;assembly code
bootm_code_address equ 0x8600
bootm_data_address equ 0x18600
bootm_rodata_address equ 0x20600
bootm_code_lba equ 7
bootm_data_lba equ 135
bootm_rodata_lba equ 199

;variables
hello_msg: db "CamOS 1.0", 0x0D, 0x0A, 0x0
stage2_loading_msg: db "Loading stage 2... ", 0x0
disk_geom_msg: db "Calculating disk geometry... ", 0x0
disk_index: db 0x0
success_msg: db "done", 0x0D, 0x0A, 0x0
failure_msg: db "failed", 0x0D, 0x0A, 0x0
bootm_code_sec: db 7
bootm_data_sec: db 135
bootm_rodata_sec: db 199
bootmanager_loading_msg: db "Loading boot manager... ", 0x0

chs_c: dw 0x0000
chs_h: db 0x00
chs_s: db 0x00
disks_number: db 0x00

init:
mov [disk_index], dl ;save disk index

;set up the stack
mov bp, 0x1000
mov sp, bp

;cleans the screen
mov al, 0x2
mov ah, 0x0
int 0x10

;welcome the user
mov si, hello_msg
call print_r

call gather_disk_geometry ;gather disk geometry
call load_stage2 ;loads stage 2
call load_bootmg ;loads boot manager

jmp bootloader_st2_linear ;jump to stage 2 code
;stage 2 enables a20, loads the boot manager, erase the shared memory area and switch to protected mode

cli
hlt

bits 16
;stage 2 loaded at 0x7E00 (linear) ends at 0x8600 (linear) (2 Kbyte), from sector 3 to 7 on the disk
load_stage2:
  mov si, stage2_loading_msg
  call print_r

  mov ah, 0x2
  mov al, 0x4            ;numbers of sectors to be read
  mov dl, [disk_index]

  mov ch, 0                     ;cylinder number
  mov dh, 0                     ;head number
  mov cl, 3                     ;start sector (the first sector has index 1)

  mov bx, 0x0000                ;segment
  mov es, bx
  mov bx, bootloader_st2_linear ;segment offset
  int 0x13
  jc process_error
  call process_done
  ret

process_error:
  mov si, failure_msg
  call print_r
  cli
  hlt

process_done:
  mov si, success_msg
  call print_r
  ret

load_bootmg:
  mov si, bootmanager_loading_msg
  call print_r

  mov bx, bootm_code_lba
  call lba_to_chs

  ;load code segment
  mov ah, 0x2
  mov al, 0x80            ;numbers of sectors to be read (128)
  mov dl, [disk_index]

  mov bx, 0x0000         ;segment
  mov es, bx
  mov bx, 0x8600         ;segment offset
  int 0x13
  jc process_error
  clc

  mov ah, 0x00
  int 0x13

  ;load data segment
  mov bx, bootm_data_lba
  call lba_to_chs

  mov ah, 0x2
  mov al, 0x40            ;numbers of sectors to be read (64)
  mov dl, [disk_index]

  mov bx, 0x1860         ;segment
  mov es, bx
  mov bx, 0x0000 ;segment offset
  int 0x13
  jc process_error
  clc

  mov ah, 0x00
  int 0x13

  ;load rodata segment
  mov bx, bootm_rodata_lba
  call lba_to_chs

  mov ah, 0x2
  mov al, 0x40            ;numbers of sectors to be read (64)
  mov dl, [disk_index]

  mov bx, 0x2000         ;segment
  mov es, bx
  mov bx, 0x600 ;segment offset
  int 0x13
  jc process_error
  clc

  mov ah, 0x00
  int 0x13
  call process_done
  ret

gather_disk_geometry:
  mov si, disk_geom_msg
  call print_r
  mov ah, 0x08
  mov dl, [disk_index]
  xor bx, bx
  mov es, bx
  mov di, bx
  int 0x13
  jc process_error
  mov [disks_number], byte dl ;disks number
  mov [chs_h], byte dh ;heads number
  mov al, cl
  and al, 00111111b
  mov [chs_s], byte al ;sectors number
  ror cx, 6
  and cx, 0000001111111111b
  mov [chs_c], word cx ;cylinders number
  call process_done
  ret

times 510-($-$$) db 0x00
dw 0xAA55
