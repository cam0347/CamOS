gdt_start:

gdt_null:
  double1: dd 0x0
  double2: dd 0x0

gdt_code:
  cs_limit1: dw 0xFFFF      ;limit
  cs_base1: dw 0x0000       ;base
  cs_base2: db 0x00         ;base
  cs_access: db 10011010b   ;access
  cs_flags_limit2: db 0xCF  ;flags and limit
  cs_base3: db 0x00         ;base

gdt_data:
  ds_limit1: dw 0xFFFF ;limit
  ds_base1: dw 0x0000 ;base
  ds_base2: db 0x00   ;base
  ds_access: db 0x92   ;access
  ds_flags_limit2: db 0xCF   ;flags and limit
  ds_base3: db 0x00   ;base

gdt_end:

gdt_descriptor:
  dw gdt_end - gdt_start - 1
  dd gdt_start


CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
