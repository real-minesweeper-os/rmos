#ifndef _KERNEL_DATA_TYPES_H_
#define _KERNEL_DATA_TYPES_H_


/*
 * PROCESSOR SPECIFIC DEFINITIONS
 */
#define far
#define near
#define FAR 
#define NEAR

/*
 * CONST and NULL POINTER
 */
/* #define NULL				((void *)0) */
#define CONST               const
#define NULL				0

/*
 * IN/OUT PHANTOM PREFIX USED BY KERNEL API's PARAMETER FIELD
 */
#define IN
#define OUT

#define KERNELAPI

/*
 * HANDLE
 */
typedef void				*HANDLE;
typedef HANDLE				*PHANDLE;

/*
 * PROCESSOR INDEPENDANT VARIABLE's SIZE DEFINITIONS
 */
typedef void				VOID;
typedef VOID near			*PVOID;
typedef VOID far            *LPVOID;
typedef CONST void far      *LPCVOID;

typedef char				CHAR;
typedef CHAR				*PCHAR;
typedef unsigned char		UCHAR;
typedef UCHAR				*PUCHAR;
typedef unsigned char       BYTE;
typedef BYTE near           *PBYTE;
typedef BYTE far            *LPBYTE;

typedef unsigned short		WORD;
typedef WORD				*PWORD;
typedef unsigned short		USHORT;
typedef USHORT				*PUSHORT;

typedef long				LONG;
typedef LONG				*PLONG;
typedef unsigned long		ULONG;
typedef ULONG				*PULONG;
typedef unsigned long       DWORD;
typedef DWORD				*PDWORD;

typedef int					INT;
typedef INT					*PINT;
typedef unsigned int        UINT;
typedef UINT				*PUINT;

typedef float				FLOAT;
typedef FLOAT				*PFLOAT;

typedef double				DOUBLE;
typedef DOUBLE				*PDOUBLE;


/*
 * BOOL DEFINITIONS
 */
typedef INT					BOOL;
typedef BOOL near			*PBOOL;
typedef BOOL far			*LPBOOL;

#define TRUE				1
#define FALSE				0


/*
 * STRING ASSOCIATED DEFINITIONS
 */
typedef CHAR near			*PSTR;
typedef CONST CHAR near		*CPSTR;
typedef CHAR far			*LPSTR;
typedef CONST CHAR far		*LPCSTR;
typedef CONST CHAR far		*LPCTSTR;


#endif /* #ifndef _KERNEL_DATA_TYPES_H_ */