#ifndef _SYSCALL_TYPE_H
#define _SYSCALL_TYPE_H

/*
 * INCLUDE FILES
 */
#include "types.h"

/*
 * DEFINITIONS
 */
typedef enum _SYSCALL_TYPES {
	/* system api */
	SYSCALL_TERMINATED=0,
	SYSCALL_DELAY,
	SYSCALL_GET_TICKCOUNT,
	SYSCALL_SHOW_TSWATCHDOG,
	SYSCALL_HIDE_TSWATCHDOG,
	/* screen */
	SYSCALL_CLEAR_SCREEN,
	SYSCALL_PRINT_TEXT,
	SYSCALL_PRINT_TEXT_XY,
	SYSCALL_PRINT_TEXT_ATTR,
	SYSCALL_PRINT_TEXT_XY_ATTR,
	/* keyboard */
	SYSCALL_HAS_KEY,
	SYSCALL_GET_KEYDATA,
} SYSCALL_TYPES;

typedef struct _SYSCALL_MSG {
	SYSCALL_TYPES		syscall_type;
	union {
		struct {
			WORD		x, y;
			BYTE		*pt_text;
			BYTE		attr;
		} PRINT_TEXT;

		struct {
			DWORD		milli_sec;
		} DELAY;
	} parameters;
} SYSCALL_MSG, *PSYSCALL_MSG;

#endif /* #ifndef _SYSCALL_TYPE_H */