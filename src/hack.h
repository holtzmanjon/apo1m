/*
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 *      This CPP module is Copyrighted software.
 *      The file COPYRIGHT must accompany this file.  See it for details.
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 */
/*
 *      This set of #defines determines the device numbers for active devices.
 *      If you wish to alter the device numbers, do it here.
 *
 *      __DEVICE is set in Config.h and tells whether DEVICE is active.
k*      _DEVICE defines the device numbers for DEVICE
 *
 *      NOTE CAREFULLY:  FORTRAN doesn't care about the upper/lower case
 *      but the C preprocessor does--preserve the capitalization in `#' lines
 *      These terminal names are identical to the PARAMETER statements in
 *      MONGODEV.F except for the underscore.  The underscore prevents
 *      the C preprocessor from replacing the PARAMETERs with numbers.
 */

#ifdef  __DR640
#       define  _DR640           1
#endif
#ifdef  __VT125
#       define  _VT125           2
#endif
#ifdef  __Tk4010
#       define  _Tk4010          3
#endif
#ifdef  __Gr270
#       define  _Gr270           4
#endif
#ifdef  __AED512
#       define  _AED512          4
#endif
#if     defined(ISI) && defined(GSI_exists) && defined(__GSI)
#       define  _GSI             4
#endif
#ifdef  __HP2648A
#       define  _HP2648A         5
#endif
#ifdef  __ImTek
#       define  _ImTek           6
#endif
#if     defined(ISI) && defined(__ISIOVW)
#       define  _ISIOVW          7
#endif
#if     defined(VMS) && defined(__uVAXUIS)
#       define  _uVAXUIS         7
#endif
#if     defined(sun) && defined(__SunView)
#       define  _SunView         7
#endif
#ifdef  __Vis603
#       define  _Vis603          8
#endif
#if     defined(Epson_exists) && defined(__ISIEpson)
#       define  _ISIEpson        9
#endif
#ifdef  __Selanar
#       define  _Selanar        10
#endif
#ifdef  __xterm
#       define  _xterm          10
#endif
#if     defined(X11_exists) && defined(__X11)
#       define  _X11            11
#endif
#if     defined(VMS) && defined(__uVAXTek)
#       define  _uVAXTek        12
#endif
#ifdef  __GO2xx
#       define  _GO2xx          13
#endif
#ifdef  __GO140
#       define  _GO140          14
#endif
#ifdef  __PostScript
#       define  _PostScript    -17
#endif
#if     defined(VMS) && (defined(__VersV80) || defined(__Printronix) || defined(__Imagen))
#       define  _VECFILE
#endif
#if     defined(VMS) && defined(__VersV80)
#       define  _VersV80        -1
#endif
#if     defined(VMS) && defined(__Printronix)
#       define  _Printronix     -2
#endif
#if     defined(VMS) && defined(__Imagen)
#       define  _Imagen         -3
#endif

/*
 *      This is magic code for the prefixes on all function names
 *      Note that these defines must be true for the CPP which is
 *      preprocessing the code.  When code is being generated for a
 *      VMS system we must at this point know what kind of machine is
 *      doing the CPP, and then afterwards turn off that machine's defines.
 *
 *      xMGO routines are publicly documented routines
 *      xTUV routines are internal and undocumented
 *      xOBS routines are obsolete, undocumented, and deprecated
 *      ATUS is used to append a terminal US as needed to call system routines
 */
#if defined(SUNWspro) || defined(__STDC__) && !alpha
#   define PASTE(x,y) _PASTE_(x,y)
	/* Indirection is to allow macro expansions. */
#   define _PASTE_(x,y) x ## y
#   define PMGO(s) s
#   define PTUV(s) PASTE(tuv,s)
#   define POBS(s) s
#   ifdef TUS
#       define UMGO(s) PASTE(s,_)
#       define UTUV(s) PASTE(tuv,PASTE(s,_))
#       define UOBS(s) PASTE(s,_)
#       define ATUS(s) PASTE(s,_)
#   else  /* TUS */
#       define UMGO(s) s
#       define UTUV(s) PASTE(tuv,s)
#       define UOBS(s) s
#       define ATUS(s) s
#   endif /* TUS */
#else 
#   if defined(SYS_V) | defined(sun) | defined(sgi) | defined(ISI) | defined(DECSta) | defined(vms) | defined(CVX) | defined(uvax) | defined(alpha) | defined(linux)
	/*  Alas, this PASTE cannot be made to work when one of the args
	 *  is actually a Cpp token that needs to be expanded.
	 */
