#include "6845crt.h"

/*
 * DECLARES DEFINITIONS USED BY 6845 CRT DRIVER
 */
#define SCREEN_CX				80
#define SCREEN_CY				25
#define VIDEO_BUFFER_ADDRESS	0xb8000
#define VIDEO_BUFFER_SIZE		((SCREEN_CX*SCREEN_CY)*2)
#define DEFAULT_ATTRIBUTE		7

#define KEY_LF					'\n'
#define KEY_CR					'\r'
#define KEY_TAB					'\t'
#define DEFAULT_TAB_SIZE		4

/*
 * GLOBAL VARIABLES USED BY 6845 CRT DRIVER
 */
static WORD m_CursorXPos, m_CursorYPos;

/*
 * GLOBAL FUNCTIONS
 */
BOOL CrtInitializeDriver(VOID);

/*
 * DECLARES INTERNEL FUNCTIONS USED BY 6845 CRT DRIVER
 */
static void CrtpSetCursorPos(WORD x, WORD y);
static void CrtpUpdateCursorPosByPtr(UCHAR *pPtr);
static UCHAR* CrtpGetNextVideoPtr(void);
static UCHAR* CrtpGetNextVideoPtrWithPos(WORD x, WORD y);
static void CrtpKeyLF(UCHAR **pVideoPtr);
static void CrtpKeyCR(UCHAR **pVideoPtr);
static void CrtpKeyBackspace(UCHAR **pVideoPtr);
static void CrtpScrollOneLine(void);
static int CrtpPrintfFmt(UCHAR Attr, WORD x, WORD y, const char *fmt, va_list args);


/**********************************************************************************************************
 *                                             GLOBAL FUNTIONS                                            *
 **********************************************************************************************************/
BOOL CrtInitializeDriver(VOID)
{
	CrtClearScreen();
	return TRUE;
}


/**********************************************************************************************************
 *                                           EXPORTED FUNTIONS                                            *
 **********************************************************************************************************/
KERNELAPI VOID CrtClearScreen(VOID)
{
	UCHAR* pVideoBuf;
	
	pVideoBuf=(UCHAR *)VIDEO_BUFFER_ADDRESS;
	while(pVideoBuf < (UCHAR *)(VIDEO_BUFFER_ADDRESS+VIDEO_BUFFER_SIZE)) {
		*pVideoBuf++=' ';
		*pVideoBuf++=DEFAULT_ATTRIBUTE;
	}

	CrtpSetCursorPos(0, 0);
}

KERNELAPI VOID CrtGetCursorPos(BYTE *pX, BYTE *pY)
{
/* ENTER_CRITICAL_SECTION(); */
	*pX = (BYTE)m_CursorXPos;
	*pY = (BYTE)m_CursorYPos;
/* EXIT_CRITICAL_SECTION(); */
}

KERNELAPI BOOL CrtPutChar(BYTE ch)
{
	BYTE str[2];

	str[0]=ch;
	str[1]='\0';

	return CrtPrintTextXYWithAttr(str, 0xffff, 0xffff, DEFAULT_ATTRIBUTE);
}

KERNELAPI BOOL CrtPutCharXY(BYTE ch, WORD x, WORD y)
{
	BYTE str[2];

	str[0]=ch;
	str[1]='\0';

	return CrtPrintTextXYWithAttr(str, x, y, DEFAULT_ATTRIBUTE);
}

KERNELAPI int CrtPrintf(const char *fmt, ...)
{
	int i;
	va_list args;

	va_start(args, fmt);
	i=CrtpPrintfFmt(DEFAULT_ATTRIBUTE, 0xffff, 0xffff, fmt, args);
	va_end(args);

	return i;
}

KERNELAPI int  CrtPrintfXY(WORD x, WORD y, const char *fmt, ...)
{
	int i;
	va_list args;

	va_start(args, fmt);
	i=CrtpPrintfFmt(DEFAULT_ATTRIBUTE, x, y, fmt, args);
	va_end(args);

	return i;
}

KERNELAPI int  CrtPrintfWithAttr(UCHAR Attr, const char *fmt, ...)
{
	int i;
	va_list args;

	va_start(args, fmt);
	i=CrtpPrintfFmt(Attr, 0xffff, 0xffff, fmt, args);
	va_end(args);

	return i;
}

KERNELAPI int  CrtPrintfXYWithAttr(UCHAR Attr, WORD x, WORD y, const char *fmt, ...)
{
	int i;
	va_list args;

	va_start(args, fmt);
	i=CrtpPrintfFmt(Attr, x, y, fmt, args);
	va_end(args);

	return i;
}

KERNELAPI BOOL CrtPrintText(LPCSTR pText)
{
	return CrtPrintTextXYWithAttr(pText, 0xffff, 0xffff, DEFAULT_ATTRIBUTE);
}

KERNELAPI BOOL CrtPrintTextXY(LPCSTR pText, WORD x, WORD y)
{
	return CrtPrintTextXYWithAttr(pText, x, y, DEFAULT_ATTRIBUTE);
}

KERNELAPI BOOL CrtPrintTextWithAttr(LPCSTR pText, UCHAR Attr)
{
	return CrtPrintTextXYWithAttr(pText, 0xffff, 0xffff, Attr);
}

