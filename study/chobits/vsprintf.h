#ifndef _VSPRINTF_H_
#define _VSPRINTF_H_

/*
 * INCLUDE FILES
 */
#include "ctype.h"
#include "stdarg.h"

/*
 * Library functions
 */
int sprintf(char *buf, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);


#endif /* #ifndef _VSPRINTF_H_ */