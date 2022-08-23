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