[BITS 16]
ORG 0

%include 'c16.mac'

;======================
; Message out function
;======================
proc    msg_out
        pusha

        mov si, WORD [bp+4]

.print:
        lodsb
        or  al, al
        jz  .out
        mov ah, 0eh
        mov bx, 08h
        int 10h
        jmp .print

.out:
        popa
endproc

;======================
; Boot records
;======================

db  0ebh, 3ch
db  90h

db  'RMOS'
dw  512
db  1  
db  1
db  2
dw  224
dw  2280
db  0f0h
dw  9
dw  18
dw  2
dw  0
dw  0
dd  0
db  0
db  0
db  029h
dd  0
db  'RMOS BIN'
db  'FAT12'

; init video
mov ax, 0600h
mov bh, 07h
xor cx, cx
mov dx, 1850h   ; row: 24,
                ; columns: 80
int 10h
mov ah, 02h
xor bh, bh
xor dx, dx
int 10h

;------------------
; loading message
;------------------
push msg_loading
call msg_out
add sp, 2

; reset disk system
xor ah, ah
mov dl, 00h
int 13h

msg_loading db  'Loading...', 00h

;-------------------
; boot signature
;-------------------
times   1feh-($-$$) db  00h
                    dw  0aa55h