#       define PASTE(x,y) x/**/y
#       define PMGO(s) s
#       define PTUV(s) tuv/**/s
#       define POBS(s) s
#       ifdef TUS
#           define UMGO(s) s/**/_
#           define UTUV(s) tuv/**/s/**/_
#           define UOBS(s) s/**/_
#           define ATUS(s) s/**/_
#       else  /* TUS */
#           define UMGO(s) s
#           define UTUV(s) tuv/**/s
#           define UOBS(s) s
#           define ATUS(s) s
#       endif /* TUS */
#   endif
#endif
/*
 *      Certain CPP defines must be set while preprocessing the above
 *      but must be unset afterwards.
 */
#ifdef  VMS
#   undef  unix
#   undef  sun
#   undef  sun3
#   undef  sun4
#   undef  ISI
#   undef  sgi
#   define VMS_VMEM     /* Due to historical reasons,                   */
			/* virtual memory is used only on VMS systems   */
#   define VMS_CHAR_INIT
#   define VMS_LOG_OPS
/*#else
#   if defined()

#   endif*/
#endif  /* VMS */
/*
 *      Damn those VMS-style OPEN and CLOSE semantics
 */
#ifdef VMS_IO_STYLE
				/* Dec should be shot for these */
#   define Shared_ReadOnly ,shared,readonly
#   define ReadOnly ,readonly
#else  /* VMS_IO_STYLE */
#   define Shared_ReadOnly                      /* as nothing */
#   define ReadOnly                             /* as nothing */
#endif /* VMS_IO_STYLE */
#ifdef VMS
#   define CarriageControlList ,carriagecontrol='list'  /* for sanity */
#   define StatNew 'NEW'                /* works when files have versions */
#else  /* VMS */
#   define CarriageControlList          /* as nothing */
#   define StatNew 'UNKNOWN'            /* works when files have NO versions */
#endif /* VMS */

	/* The ANSI committee for FORTRAN 77 should have thought of these */
#ifdef  f77_CHAR_INIT
#   define NUL_DATA '\0'
#   define SOH_DATA '\'
#   define ETX_DATA '\'
#   define TAB_DATA '\t'
#   define FF__DATA '\'
#   define CR__DATA '\'
#   define DLE_DATA '\'
#   define NAK_DATA '\'
#   define CAN_DATA '\'
#   define SUB_DATA '\'
#   define ESC_DATA '\'
#   define GS__DATA '\'
#   define US__DATA '\'
#else   /* f77_CHAR_INIT */
#ifdef  VMS_CHAR_INIT
#   define NUL_DATA 0
#   define SOH_DATA 1
#   define ETX_DATA 3
#   define TAB_DATA 9
#   define FF__DATA 12
#   define CR__DATA 13
#   define DLE_DATA 16
#   define NAK_DATA 21
#   define CAN_DATA 24
#   define SUB_DATA 26
#   define ESC_DATA 27
#   define GS__DATA 29
#   define US__DATA 31
#else   /* VMS_CHAR_INIT */
#ifdef  F90_CHAR_INIT
#   define NUL_DATA O'00'
#   define SOH_DATA O'01'
#   define ETX_DATA O'03'
#   define TAB_DATA O'11'
#   define FF__DATA O'14'
#   define CR__DATA O'15'
#   define DLE_DATA O'20'
#   define NAK_DATA O'25'
#   define CAN_DATA O'30'
#   define SUB_DATA O'32'
#   define ESC_DATA O'33'
#   define GS__DATA O'35'
#   define US__DATA O'37'
#else   /* F90_CHAR_INIT */
    /* The Fortran compiler should croak and die */
#endif  /* F90_CHAR_INIT */
#endif  /* VMS_CHAR_INIT */
#endif  /* f77_CHAR_INIT */

#ifdef DOUBLE_BACKSLASH
/* #   define AsciiBackSlash '\\\\'     */
#   define AsciiBackSlash '\\'
#else  /* DOUBLE_BACKSLASH */
/* #   define AsciiBackSlash '\\'       */
#   define AsciiBackSlash '\\'
#endif /* DOUBLE_BACKSLASH */
