/*==========================================================================*/
/* Apogee CCD Control API - NET Interface I/O Functions                     */
/*                                                                          */
/* (c) 1999 GKR Computer Consulting                                         */
/*                                                                          */
/* revision   date       by                description                      */
/* -------- --------   ----     ------------------------------------------  */
/*   1.00   12/07/99    gkr     Created                                     */
/*                                                                          */
/*==========================================================================*/

#ifdef _APGDLL
#include <windows.h>
#include <time.h>
#endif

#include <conio.h>

#include "apccd.h"

extern unsigned short PASCAL NetRead1(unsigned short reg);
extern unsigned long PASCAL NetReadN(unsigned short reg, unsigned long cnt, unsigned short *pData);
extern unsigned long PASCAL NetDiscardN(unsigned short reg, unsigned long cnt);
extern void PASCAL NetSetPriority(unsigned long lProcess, unsigned long lThread);
extern void PASCAL NetWrite1(unsigned short reg, unsigned short val);
extern BOOL PASCAL NetInit(char *szServer, UINT uiPort);
extern void PASCAL NetFini(void);

char szServer[256] = "localhost";
UINT uiPort = 5555;

void DLLPROC
net_priority(DWORD lProcess, DWORD lThread)
{
	NetSetPriority(lProcess, lThread);
}

BOOL DLLPROC
net_init(void)
{
	return (NetInit(szServer,uiPort));
}

void DLLPROC
net_fini(void)
{
	NetFini();
}

USHORT DLLPROC
net_inpw(USHORT port)
{
	return (NetRead1(port));
}

ULONG DLLPROC
net_inpwn(USHORT port, ULONG cnt, PDATA pData)
{
	return (NetReadN(port, cnt, pData));
}

ULONG DLLPROC
net_discwn(USHORT port, ULONG cnt)
{
	return (NetDiscardN(port, cnt));
}

void DLLPROC
net_outpw(USHORT port, USHORT data)
{
	NetWrite1(port, data);
}
