#ifndef _WHICH_SYSTEM_H_
#define _WHICH_SYSTEM_H_

/* -------------------- */
/* Undefine all systems */
/* -------------------- */
#undef LINUX
#undef UNIX
#undef WIN2K

/* -------------------- */
/* Define the system    */
/* -------------------- */
#define LINUX 

/* -------------------------------- */
/* Define system dependent types    */
/* -------------------------------- */
#ifndef WIN2K
	typedef int HANDLE;
#endif

#endif
