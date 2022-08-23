		.386P
		.MODEL FLAT
		PUBLIC	_READ_PORT_UCHAR, _WRITE_PORT_UCHAR
		PUBLIC	_HalpEnableInterrupt
		
SOFT_TASK_SW_INT_NUMBER	equ 30h
		
.DATA

.CODE
;===============================================================================
; UCHAR READ_PORT_UCHAR(IN PUCHAR Port);
;===============================================================================
_READ_PORT_UCHAR	PROC
		push	ebp
		mov		ebp, esp
		push	edx
		
		mov		edx, [ebp+8]
		in		al, dx
		
		pop		edx
		pop		ebp
		ret
_READ_PORT_UCHAR	ENDP

;===============================================================================
; VOID WRITE_PORT_UCHAR(IN PUCHAR Port, IN UCHAR Value);
;===============================================================================
_WRITE_PORT_UCHAR	PROC
        push	ebp
        mov		ebp, esp
        push	edx
        push	eax

        mov		edx, [ebp+8]
        mov		al, [ebp+12]
        out		dx, al
        
        pop		eax
        pop		edx
        pop		ebp
        ret
_WRITE_PORT_UCHAR	ENDP

;===============================================================================
; VOID HalpEnableInterrupt(PIDTR_DESC idtr);
;===============================================================================
_HalpEnableInterrupt	PROC
		push	ebp
		mov		ebp, esp
		push	eax
		
		mov		eax, [ebp+8]
		lidt	fword ptr [eax]

		pop		eax		
		pop		ebp
		ret
_HalpEnableInterrupt	ENDP

;===============================================================================
; VOID HalTaskSwitch(VOID);
;===============================================================================
_HalTaskSwitch	PROC
		int		SOFT_TASK_SW_INT_NUMBER
		ret
_HalTaskSwitch	ENDP

        END