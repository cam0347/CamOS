bits 32

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

print_p:
  pusha
  mov edx, VIDEO_MEMORY
  jmp print_p_loop

print_p_loop:
  mov al, [ebx]
  mov ah, WHITE_ON_BLACK
  cmp byte al, 0x0
  je print_p_done

  mov [edx], ax

  inc ebx
  add edx, 2

  jmp print_p_loop

print_p_done:
  popa
  ret
