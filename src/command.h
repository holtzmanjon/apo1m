int getsao(double,double,double,double,double,double,
  double *,double *,long *,double *,double,long *,int,int);
int getsaved(double *, double *, double *);
void gethms(double, int *, int *, double *, int *);
void display_menu();
void vcopy(char *, char *, unsigned char);
BOOL illegal_ra(double);
BOOL illegal_dec(double);
BOOL illegal_az(double);
BOOL illegal_alt(double);
BOOL samestar(long,long *, int);

/* saved coordinate list */
#define MAXSAVE 5
int isave=0;
double savedra[MAXSAVE] = {0.,0.,0.,0.,0.};
double saveddec[MAXSAVE] = {0.,0.,0.,0.,0.};
double savedepoch[MAXSAVE] = {0.,0.,0.,0.,0.};

#define TCSERR_OK 0
