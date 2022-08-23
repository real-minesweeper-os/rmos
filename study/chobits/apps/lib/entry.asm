		.386P
		.MODEL FLAT
		
		EXTERN	_main:near, _API_ExitProgram:near
		PUBLIC	_AppEntryPoint
		
SYSCALL_GATE	equ		48h

.CODE
;===============================================================================
; Application Entry Point
;===============================================================================
_AppEntryPoint		PROC	NEAR

		call		_main
		call		_API_ExitProgram
		
$infinate:
		jmp			$infinate
		
_AppEntryPoint		ENDP

.DATA
callgate	dd 00000000h
			dw SYSCALL_GATE

		END
