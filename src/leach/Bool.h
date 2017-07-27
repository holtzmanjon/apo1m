/******************************************************************************
*
*   File:       Bool.h
*   Version:	1.00
*   Author:     Scott Streit
*   Abstract:   Defines boolean values.
*
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*	03/22/01	sds	1.00	Initial
*
******************************************************************************/
#ifndef __BOOLEAN__
#define __BOOLEAN__

#ifndef WIN2K
	enum BOOL {FALSE, TRUE};
	typedef enum BOOL boolean;
#endif

#endif
