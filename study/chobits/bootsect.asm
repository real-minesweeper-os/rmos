;===============================================================================
; bootsect.asm - for FAT12 disk filesystem.
;
; First written = Aug 5th, 2003.
; Writer = Yeori (alphamcu@hanmail.net, http://www.zap.pe.kr)
;===============================================================================

	[BITS 16]
	ORG	0
	
	%include 'c16.mac'


;===============================================================================
; boot records (FAT12 compatible)
;===============================================================================
	db		0ebh, 3ch				; jmp entry
	db		90h						; nop		- for assembler independence
		
	db		'Chobits '				; 8 bytes
	dw		512						; bytes per sector
	db		1						; sectors per cluster
	dw		1						; reserved sectors
	db		2						; numbers of FAT
	dw		224						; entries of root directory
	dw		2880					; total sectors
	db		0f0h
	dw		9						; sectors per FAT
	dw		18						; sectors per track
	dw		2						; heads
	dw		0						; numbers of hidden sectors
	dw		0
	dd		0
	db		0						; disk driver number (A:0, C:2)
	db		0
	db		029h
	dd		0
	db		'CHOBITS BIN'
	db		'FAT12   '
		

;===============================================================================
; main
;
; * description - read carefully - *
; Only "conventional memory" area(00000h~9ffffh) is available freely. First 1k
; memory(from 000h~3ffh) is set aside for the "interrupt vector table". The area
; from 400h to 4ffh(a total of 256 bytes) is set aside for the "BIOS data" area.
; Memory from 500h to 5ffh(256 bytes) is set aside to be used for "DOS" parameters.
; Locations 700h to 9ffffh are available to "ANY OS".
;
; But, we only use the area from 1000h to 9ffffh area(a total of 650kbytes).
; first 512bytes(1000h~11ffh) is used for a copy of "boot sector". Next 4k memory
; from(1200h~23ffh) is used for a copy of "FAT" table. The area from 2400h to
; 3fffh(7kbytes) is used for "Directory Entry" table. Memory from 10000h to 1ffffh
; (64kbytes) is used for the "stack" area. Finally, the area from 20000h to 9ffffh
; (588kbytes) is set aside to be used for launching "CHOBITS OS" image file.
;===============================================================================
%define BOOTSEG		07c0h		; 7c00h
%define INITSEG		0100h		; 1000h
%define INITSS		01000h		; 10000h
%define INITSP		0ffffh

%define	FATSEG		0120h		; 1200h
%define DIRSEG		0240h		; 2400h
%define LDRSEG		02000h		; 20000h ~ 9ffffh

entry:
		cli
		mov		ax, INITSS
		mov		ss, ax
		mov		sp, INITSP
		sti
		
		; re-load
		cld
		mov		ax, INITSEG
		mov		es, ax
		xor		di, di
		mov		ax, BOOTSEG
		mov		ds, ax
		xor		si, si
		mov		cx, 100h				; 256 * 2bytes(movsw) = 512 bytes
		repz	movsw
		
		jmp		INITSEG:main
		

;===============================================================================
; main procedure
;===============================================================================
main:
		mov		ax, INITSEG
		mov		ds, ax
		
		; init video
		mov		ax, 0600h
		mov		bh, 07h
		xor		cx, cx
		mov		dx, 1850h		; row:24, column:80
		int		10h				; reset window
		mov		ah, 02h
		xor		bh, bh
		xor		dx, dx
		int		10h				; set cursor position onto the top-left
		
		; loading msg
		push	msg_loading
		call	msgout
		add		sp, 2
		
		; reset disk system
		xor		ah, ah
		mov		dl, 00h			; a:
		int		13h
		
		; read FAT & Directory table
		mov		cx, 9+14		; sizeof FAT sectors & Directory table
		mov		bx, 0000h		; start address of buffer in FATSEG
		mov		dx, 0ah			; start address of FAT2
		
.read_fat_dir:	
		push	bx
		push	FATSEG
		push	dx
		call	read_sector		; read sector one by one
		add		sp, 6
		or		ax, ax
		jnz		.read_next_fat_dir			; check whether or not an error occured
		call	sys_halt

.read_next_fat_dir:
		inc		dx
		add		bx, 512
		loop	.read_fat_dir	; read next sector
		
		; find chobits os image file from directory table
		mov		cx, 224
		mov		bx, 1400h
		
.find_image:
		push	8
		push	bx
		push	str_kernel
		call	is_equal
		add		sp, 6
		or		ax, ax
		jnz		.image_found
		add		bx, 32
		loop	.find_image
		call	sys_halt		; no file found
		
.image_found:
		mov		dx, word [bx+1ah]	; get cluster number
		mov		bx, LDRSEG			; initial segment value
		
.read_cluster:						; using bx(LDRSEG), dx(cluster)
		cmp		dx, 0ff8h
		jae		.load_kernel
		
		push	msg_dot				; print progress
		call	msgout
		add		sp, 2

		add		dx, 01fh			; offset to real data area
		push	00h
		push	bx
		push	dx
		call	read_sector
		add		sp, 6
		or		ax, ax
		jnz		.increase_ldr_addr
		call	sys_halt
		
.increase_ldr_addr:
		sub		dx, 01fh
		add		bx, 20h				; increase LDRSEG(512bytes)
		
.get_next_cluster:
		push	bx
		push	dx
		mov		bx, 3
		
		mov		ax, dx
		clc
		rcr		ax, 1
		jc		.odd_number
		
		; even number
		mul		bx
		mov		bx, ax
		mov		ax, word [bx+200h]		; 200h = DIR's offset from FAT2
		and		ax, 0fffh
		pop		dx
		pop		bx
		mov		dx, ax
		jmp		.read_cluster
		
.odd_number:
		mul		bx
		mov		bx, ax
		inc		bx
		mov		ax, word [bx+200h]
		shr		ax, 4
		pop		dx
		pop		bx
		mov		dx, ax
		jmp		.read_cluster
		
.load_kernel:
		cli
		mov		ax, LDRSEG
		mov		ds, ax
		jmp		LDRSEG:1000h			; jump to Chobits OS
		

;===============================================================================
; sys_halt
;===============================================================================
proc	sys_halt
		push	msg_error
		call	msgout
		add		sp, 2
		
.infinate:
		jmp		.infinate
endproc


;===============================================================================
; is_equal
;
; type : int is_equal(int first_str, int second_str, int count);
; desc : if two strings are not euqal, return zero value
;===============================================================================
proc	is_equal
		mov		ax, 1
		pusha
		
		; init
		mov		si, word [bp+4]
		mov		di, word [bp+6]
		mov		cx, word [bp+8]
		
		; proceed
.loop:
		lodsb
		cmp		al, byte [di]
		jne		.different
		inc		di
		loop	.loop
		
		popa
		jmp		.out

.different:
		popa
		xor		ax, ax
.out:
endproc

;===============================================================================
; read_sector
;
; type : int read_sector(int sector_number, int es_value, int bx_value);
; desc : sector_number - initial number is zero not one!
;        if an error occurred, return zero value
;===============================================================================

%define SECTORS_PER_TRACK			18
%define SECTORS_OF_TWO_TRACKS		SECTORS_PER_TRACK*2

proc	read_sector
		mov		ax, 1				; no error
		pusha
		
		; init variables.
.init:
		mov		ax, word [bp+4]
		
		; get "track" number.
		mov		bl, SECTORS_OF_TWO_TRACKS
		div		bl
		mov		ch, al							; track
		shr		ax, 8

		; get "head" number.
		mov		bl, SECTORS_PER_TRACK
		div		bl
		mov		dh, al							; head
		mov		cl, ah
		inc		cl								; add 1 to sector number.

		; read sector
		mov		ax, 0201h
		mov		dl, 00h							; floppy drive number
		mov		es, word [bp+6]
		mov		bx, word [bp+8]					; es:bx
		int		13h
		
		popa
		
		jnc		.out
		xor		ax, ax							; error occurred
.out:
endproc

;===============================================================================
; msgout
;
; type : void msgout(word ptMsg);  // ptMsg is an word-size address.
;===============================================================================
proc	msgout
		pusha
		
		mov		si, WORD [bp+4]
.print:
		lodsb
		or		al, al
		jz		.out
		mov		ah, 0eh					; teletype output function
		mov		bx, 08h					; back/fore ground color
		int		10h
		jmp		.print
		
.out:
		popa
endproc


;===============================================================================
; datas used by boot procedure
;===============================================================================
msg_loading	db	'Loading', 00h
msg_dot		db	'.', 00h
msg_error	db	'Err!', 00h
str_kernel	db	'CHOBITS BIN'


;===============================================================================
; boot signature
;===============================================================================
times	1feh-($-$$)	db	00h
		dw		0aa55h