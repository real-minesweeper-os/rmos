#include "string.h"

#ifndef NULL
# define NULL ((void *)0)
#endif

void* memset(void *pSource, int ch, size_t size)
{
	size_t i;

	for(i=0; i<size; i++)
		*(((char *)pSource)+i) = (char)ch;

	return pSource;
}

char *strlwr(char *str)
{
	char *cp = str;

	while(*cp != '\0') {
		if(*cp >= 'A' && *cp <= 'Z')
			*cp = *cp-'A' + 'a';
		cp++;
	}

	return str;
}

char *strupr(char *str)
{
	char *cp = str;

	while(*cp != '\0') {
		if(*cp >= 'a' && *cp <= 'z')
			*cp = *cp-'a' + 'A';
		cp++;
	}

	return str;
}

/* codes from Linux */
int strcmp(const char *cs, const char *ct)
{
	char __res;

	while(1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}

/* codes from Linux */
size_t strlen(const char * s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

/* codes from Linux */
void *memcpy(void *dest, const void *src, size_t count)
{
	char *tmp = (char *) dest, *s = (char *) src;

	while (count--)
		*tmp++ = *s++;

	return dest;
}

/* codes from Linux */
char * strrchr(const char * s, int c)
{
       const char *p = s + strlen(s);
       do {
           if (*p == (char)c)
               return (char *)p;
       } while (--p >= s);
       return NULL;
}

/* codes from Linux */
char * strcpy(char * dest,const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}

/* codes from Linux */
char * strcat(char * dest, const char * src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;

	return tmp;
}