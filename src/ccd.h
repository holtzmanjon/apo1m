#ifdef TELERIS2
// Spectrasource Teleris2 with KAF260
#define NCOL 524
#define NROW 512
#define NBIAS 6
#define NSPE 539458
#define XYRAT 1.
#define DTIME 1
int offset = 2880;
int tvflip=0;
#endif

#ifdef LYNXX
// Spectrasource Lynxx
#define NSPE 69542
#define NCOL 202
#define NROW 165
#define NBIAS 10
#define XYRAT (165./192.)
#define DTIME 1
int offset = 2880;
int tvflip=0;
#endif

#ifdef APOGEE
// Apogee AP7P
#define NCOL 525
#define NROW 512
#define NBIAS 10
#define NSPE 566048
#define XYRAT 1.
#define DTIME 2
int offset = 0;
int tvflip=1;
#endif

#ifdef LCCD
// Marconi 2Kx2K from Leach/LANL
#define NCOL 2200
#define NROW 2200
#define NBIAS 152
#define NSPE 9682880
#define XYRAT 1.
#define DTIME 2
int offset = 0;
int tvflip=1;
#endif

#ifdef PI
// Princeton CCD
#define NCOL 1050
#define NROW 1024
#define NBIAS 20
#define NSPE 2207000
#define XYRAT 1.
#define DTIME 34
#undef HAVEDARK
int offset = 2000;
int tvflip=0;
#endif

