		.386P
		.MODEL FLAT
		
		PUBLIC	_EntryPoint16, _m_GdtTable, _m_GdtrDesc
		EXTERN	_chobits_init:near

;===============================================================================
; DECLARE MACROs
;===============================================================================
LGDT16	MACRO	addr				; load from ds segment area
		db	3eh
		db	67h
		db	0fh
		db	01h
		db	15h
		dd	addr
ENDM

FJMP16	MACRO	selector, offset
		db	66h
		db	0eah
		dd	offset
		dw	selector
ENDM


.CODE
;===============================================================================
; EntryPoint called by bootsect.
;===============================================================================
_EntryPoint16		PROC	NEAR

		LGDT16	_m_GdtrDesc-20000h

		mov		eax, cr0
		or		ax, 1
		mov		cr0, eax
		jmp		$+2

		FJMP16	08h, EntryPoint32
		
_EntryPoint16		ENDP

;===============================================================================
; 32bit routine
;===============================================================================
EntryPoint32	PROC		NEAR

		mov		ax, 10h
 		mov		ds, ax
 		mov		es, ax
 		mov		fs, ax
 		mov		gs, ax
 		mov		ss, ax
 		mov		esp, 1ffffh		; top of stack

		call	_chobits_init

infinate:
		hlt
		jmp	infinate
		
EntryPoint32	ENDP


.DATA
;===============================================================================
; GDT table descriptor
;===============================================================================
				ALIGN	4

_m_GdtrDesc		dw		GDT_SIZE-1
 				dd		_m_GdtTable

;===============================================================================
; GDT table
;===============================================================================
				ALIGN	4

;----------------------------------
; GDT TABLE INDEX #0
; INDEX      : NULL SELECTOR
;----------------------------------
_m_GdtTable		dd		0
 				dd		0

;----------------------------------
; GDT TABLE INDEX #1
; INDEX      : 0x0008h
; TYPE       : CODE SEGMENT
; RING LEVEL : 0
; ATTRIBUTES : Excute/Read, Nonconforming, 4GB
;----------------------------------
				dw		0ffffh			; segment limit 15:00
				dw		0000h			; base address 15:00
				db		00h				; base address 23:16
				db		10011010b		; P | DPL | S | TYPE
				db		11001111b		; G | D/B | 0 | AVL | segment limit 19:16
				db		00h				; base address 31:24
				
;----------------------------------
; GDT TABLE INDEX #2
; INDEX      : 0x0010h
; TYPE       : DATA SEGMENT
; RING LEVEL : 0
; ATTRIBUTES : Read/Write, 4GB
;----------------------------------
				dw		0ffffh
				dw		0000h
				db		00h
				db		10010010b
				db		11001111b
				db		00h
				
;----------------------------------
; GDT TABLE INDEX #3
; INDEX      : 0x001Bh
; TYPE       : CODE SEGMENT
; RING LEVEL : 3
; ATTRIBUTES : Excute/Read, Nonconforming, second 1mega area
;----------------------------------
				dw		000ffh			; segment limit 15:00
				dw		0000h			; base address 15:00
				db		10h				; base address 23:16
				db		11111010b		; P | DPL | S | TYPE
				db		11001111b		; G | D/B | 0 | AVL | segment limit 19:16
				db		00h				; base address 31:24
				
;----------------------------------
; GDT TABLE INDEX #4
; INDEX      : 0x0023h
; TYPE       : DATA SEGMENT
; RING LEVEL : 3
; ATTRIBUTES : Read/Write, second 1mega area
;----------------------------------
				dw		000ffh
				dw		0000h
				db		10h
				db		11110010b
				db		11001111b
				db		00h
				
;----------------------------------
; GDT TABLE INDEX #5
; INDEX      : 0x0028h
; TYPE       : 32-Bit TSS
; RING LEVEL : 0
; DESC       : used by tmr_task_gate
;----------------------------------
				dw		0067h				; segment limit 15:00
				dw		0000h				; base address 15:00
				db		00h					; base address 23:16
				db		10001001b			; P | DPL | 0 | TYPE
				db		00000000b			; G | 0 | 0 | AVL | limit 19:16
				db		00h					; base address 31:24

;----------------------------------
; GDT TABLE INDEX #6
; INDEX      : 0x0030h
; TYPE       : 32-Bit TSS
; RING LEVEL : 0
; DESC       : used during system initialization
;----------------------------------
				dw		0067h				; segment limit 15:00
				dw		0000h				; base address 15:00
				db		00h					; base address 23:16
				db		10001001b			; P | DPL | 0 | TYPE
				db		00000000b			; G | 0 | 0 | AVL | limit 19:16
				db		00h					; base address 31:24
				
;----------------------------------
; GDT TABLE INDEX #7
; INDEX      : 0x0038h
; TYPE       : 32-Bit TSS
; RING LEVEL : 0
; DESC       : called whenever the system timer generates an interrupt
;----------------------------------
				dw		0067h				; segment limit 15:00
				dw		0000h				; base address 15:00
				db		00h					; base address 23:16
				db		10001001b			; P | DPL | 0 | TYPE
				db		00000000b			; G | 0 | 0 | AVL | limit 19:16
				db		00h					; base address 31:24
				
;----------------------------------
; GDT TABLE INDEX #8
; INDEX      : 0x0040h
; TYPE       : 32-Bit TSS
; RING LEVEL : 0
; DESC       : soft-task-switch. when any thread wants to task-swithing, call this.
;----------------------------------
				dw		0067h				; segment limit 15:00
				dw		0000h				; base address 15:00
				db		00h					; base address 23:16
				db		10001001b			; P | DPL | 0 | TYPE
				db		00000000b			; G | 0 | 0 | AVL | limit 19:16
				db		00h					; base address 31:24
				
;----------------------------------
; GDT TABLE INDEX #9
; INDEX      : 0x0048h
; TYPE       : 32-Bit TSS
; RING LEVEL : 0
; DESC       : call-gate for syscall
;----------------------------------
				dw		0000h				; offset 15:00
				dw		0000h				; selector
				db		01h					; dword count = 1 (4bytes)
				db		11101100b			; P, DPL=3, S=0, TYPE=1100
				dw		0000h				; offset 31:16

 
GDT_SIZE	EQU	$ - _m_GdtTable		; Size, in bytes

		END
