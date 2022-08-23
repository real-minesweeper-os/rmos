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