#define MAXFILT 10

// Note these arrays have an extra element to allow values to start at
//  index 1 to match filter convention
extern double zero[MAXFILT+1];
extern int mag[MAXFILT+1];
extern char filtname[MAXFILT+1][6];
extern char longfiltname[MAXFILT+1][30];
extern int focoff[MAXFILT+1];
extern double fudge;

int initfilt();
int getfilt(char *fname);
