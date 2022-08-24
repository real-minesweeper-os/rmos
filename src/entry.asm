.368p
.MODEL FLAT

PUBLIC _EntryPoint16, _m_GdtTable, _m_GdtrDesc
EXTERN _rmos_init:near

.CODE 
;==================================
; EntryPoint called by bootsector
;==================================
_EntryPoint16   PROC NEAR
    LGDT16 _m_GdtrDesc-20000h
    mov eax, cr0
    or ax, 1
    mov cr0, eax
    jmp $+2

    FJMP16 08h, EntryPoint32

_EntryPoint16   ENDP

;=====================================
; GDT table
;=====================================
    ALIGN4

;-------------------------------------
; GDT Table index #0
;-------------------------------------
_m_GdtTable     dd      0
                dd      0

;-------------------------------------
; GDT Table index #1
; Index : 0x0008h
; Type  : CODE SEGMENT
; Ring level : 0
; Attributes : Excute/Read, Nonconforming, 4GB
;-------------------------------------
    dw 0ffffh
    dw 0000h
    db 00h 
    db 10011010b
    db 11001111b
    db 00h

;-------------------------------------
; GDT Table Index #2
; Index : 0x0010h
; Type  : DATA SEGMENT
; Ring level : 0
; Attributes : Read/Write, 4GB
;-------------------------------------
    dw 0ffffh
    dw 0000h 
    db 00h 
    db 10010010b 
    db 11001111b
    db 00h 

;-------------------------------------
; GDT Table Index #3
; Index : 0x001Bh
; Type  : CODE SEGMENT
; Ring level : 3
; Attribute  : Excute/Read, Nonconforming, second Imege area
;-------------------------------------
    dw 000ffh
    dw 0000h 
    db 10h
    db 11110010b 
    db 11001111b
    db 00h 

;--------------------------------------
; GDT Table Index : #4
; Index : 0x0023h
; Type  : DATA SEGMENT
; Ring level : 3
; Attributes : Read/Write, second Imege area
;---------------------------------------
    dw 000ffh 
    dw 0000h 
    db 10h 
    db 11110010b
    db 11001111b
    db 00h 

;---------------------------------------
; GDT Table Index : #5
; Index : 0x0028h
; Type  : 32-bit TSS
; Ring level : 0
; Desc       : used by tmr_task_gate
;----------------------------------------
    db 0067h
    dw 0000h 
    db 00h 
    db 10001001b
    db 00000000b
    db 00h

;-----------------------------------------
; GDT Table Index #6
; Index : 0x0030h
; Type  : 32-bit TSS
; Ring level : 0 
; DESC       : used during system initialization
;------------------------------------------
    dw 0067h
    dw 0000h
    db 00h 
    db 10001001b
    db 00000000b
    db 00h 

;--------------------------------------------
; GDT Table Index #7
; Index : 0x0038h
; Type  : 32-Bit TSS
; Ring level : 0
; DESC       : called whenever the system timer generates an interrupt
;---------------------------------------------
    dw 0067h 
    dw 0000h 
    db 00h 
    db 10001001b
    db 00000000b
    db 00h 

;---------------------------------------------
; GDT Table Index #8
; Index : 0x0040h
; Type  : 32-Bit TSS
; Ring level : 0
; DESC       : soft-task-switch. when any thread wants to task-switching, call this
;----------------------------------------------
    dw 0067h
    dw 0000h
    db 00h 
    db 10001001b 
    db 00000000b
    db 00h 

;-----------------------------------------------
; GDT Table Index #9
; Index : 0x0048h
; Type  : 32-Bit TSS
; Ring level : 0
; Desc       : call-gate for syscall
;------------------------------------------------
    dw 0000h
    dw 0000h 
    db 01h 
    db 11101100b 
    dw 0000h 