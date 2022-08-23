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
db  'FAT12      '

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

;========================
; Read sector
;========================
%define SECTORS_PER_TRACK       18
%define SECTORS_OF_TWO_TRACK    SECTORS_PER_TRACK * 2

proc    read_sector
        mov ax, 1
        pusha

.init:
        mov ax, word [bp+4]

        mov bl, SECTORS_OF_TWO_TRACK
        div bl  ; division with ax, bl
                ; quotient will moved to al, remainder will be moved to 
        mov ch, al      ; move track
        shr ax, 8

        ; get "head" number
        mov bl, SECTORS_PER_TRACK
        mov dl, 00h
        mov es, word[bp+6]
        mov bx, word[bp+8]
        int 13h

        popa

        jnc .out
        xor ax, ax

.out:
endproc

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

        ; read FAT & Directory table
        mov cx, 9+14
        mov bx, 0000h
        mov dx, 0ah

.read_fat_dir:
        push bx
        push FATSEG
        push dx
        call read_sector

        add sp, 6
        or ax, ax
        jnz .read_next_fat_dir
        call sys_halt

.read_next_fat_dir:
        inc dx
        add bx, 512
        loop .read_fat_dir

        mov cx, 224
        mov bx, 1400h

;=====================
; sys_halt
;=====================
proc    sys_halt
        push msg_error
        call msg_out
        add sp, 2

.infinate:
        jmp .infinate
endproc

;=====================
; is_equal
;=====================
proc    is_equal
        mov ax, 1
        pusha

        ; init
        mov si, word [bp + 4]
        mov di, word [bp + 5]
        mov cx, word [bp + 6]

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

;====================================
; called by find_image function
; if entry become "1" then call it.
;====================================
.image_found:
        mov dx, word [bx+1ah]
        mov bx, LDRSEG

%define LDRSEG  02000h

;====================================
; moving 1 by 1 at root directory
;====================================
.find_image:
        push 8
        push bx
        push str_kernel
        call is_equal
        add sp, 6
        or ax, ax
        jnz .image_found
        add bx, 32
        loop .find_image
        call sys_halt

str_kernel db 'RMOS BIN' 

;=====================================
; read cluster function
;=====================================
.read_cluster:
        cmp dx, 0ff8h
        jae .load_kernel

        push msg_dot
        call msg_out
        add sp, 2

        add dx, 01fh

        push 00h
        push bx 
        push dx
        call read_sector
        add sp, 6
        or ax, ax
        jnz .increase_ldr_addr
        call sys_halt

;==================================
; increase_ldr_addr function
;==================================
.increase_ldr_addr:
        sub dx, 01fh
        add bx, 20h     ; increase LDRSEG (512 bytes)

;==================================
; get_next_cluster function
;==================================
.get_next_cluster:
        push bx
        push dx
        mov bx, 3

        mov ax, dx
        clc
        rcr ax, 1
        jc .odd_number

        ; even_number
        mul bx
        mov bx, ax
        mov ax, word [bx+200h]
        and ax, 0fffh 
        pop dx
        pop bx
        mov dx, ax
        jmp .read_cluster

;====================================
; Odd number
;====================================
.odd_number:
        mul bx
        mov bx, ax
        inc bx
        mov ax, word [bx + 200h]
        shr ax, 4
        pop dx
        pop bx 
        mov dx, ax 
        jmp .read_cluster

;===================================
; load kernel function
;===================================
.load_kernel:
        cli
        mov ax, LDRSEG
        mov ds, ax
        jmp LDRSEG:1000h
                        ; jump to RMOS kernel

;===================================
; print message as error
;===================================
msg_error db 'Error!', 00h

;===================================
; print message as starting Loading
;===================================
msg_loading     db      'Starting RMOS...', 00h 

;-------------------
; msg_dot
;-------------------
msg_dot         db      '.', 00h

;-------------------
; boot signature
;-------------------
times   1feh-($-$$) db  00h
                    dw  0aa55h