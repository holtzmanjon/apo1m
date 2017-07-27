/*==========================================================================*/
/* Apogee CCD Control API - Network Interface I/O Functions                 */
/*                                                                          */
/* (c) 1999 GKR Computer Consulting                                         */
/*                                                                          */
/* revision   date       by                description                      */
/* -------- --------   ----     ------------------------------------------  */
/*   1.00   12/07/99    gkr     Created                                     */
/*                                                                          */
/*==========================================================================*/

#ifdef _APG_NET

extern char szServer[];
extern UINT uiPort;

// Initialize net interface
BOOL DLLPROC net_init(void);

// Deinitialize PPI interface
void DLLPROC net_fini(void);

// Set process priorities
void DLLPROC net_priority(DWORD lClass, DWORD lThread);

// Input word from camera register
USHORT DLLPROC net_inpw(USHORT reg);

// Input count words from camera register
ULONG DLLPROC net_inpwn(USHORT reg, ULONG cnt, PDATA pData);

// Discard count words from camera register
ULONG DLLPROC net_discwn(USHORT reg, ULONG cnt);

// Output word to camera register
void DLLPROC net_outpw(USHORT reg, USHORT data);

// Input count words from camera register
USHORT DLLPROC net_inpw(USHORT reg);

#endif
