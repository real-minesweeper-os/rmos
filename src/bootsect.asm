[BITS 16]
ORG 0

%include 'c16.mac'

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

; ====================
; main bootsector code
; =====================

%define BOOTSEG 07c0h
%define INITSEG 0100h
%define INITSS  01000h
%define INITSP  0ffffh

%define FATSEG  0120h
%define DIRSEG  0240h
%define LDRSEG  02000h

entry:
        cli
        mov ax, INITSS
        mov ss, ax
        mov sp, INITSP
        sti

        ; re-load
        cld
        mov ax, INITSEG
        mov es, ax
        xor di, di
        mov ax, BOOTSEG
        mov ds, ax
        xor si, si
        mov cx, 100h
        repz    movsw

        jmp INITSEG:main

main:
        mov ax, INITSEG
        mov ds, ax

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

        push msg_loading
        call msg_out
        add sp, 2

        xor ah, ah
        mov dl, 00h
        int 13h

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

;===================================
; print message as starting Loading
;===================================
msg_loading     db      'Welcome to "REAL-MINESWEEPERS OPERATING SYSTEM".', 00h
;                db      'Starting Loading...', 00h 

;-------------------
; boot signature
;-------------------
times   1feh-($-$$) db  00h
                    dw  0aa55h