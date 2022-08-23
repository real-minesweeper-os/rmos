mov bx, 30

cmp bx, 4
jle if_block
jmp:
    jl else_if_block 
mov al, 'C'

if_block:
    mov al, 'A'

else_if_block:
    mov al, 'B'

mov ah, 0x10
int 0x10

jmp $

times 510-($-$$) db 0
dw 0xaa55