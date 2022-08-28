mov al, 'E' ; al == 'E' 
            ; mov al, 'H'
cmp al, 'H' ; compare with al and 'H'
            ; It's means check the letter is same with al and 'H' letter.
            ; But now we are allocated al as 'E'.   
je print_function

mov al, 'A'
int 0x10

return_here:
    mov al, 'D'
    int 0x10
    ret 0x0e

print_function:
    mov ah, 0x0e
    int 0x10
    jmp return_here

jmp $

times 510-($-$$) db 0
dw 0xaa55