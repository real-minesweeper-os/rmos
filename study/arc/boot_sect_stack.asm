mov ah, 0x0e

mov bp, 0x8000
mov sp, bp 

push 'A'
push 'B'
push 'C'

; 0x8000 - 2 = 0x7ffe
; print 0x7ffe 
mov al, [0x7ffe]
int 0x10 

; print top of stack
mov al, [0x8000]
int 0x10

pop bx
mov al, bl
int 0x10

pop bx
mov al, bl
int 0x10

pop bx
mov al, bl
int 0x10

mov al, [0x8000]
int 0x10

jmp $
times 510-($-$$) db 0
dw 0xaa55