#ifndef _KEYBOARD_DRIVER_HEADER_FILE_
#define _KEYBOARD_DRIVER_HEADER_FILE_


/**********************************************************************************************************
 *                                                INCLUDE FILES                                           *
 **********************************************************************************************************/
#include "chobits.h"
#include "key_def.h"


/**********************************************************************************************************
 *                                               DIRVER FUNCTIONS                                         *
 **********************************************************************************************************/
KERNELAPI BOOL KbdHasKey(VOID); /* any key data exist? */
KERNELAPI BOOL KbdGetKey(KBD_KEY_DATA *pKeyData);


#endif /* #ifndef _KEYBOARD_DRIVER_HEADER_FILE_ */