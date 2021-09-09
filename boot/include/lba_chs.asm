lba_to_chs:
  cmp bx, 0
  je lba_to_chs_error

  mov ax, bx
  div byte [chs_s] ;AX / [chs_s] = AL
  mov ah, 0
  div byte [chs_h] ;AX / [chs_h] = AL
  mov ah, 0
  push word ax ;cylinder (al)

  mov ax, bx
  div byte [chs_s]
  mov ah, 0
  div byte [chs_h] ;AX % [chs_h] = AH
  mov al, ah
  mov ah, 0
  push word ax ;head (al)

  mov ax, bx
  div byte [chs_s]
  mov al, ah
  mov ah, 0
  push word ax

  xor ax, ax
  xor bx, bx
  xor cx, cx
  xor dx, dx

  pop word cx
  mov ch, 0

  pop word dx
  mov dh, dl
  mov dl, 0

  pop word bx
  mov ch, bl
  xor bx, bx

  mov ax, 1
  ret

lba_to_chs_error:
  mov ax, 0
  ret

  ;Return:
  ;ch: cylinder
  ;dh: head
  ;cl: sector (0 if error)
