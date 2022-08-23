#include "chobits_api.h"
#include "../../string.c" /* string library */
#include "../../vsprintf.c" /* sprintf ~~~ */

static int internel_syscall(SYSCALL_MSG *pSyscall);

/**********************************************************************************************************
 *                                            SYSTEM API CALLS                                            *
 **********************************************************************************************************/
VOID API_ExitProgram(VOID)
{
	SYSCALL_MSG syscall;

	syscall.syscall_type = SYSCALL_TERMINATED;
	internel_syscall(&syscall);

	while(1) ;
}

DWORD API_GetTickCount(VOID)
{
	DWORD tick;
	SYSCALL_MSG syscall;

	syscall.syscall_type = SYSCALL_GET_TICKCOUNT;
	tick = (DWORD)internel_syscall(&syscall);

	return tick;
}

VOID API_Delay(DWORD MilliSec)
{
	SYSCALL_MSG syscall;

	syscall.syscall_type = SYSCALL_DELAY;
	syscall.parameters.DELAY.milli_sec = MilliSec;
	internel_syscall(&syscall);
}

VOID API_ShowTSWatchdogClock(VOID)
{
	SYSCALL_MSG syscall;

	syscall.syscall_type = SYSCALL_SHOW_TSWATCHDOG;
	internel_syscall(&syscall);
}

VOID API_HideTSWatchdogClock(VOID)
{
	SYSCALL_MSG syscall;

	syscall.syscall_type = SYSCALL_HIDE_TSWATCHDOG;
	internel_syscall(&syscall);
}


/**********************************************************************************************************
 *                                             SCREEN API CALLS                                           *
 **********************************************************************************************************/
VOID API_ClearScreen(VOID)
{
	SYSCALL_MSG syscall;

	syscall.syscall_type = SYSCALL_CLEAR_SCREEN;
	internel_syscall(&syscall);
}

VOID API_PrintText(BYTE *pText)
{
	SYSCALL_MSG syscall;

	if(pText == NULL) return;

	syscall.syscall_type = SYSCALL_PRINT_TEXT;
	syscall.parameters.PRINT_TEXT.pt_text = pText;
	internel_syscall(&syscall);
}

VOID API_PrintTextXY(BYTE *pText, WORD x, WORD y)
{
	SYSCALL_MSG syscall;

	if(pText == NULL) return;

	syscall.syscall_type = SYSCALL_PRINT_TEXT_XY;
	syscall.parameters.PRINT_TEXT.pt_text = pText;
	syscall.parameters.PRINT_TEXT.x = x;
	syscall.parameters.PRINT_TEXT.y = y;
	internel_syscall(&syscall);
}

VOID API_PrintTextWithAttr(BYTE *pText, BYTE Attr)
{
	SYSCALL_MSG syscall;

	if(pText == NULL) return;

	syscall.syscall_type = SYSCALL_PRINT_TEXT_ATTR;
	syscall.parameters.PRINT_TEXT.pt_text = pText;
	syscall.parameters.PRINT_TEXT.attr = Attr;
	internel_syscall(&syscall);
}

VOID API_PrintTextXYWithAttr(BYTE *pText, WORD x, WORD y, BYTE Attr)
{
	SYSCALL_MSG syscall;

	if(pText == NULL) return;

	syscall.syscall_type = SYSCALL_PRINT_TEXT_XY_ATTR;
	syscall.parameters.PRINT_TEXT.pt_text = pText;
	syscall.parameters.PRINT_TEXT.x = x;
	syscall.parameters.PRINT_TEXT.y = y;
	syscall.parameters.PRINT_TEXT.attr = Attr;
	internel_syscall(&syscall);
}


/**********************************************************************************************************
 *                                            KEYBOARD API CALLS                                          *
 **********************************************************************************************************/
BOOL API_HasKey(VOID)
{
	BOOL result;
	SYSCALL_MSG syscall;

	syscall.syscall_type = SYSCALL_HAS_KEY;
	result = (BOOL)internel_syscall(&syscall);

	return result;
}

BOOL API_GetKey(KBD_KEY_DATA *pKeyData)
{
	int result;
	SYSCALL_MSG syscall;

	syscall.syscall_type = SYSCALL_GET_KEYDATA;
	result = internel_syscall(&syscall);

	if(result == 0xffffffff) return FALSE;
	
	pKeyData->key = (BYTE)(result & 0x000000ff);
	pKeyData->type = (BYTE)((result & 0x0000ff00) >> 8);

	return TRUE;
}

VOID API_FlushKbdBuf(VOID)
{
	KBD_KEY_DATA key;

	while(API_GetKey(&key)) ;
}


/**********************************************************************************************************
 *                                           INTERNAL FUNTIONS                                            *
 **********************************************************************************************************/
static int internel_syscall(SYSCALL_MSG *pSyscall)
{
#define SYSCALL_GATE			0x0048
	WORD syscall_ptr[3];

	memset(syscall_ptr, 0, sizeof(WORD)*3);
	syscall_ptr[2] = SYSCALL_GATE;

	_asm {
		push		pSyscall
		call		fword ptr syscall_ptr
		add			esp, 4
	}
}