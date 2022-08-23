#ifndef _FDD_DRIVER_HEADER_FILE_
#define _FDD_DRIVER_HEADER_FILE_


/**********************************************************************************************************
 *                                                INCLUDE FILES                                           *
 **********************************************************************************************************/
#include "chobits.h"


/**********************************************************************************************************
 *                                               DIRVER FUNCTIONS                                         *
 **********************************************************************************************************/
KERNELAPI BOOL FddReadSector (WORD SectorNumber, BYTE NumbersOfSectors, BYTE *pData);
KERNELAPI BOOL FddWriteSector(WORD SectorNumber, BYTE NumbersOfSectors, BYTE *pData);


#endif /* #ifndef _FDD_DRIVER_HEADER_FILE_ */