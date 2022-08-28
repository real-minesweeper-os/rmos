gdt_start:

; the mandatory null descripition
gdt_null:
    dd 0x0  ; 'dd' means define dobule words -> 4bytes
    dd 0x0

; the code segement descriptor
;-------------------------------
; base code : 0x0
; limit : 0xfffff
; First flag :     (present)1      (privilege)00 (descriptior type)1        ->  1001b
;  type flag :        (code)1     (conforming)0 (readable)1 (accessed)0     ->  1010b
; Second flag: (granularity)1 (32-bit default)1 (64-bit segment)0 (AVL)0    ->  1100b
;-------------------------------
gdt_code:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

; GDT Descripition
gdt_descriptior:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start