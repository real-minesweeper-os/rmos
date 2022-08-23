#ifndef _CHOBITS_API_H_
#define _CHOBITS_API_H_

/*
 * INCLUDE FILES
 */
#include "../../types.h"
#include "../../syscall_type.h"
#include "../../key_def.h"			/* key definitions */
#include "../../string.h"			/* import string library : strlen, etc... */
#include "../../vsprintf.h"			/* sprintf, vsprintf */

/*
 * API CALLs
 */

/* system api */
VOID API_ExitProgram(VOID);
DWORD API_GetTickCount(VOID);
VOID API_Delay(DWORD MilliSec); /* 1000 = 1sec */
VOID API_ShowTSWatchdogClock(VOID);
VOID API_HideTSWatchdogClock(VOID);

/* screen associated */
VOID API_ClearScreen(VOID);
VOID API_PrintText(BYTE *pText);
VOID API_PrintTextXY(BYTE *pText, WORD x, WORD y);
VOID API_PrintTextWithAttr(BYTE *pText, BYTE Attr);
VOID API_PrintTextXYWithAttr(BYTE *pText, WORD x, WORD y, BYTE Attr);

/* keyboard api */
BOOL API_HasKey(VOID);
BOOL API_GetKey(KBD_KEY_DATA *pKeyData);
VOID API_FlushKbdBuf(VOID);


#endif /* #ifndef _CHOBITS_API_H_ */