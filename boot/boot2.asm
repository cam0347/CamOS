bits 16
org 0x7E00

;A20 ENABLING PROCEDURE
;segment: 0x0000
;offset:  0x7E00

jmp start

%include "include/print_r.asm"
%include "include/protected_mode.asm"
%include "include/gdt.asm"
bits 16

;variables
checking_string: db "Checking A20...", 0x0D, 0x0A, 0x0
enabled_string: db "A20 enabled", 0x0D, 0x0A, 0x0
disabled_string: db "A20 disabled, enabling...", 0x0D, 0x0A, 0x0
success_msg: db "done", 0x0D, 0x0A, 0x0
failure_msg: db "failed", 0x0D, 0x0A, 0x0
gdt_msg: db "Loading GDT... ", 0x0
sh_area_msg: db "Setting up shared memory area... ", 0x0
bios_info_msg: db "Gathering some hardware infos...", 0x0
ssma_addr: dw 0x7000

;shared memory area ranging from 0x7000 to 0x7400 (1 Kbyte)

start:
  mov si, checking_string
  call print_r

  call A20_test
  cmp ax, 1
  je A20_enabled_func
  jmp A20_disabled_func

continue:
  call init_ssma
  jmp switch_to_pm

A20_test:
  pushf
  push ds
  push es
  push di
  push si
  cli
  xor ax, ax
  mov es, ax
  mov di, 0x0500
  mov ax, 0xffff
  mov ds, ax
  mov si, 0x0510
  mov al, byte es:[di]
  push ax
  mov al, byte ds:[si]
  push ax
  mov byte es:[di], 0x00
  mov byte ds:[si], 0xFF
  cmp byte es:[di], 0xFF
  pop ax
  mov byte ds:[si], al
  pop ax
  mov byte es:[di], al
  mov ax, 0
  je A20_test_exit
  mov ax, 1

A20_test_exit:
  pop si
  pop di
  pop es
  pop ds
  popf
  ret

enable_A20:
  cli
  call a20wait
  mov al, 0xAD
  out 0x64, al
  call a20wait
  mov al, 0xD0
  out 0x64, al
  call a20wait2
  in al, 0x60
  push eax
  call a20wait
  mov al, 0xD1
  out 0x64, al
  call a20wait
  pop eax
  or al, 2
  out 0x60, al
  call a20wait
  mov al, 0xAE
  out 0x64, al
  call a20wait
  sti
  ret

a20wait:
  in al, 0x64
  test al, 2
  jnz a20wait
  ret

a20wait2:
  in al, 0x64
  test al, 1
  jz a20wait2
  ret

A20_disabled_func:
  mov si, disabled_string
  call print_r
  call enable_A20
  jmp start ;after enabling A20, jumps to the check

A20_enabled_func:
  mov si, enabled_string
  call print_r
  jmp continue

init_ssma:
  mov si, sh_area_msg
  call print_r
  mov cx, 1024
  call ssma_loop
  call process_done
  ret

ssma_loop:
  cmp cx, 0
  jne ssma_loop_exec
  ret

ssma_loop_exec:
  mov ax, [sh_area_msg]
  inc ax
  mov [ssma_addr], ax
  mov word [ssma_addr], 0x00
  dec cx
  jmp ssma_loop

times 2044-($-$$) db 0x00
bootm_entry: dd 0x00000000