KERNELAPI BOOL CrtPrintTextXYWithAttr(LPCSTR pText, WORD x, WORD y, UCHAR Attr)
{
	int i;
	UCHAR *pVideoBuf;

	if(pText == NULL) return FALSE;

	if(x==0xffff && y==0xffff) {
		pVideoBuf=CrtpGetNextVideoPtr();
	} else {
		pVideoBuf=CrtpGetNextVideoPtrWithPos(x, y);
	}

	while( *pText != NULL ) {
		if(pVideoBuf >= (UCHAR *)(VIDEO_BUFFER_ADDRESS+VIDEO_BUFFER_SIZE)) {
			CrtpScrollOneLine();
			pVideoBuf -= (SCREEN_CX*2);
		}

		if(*pText==KEY_LF) {
			;
		} else if(*pText==KEY_CR) {
			CrtpKeyCR(&pVideoBuf);
			CrtpKeyLF(&pVideoBuf);
		} else if(*pText=='\b') {
			CrtpKeyBackspace(&pVideoBuf);
		} else if(*pText==KEY_TAB) {
			for(i=0; i<DEFAULT_TAB_SIZE; i++) {
				*pVideoBuf++ = ' ';
				*pVideoBuf++ = Attr;
			}
		} else {
			*pVideoBuf++=(*pText);
			*pVideoBuf++=Attr;
		}

		pText++;
	}

	CrtpUpdateCursorPosByPtr(pVideoBuf);

	return TRUE;
}


/**********************************************************************************************************
 *                                           INTERNAL FUNTIONS                                            *
 **********************************************************************************************************/
static void CrtpSetCursorPos(WORD x, WORD y)
{
	UCHAR data;

ENTER_CRITICAL_SECTION();
	m_CursorXPos=x;	m_CursorYPos=y;
EXIT_CRITICAL_SECTION();

	data=(UCHAR)((x+y*SCREEN_CX)>>8);
	WRITE_PORT_UCHAR((PUCHAR)0x3d4, 0x0e);
	WRITE_PORT_UCHAR((PUCHAR)0x3d5, data);
	data=(UCHAR)((x+y*SCREEN_CX)&0xff);
	WRITE_PORT_UCHAR((PUCHAR)0x3d4, 0x0f);
	WRITE_PORT_UCHAR((PUCHAR)0x3d5, data);
}

static void CrtpUpdateCursorPosByPtr(UCHAR *pPtr)
{
	WORD x, y;
	int bufPos = ((int)pPtr-VIDEO_BUFFER_ADDRESS)/2;

	y = (WORD)(bufPos / SCREEN_CX);
	x = (WORD)(bufPos % SCREEN_CX);

	CrtpSetCursorPos(x, y);
}

static UCHAR* CrtpGetNextVideoPtr(void)
{
	return (UCHAR *)(VIDEO_BUFFER_ADDRESS+(m_CursorXPos+m_CursorYPos*SCREEN_CX)*2);
}

static UCHAR* CrtpGetNextVideoPtrWithPos(WORD x, WORD y)
{
	return (UCHAR *)(VIDEO_BUFFER_ADDRESS+(x+y*SCREEN_CX)*2);
}

static void CrtpKeyLF(UCHAR **pVideoPtr)
{
	WORD x, y;
	UCHAR *pVideoBase = (UCHAR *)VIDEO_BUFFER_ADDRESS;
	int videoPos = ((int)*pVideoPtr-VIDEO_BUFFER_ADDRESS)/2;

	y = (WORD)(videoPos / SCREEN_CX);
	x = (WORD)(videoPos % SCREEN_CX);

	if(y == (SCREEN_CY-1)) {
		CrtpScrollOneLine();
		*pVideoPtr = (UCHAR *)(VIDEO_BUFFER_ADDRESS+VIDEO_BUFFER_SIZE-(SCREEN_CX-x)*2);
	} else {
		*pVideoPtr += (SCREEN_CX*2);
	}
}

static void CrtpKeyCR(UCHAR **pVideoPtr)
{
	WORD x;
	int videoPos = ((int)*pVideoPtr-VIDEO_BUFFER_ADDRESS)/2;

	x = (WORD)(videoPos % SCREEN_CX);
	*pVideoPtr-=(x*2);
}

static void CrtpKeyBackspace(UCHAR **pVideoPtr)
{
	if(*pVideoPtr < (UCHAR *)(VIDEO_BUFFER_ADDRESS+2))
		return;

	*pVideoPtr-=2;
	**pVideoPtr = ' ';
}

static void CrtpScrollOneLine(void)
{
	UCHAR *pVideoBase = (UCHAR *)VIDEO_BUFFER_ADDRESS;
	UCHAR *pSecondLine = (UCHAR *)(VIDEO_BUFFER_ADDRESS+SCREEN_CX*2);

	while(pSecondLine < (UCHAR *)(VIDEO_BUFFER_ADDRESS+VIDEO_BUFFER_SIZE)) {
		*pVideoBase++ = (*pSecondLine++);
	}
	while(pVideoBase < (UCHAR *)(VIDEO_BUFFER_ADDRESS+VIDEO_BUFFER_SIZE)) {
		*pVideoBase++ = ' ';
		*pVideoBase++ = DEFAULT_ATTRIBUTE;
	}
}

static int CrtpPrintfFmt(UCHAR Attr, WORD x, WORD y, const char *fmt, va_list args)
{
	char buf[256];
	int i;

	i=vsprintf(buf, fmt, args);
	CrtPrintTextXYWithAttr(buf, x, y, Attr);

	return i;
}