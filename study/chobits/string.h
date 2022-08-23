#ifndef _STRING_H_
#define _STRING_H_

/*
 * INCLUDE FILES
 */
#include "ctype.h"

/*
 * Library functions
 */
void* memset(void *pSource, int ch, size_t size);
int strcmp(const char *cs, const char *ct);
size_t strlen(const char * s);
void *memcpy(void *dest, const void *src, size_t count);
char *strlwr(char *str);
char *strupr(char *str);
char * strrchr(const char * s, int c);
char * strcpy(char * dest,const char *src);
char * strcat(char * dest, const char * src);


#endif /* #ifndef _STRING_H_ */