mov ah, 0x0e

mov bp, 0x8000
mov sp, bp

push 'A'
push 'B'
push 'C'

pop bx
mov al, bl

pop bx
mov cl, bl

pop bx
mov dl, bl

mov al, dl
int 0x10

times 510-($-$$) db 0
dw 0xaa55