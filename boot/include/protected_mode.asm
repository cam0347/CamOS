bits 16
process_error:
  mov si, failure_msg
  call print_r
  jmp $

process_done:
  mov si, success_msg
  call print_r
  ret

switch_to_pm:
  mov si, gdt_msg
  call print_r
  cli
  lgdt [gdt_descriptor]
  call process_done
  mov eax, cr0
  or eax, 0x1
  mov cr0, eax
  jmp CODE_SEG:init_pm

bits 32
init_pm:
  mov ax, DATA_SEG
  mov ds, ax
  mov ss, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ebp, 0x8600
  mov esp, ebp
  mov eax, [bootm_entry]
  jmp [bootm_entry]
  cli
  hlt
