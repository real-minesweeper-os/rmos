mov dx, 0x1fb6
call print_hex

print_hex:
    mov bx, HEX_OUT
    call print_string
    ret

HEX_OUT:
    db '0x0000', 0