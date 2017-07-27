int power_status(int *);
void power_on(int);
void power_off(int);
void power_change(int);

#ifdef LAMPS
#define NPLUG 5
#else
#define NPLUG 24
#endif

#define ALL 0
#ifdef LAMPS
#define HE 1
#define NE 2
#define AR 3
#define BRIGHT 4
#define DIM 5
#else
#define TOCC 1
#define WEATHER 3
#define FILL 7
#define TEMPAGERA 8
#define VIDEO 10
#define EYEBALL 11
#define DOMEFAN 12
#define LOUVERS 13
#define LIGHTS 14
#define RACKFAN 15
#define MOTORS 16
#define FIBERUSB 17
//#define MIRRORFANS 18
#define HSP 18
//#define HSP 19
//#define NETPOW 19
#define GCS 20
#define LEACHPOW 21
//#define ROPERPOW 22
//#define DOROTHEA 23
//#define ASTROTIMER 24
#define TEMPAGERB 24
#endif

#define OFF 0
#define ON 1
#define UNKNOWN -1
