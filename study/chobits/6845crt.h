#ifndef _6845_CRT_DRIVER_HEADER_FILE_
#define _6845_CRT_DRIVER_HEADER_FILE_


/**********************************************************************************************************
 *                                                INCLUDE FILES                                           *
 **********************************************************************************************************/
#include "chobits.h"


/**********************************************************************************************************
 *                                               DIRVER FUNCTIONS                                         *
 **********************************************************************************************************/
KERNELAPI VOID CrtClearScreen(VOID);
KERNELAPI VOID CrtGetCursorPos(BYTE *pX, BYTE *pY);

KERNELAPI BOOL CrtPutChar(BYTE ch);
KERNELAPI BOOL CrtPutCharXY(BYTE ch, WORD x, WORD y);

KERNELAPI BOOL CrtPrintText(LPCSTR pText);
KERNELAPI BOOL CrtPrintTextXY(LPCSTR pText, WORD x, WORD y);
KERNELAPI BOOL CrtPrintTextWithAttr(LPCSTR pText, UCHAR Attr);
KERNELAPI BOOL CrtPrintTextXYWithAttr(LPCSTR pText, WORD x, WORD y, UCHAR Attr);

KERNELAPI int  CrtPrintf(const char *fmt, ...); /* by using standard c library */
KERNELAPI int  CrtPrintfXY(WORD x, WORD y, const char *fmt, ...);
KERNELAPI int  CrtPrintfWithAttr(UCHAR Attr, const char *fmt, ...);
KERNELAPI int  CrtPrintfXYWithAttr(UCHAR Attr, WORD x, WORD y, const char *fmt, ...);


#endif /* #ifndef _6845_CRT_DRIVER_HEADER_FILE_ */