#undef MSBFirst
#define _VISTADIR      '/usr/local/src/xvista/source/../'
#define _FONTBIN       '/usr/local/src/xvista/source/../lickmongo/fonts.ltl/fonts.vis'
#define _FONTDAT       '/usr/local/src/xvista/source/../lickmongo/fonts.ltl/fonts.dat'
#define _HELPFILE      '/usr/local/src/xvista/source/../lickmongo/help.dat' 
#define _MONGOSCR      '/tmp/'
#define _MONGOPS       '/usr/local/src/xvista/source/../lickmongo/'
#define __MACHINE 'linux'
#define ADDRESS int
#undef __64BITADDRESS
#define __UNIX
#define __LINUX
#define GCC
#define G77
#undef __TAPESUPPORTED
#undef __UNIXTAPE
#define __X11
#undef __SUNVIEW
#undef  __AED512
#undef  __AED1024
#undef  __PER
#define __HAIRS
#define __VDAO
#define __NEWLIB
#define __MAXSTR 20000
#define __USEWFPC
#undef __USECRI
#undef __HAVEDST
#undef __USEIRAF
#undef SunFortran1_2
#define IMPLICIT_NONE    implicit none
#define DOUBLE_BACKSLASH
#define  VMS_CHAR_INIT
#undef f77_CHAR_INIT
#define Unix_LOG_OPS
#undef  VMS_LOG_OPS
#undef  CHAR_NOT_CHAR
#undef __READONLY
#undef VMS_IO_STYLE
#define Fdtrig
#define Fibit
#define Fctime
#define Fdtime
#define Ftime
#define Fperror
#define Fchdir
#define Frand
#define NoXOR
#define _POSIX_SOURCE
#define TUS
#define usleep_exists
#define INTERRUPT_HANDLE
#define XBUTTON3
#define __READLINE
#define __NDPGREX
#define __DR640
#undef  __VT125
#define __Tk4010
#undef  __Gr270
#undef  __AED512
#undef  __GSI
#undef  __HP2648A
#undef  __ImTek
#undef  __ISIOVW
#undef  __uVAXUIS
#undef  __SunView
#define __Vis603
#undef  __ISIEpson
#undef  __Selanar
#define __xterm
#undef  __uVAXTek
#define __GO2xx
#define __GO140
#define __PostScript
#undef  __VersV80
#undef  __Printronix
#undef  __Imagen
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
#if defined(SUNWspro) || defined(GCC)
#   define PASTE(x,y) _PASTE_(x,y)
#   define _PASTE_(x,y) x ## y
#   define PMGO(s) s
#   define PTUV(s) s
#   define POBS(s) s
#   ifdef TUS
#       define UMGO(s) PASTE(s,_)
#       define UTUV(s) PASTE(s,_)
#       define UOBS(s) PASTE(s,_)
#       define ATUS(s) PASTE(s,_)
#   else  /* TUS */
#       define UMGO(s) s
#       define UTUV(s) s
#       define UOBS(s) s
#       define ATUS(s) s
#   endif /* TUS */
#else
#       define PASTE(x,y) x/**/y
#       define PMGO(s) s
#       define PTUV(s) s
#       define POBS(s) s
#       ifdef TUS
#           define UMGO(s) s/**/_
#           define UTUV(s) s/**/_
#           define UOBS(s) s/**/_
#           define ATUS(s) s/**/_
#       else  /* TUS */
#           define UMGO(s) s
#           define UTUV(s) s
#           define UOBS(s) s
#           define ATUS(s) s
#       endif /* TUS */
#endif
#ifdef  VMS
#   undef  unix
#   undef  sun
#   undef  sun3
#   undef  sun4
#   undef  ISI
#   undef  sgi
#   define VMS_VMEM */
#   define VMS_CHAR_INIT
#   define VMS_LOG_OPS
#endif  /* VMS */
#ifdef VMS_IO_STYLE
#   define Shared_ReadOnly ,shared,readonly
#   define ReadOnly ,readonly
#else  /* VMS_IO_STYLE */
#   define Shared_ReadOnly   /* as nothing */
#   define ReadOnly          /* as nothing */
#endif /* VMS_IO_STYLE */
#ifdef VMS
#   define CarriageControlList ,carriagecontrol='list'  /* for sanity */
#   define StatNew 'NEW'     /* works when files have versions */
#else  /* VMS */
#   define CarriageControlList          /* as nothing */
#   define StatNew 'UNKNOWN'            /* works when files have NO versions */
#endif /* VMS */
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
#endif  /* F90_CHAR_INIT */
#endif  /* VMS_CHAR_INIT */
#endif  /* f77_CHAR_INIT */
#ifdef DOUBLE_BACKSLASH
#define AsciiBackSlash '\\' 
#else  /* DOUBLE_BACKSLASH */
#define AsciiBackSlash char(92)
#endif /* DOUBLE_BACKSLASH */
