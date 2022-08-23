#include "chobits.h"
#include "sys_desc.h"
#include "syscall_type.h"

/*
 * DEFINITIONS
 */
#define DEFAULT_STACK_SIZE			(64*1024) /* 64kbytes */

/*
 * GLOBAL FUNCTIONS
 */
BOOL SysInitializeSyscall(VOID);
VOID *SysGetSyscallStackPtr(VOID);
DWORD SysGetSyscallStackSize(VOID);

/*
 * INTERNEL FUNCTIONS
 */
static BOOL SyspSetupSysCallgate(void);

/*
 * GLOBAL VARIABLES
 */
extern CALLGATE_DESC m_GdtTable[NUMBERS_OF_GDT_ENTRIES];
static BYTE *m_pSyscallStack;

/**********************************************************************************************************
 *                                             GLOBAL FUNTIONS                                            *
 **********************************************************************************************************/
BOOL SysInitializeSyscall(VOID)
{
	if(!SyspSetupSysCallgate()) {
		DbgPrint("SyspSetupSysCallgate() returned an error.\r\n");
		return FALSE;
	}

	/* setup stack for syscall */
	m_pSyscallStack = MmAllocateNonCachedMemory(DEFAULT_STACK_SIZE);
	if(m_pSyscallStack == NULL) return FALSE;

	return TRUE;
}

VOID *SysGetSyscallStackPtr(VOID)
{
	return (VOID *)m_pSyscallStack;
}

DWORD SysGetSyscallStackSize(VOID)
{
	return DEFAULT_STACK_SIZE;
}


/**********************************************************************************************************
 *                                           INTERNAL FUNTIONS                                            *
 **********************************************************************************************************/
_declspec(naked)  static VOID Sysp_SERVICE_CALL_MANAGER(void)
{
	static PSYSCALL_MSG call_msg;
	static KBD_KEY_DATA key_data;
	static int result=0;

	_asm {
		mov		eax, [esp+8]
		mov		call_msg, eax

		pushad
	}

	switch(call_msg->syscall_type) {
		/* system api */
		case SYSCALL_TERMINATED:
			PsSetThreadStatus(PsGetCurrentThread(), THREAD_STATUS_TERMINATED);
			HalTaskSwitch();
			break;
		case SYSCALL_DELAY:
			KeDelay(call_msg->parameters.DELAY.milli_sec);
			break;
		case SYSCALL_GET_TICKCOUNT:
			result = (int)PsGetTickCount();
			break;
		case SYSCALL_SHOW_TSWATCHDOG:
			PsShowTSWachdogClock(TRUE);
			break;
		case SYSCALL_HIDE_TSWATCHDOG:
			PsShowTSWachdogClock(FALSE);
			break;

		/* screen api */
		case SYSCALL_CLEAR_SCREEN:
			CrtClearScreen();
			break;
		case SYSCALL_PRINT_TEXT:
			CrtPrintText(call_msg->parameters.PRINT_TEXT.pt_text);
			break;
		case SYSCALL_PRINT_TEXT_XY:
			CrtPrintTextXY(call_msg->parameters.PRINT_TEXT.pt_text, 
				call_msg->parameters.PRINT_TEXT.x, call_msg->parameters.PRINT_TEXT.y);
			break;
		case SYSCALL_PRINT_TEXT_ATTR:
			CrtPrintTextWithAttr(call_msg->parameters.PRINT_TEXT.pt_text, call_msg->parameters.PRINT_TEXT.attr);
			break;
		case SYSCALL_PRINT_TEXT_XY_ATTR:
			CrtPrintTextXYWithAttr(call_msg->parameters.PRINT_TEXT.pt_text, 
				call_msg->parameters.PRINT_TEXT.x, call_msg->parameters.PRINT_TEXT.y,
				call_msg->parameters.PRINT_TEXT.attr);
			break;

		/* keyboard api */
		case SYSCALL_HAS_KEY:
			if(KbdHasKey())
				result = 1;
			else
				result = 0;
			break;
		case SYSCALL_GET_KEYDATA:
			if(!KbdGetKey(&key_data)) {
				result = 0xffffffff;
				break;
			}
			result = (int)key_data.type;
			result <<= 8;
			result += (int)key_data.key;
			break;

		default:
			break;
	}
	
	_asm {
		popad
		mov			eax, result
		ret			4
	}
}

static BOOL SyspSetupSysCallgate(void)
{
ENTER_CRITICAL_SECTION();
	m_GdtTable[SYSCALL_GATE>>3].count		= 1;
	m_GdtTable[SYSCALL_GATE>>3].type		= 0xec; /* 11101100 */
	m_GdtTable[SYSCALL_GATE>>3].selector	= KERNEL_CS;
	m_GdtTable[SYSCALL_GATE>>3].offset_1	= (BYTE) (((int)Sysp_SERVICE_CALL_MANAGER) & 0x000000ff);
	m_GdtTable[SYSCALL_GATE>>3].offset_2	= (BYTE)((((int)Sysp_SERVICE_CALL_MANAGER) & 0x0000ff00) >> 8);
	m_GdtTable[SYSCALL_GATE>>3].offset_3	= (BYTE)((((int)Sysp_SERVICE_CALL_MANAGER) & 0x00ff0000) >> 16);
	m_GdtTable[SYSCALL_GATE>>3].offset_4	= (BYTE)((((int)Sysp_SERVICE_CALL_MANAGER) & 0xff000000) >> 24);
EXIT_CRITICAL_SECTION();

	return TRUE;
}