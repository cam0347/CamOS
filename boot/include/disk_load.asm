disk_load:
  push dx
  mov ah, 0x02
  mov al, dh
  mov ch, 0x00
  mov dh, 0x00
  mov cl, 0x02
  int 0x13
  jc disk_error

  pop dx
  cmp dh, al
  jne disk_error
  ret

disk_error:
  jmp $


load_kernel:
  mov ah, 0x2            ;subroutine
  mov al, 0x1            ;numbers of sectors to be read
  mov dl, [BOOT_DRIVE]   ;drive number

  mov ch, 0              ;cylinder number
  mov dh, 0              ;head number
  mov cl, 2              ;start sector (the first sector has index 1)

  mov bx, KERNEL_SEGMENT ;segment
  mov es, bx
  mov bx, 0x0000         ;segment offset
  int 0x13               ;drive low level routines
  ret
