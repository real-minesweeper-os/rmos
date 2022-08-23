#ifndef _DEBUG_H_
#define _DEBUG_H_


/*
 * INCLUDE FILES
 */
#include "6845crt.h"


/*
 * DEBUG DEFINITIONS
 */
#ifdef __DEBUG__
#define DbgPrint			CrtPrintf
#else
int DbgPrint(const char *fmt, ...) { return -1; }
#endif


#endif /* #ifndef _DEBUG_H_ */