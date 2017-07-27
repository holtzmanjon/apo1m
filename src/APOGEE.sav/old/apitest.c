/* compile with gcc -O2 -o apitest42 apitest42.c libapogee.a */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/io.h>
//#include <asm/io.h>
#include <ctype.h>

#define VERSION		"4.2"
#define INI_FILE	"APCCD.INI"
#define DWORD       unsigned long
#define MAX(a,b)    ((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)>(b)?(b):(a))

// define TEST to output last image as raw file TEST.DAT
//#define TEST

// must define WINUTIL to call Win32 utility functions


#include "apccd.h"
#include "aplow.h"
#include "aperr.h"

STATUS  test_image(USHORT, USHORT, PDATA);

CAMDATA cdLocal;
HCCD hccd;
BOOL bPrompt = FALSE;
DWORD dwSecs;

static void InitTicks(void)
{
	struct timeval tv;
	
	gettimeofday(&tv,NULL);
	dwSecs = tv.tv_sec;
}	

static DWORD GetTickCount(void)
{
	struct timeval tv;
	
	gettimeofday(&tv,NULL);
	return((tv.tv_sec-dwSecs) * 1000 + (tv.tv_usec + 500) / 1000);
}

static char *temp_status(USHORT uStatus)
{
	switch (uStatus) {
	case CCD_TMP_OK:
		return ("stable");
	case CCD_TMP_UNDER:
		return ("under");
	case CCD_TMP_OVER:
		return ("over");
	case CCD_TMP_ERR:
		return ("error");
	case CCD_TMP_OFF:
		return ("off");
	case CCD_TMP_RDN:
		return ("ramping down");
	case CCD_TMP_RUP:
		return ("ramping up");
	case CCD_TMP_STUCK:
		return ("stuck");
	case CCD_TMP_MAX:
		return ("highest");
	case CCD_TMP_DONE:
		return ("done");
	default:
		return ("unknown");
	}
}

static void extended_errors(void)
{
	printf("CAMDATA extended error : ");
	switch(cdLocal.errorx) {
	case APERR_NONE:
		puts("none");
		break;
	case APERR_CFGNAME:
		puts("blank config filename");
		break;
	case APERR_CFGBASE:
		puts("no interface base");
		break;
	case APERR_CFGROWS:
		puts("camera rows not defined");
		break;
	case APERR_CFGCOLS:
		puts("camera columns not defined");
		break;
	case APERR_NOCFG:
		puts("no recognized config entries");
		break;
	case APERR_FRAME:
		puts("frame done timeout");
		break;
	case APERR_CALCAIC:
		puts("calc aic, camc <= bic + skipc + imgc");
		break;
	case APERR_CALCAIR:
		puts("calc air, camr <= bir + skipr + imgr");
		break;
	case APERR_AIC:
		puts("aic < 1 or > 4095");
		break;
	case APERR_BIC:
		puts("bic < 1 or > 4095");
		break;
/*	case APERR_HBPW:
		puts("hbin < 1 or > 7");
		break;*/
	case APERR_VBIN:
		puts("vbin < 1 or > 63");
		break;
	case APERR_COLCNT:
		puts("col < 1 or > 4095/hbin");
		break;
	case APERR_ROWCNT:
		puts("row < 1 or > 4095");
		break;
	case APERR_CMDACK:
		puts("command acknowledge timeout");
		break;
	case APERR_RSTSYS:
		puts("reset system timeout");
		break;
	case APERR_FLSTRT:
		puts("start flush timeout");
		break;
	case APERR_CLOSE:
		puts("couldn't clear camdata structure");
		break;
	case APERR_TRIGOFF:
		puts("trigger not active");
		break;
	case APERR_LINACK:
		puts("next line ack timeout");
		break;
	case APERR_LINE:
		puts("line timeout");
		break;
	case APERR_PDATA:
		puts("void data pointer");
		break;
	case APERR_TEMP:
		puts("temp < -40C or > 40C");
		break;
	case APERR_SLICE:
		puts("slice acquisitions not available");
		break;
	case APERR_CFGSLICE:
		puts("config_slice not called");
		break;
	case APERR_SKIPR:
		puts("bir <= skipr");
		break;
	case APERR_SKIPC:
		puts("bic <= skipc");
		break;
	default:
		printf("unknown extended error %d\n", cdLocal.errorx);
	}
}

static void show_errors(void)
{
	STATUS ok;
	
	ok = get_openerror();
	printf("Open error value %d\n", ok);

	ok = get_camdata(hccd, &cdLocal);
	if (ok != CCD_OK) {
		puts("Couldn't get CAMDATA structure");
		return;
	}
	printf("CAMDATA error value %d = 0x%x\n", cdLocal.error, cdLocal.error);

	extended_errors();
}

static void open_error(void)
{
	STATUS oerr = get_openerror();
	printf("Open error #%d\n", oerr);
	switch(oerr) {
	case CCD_OPEN_NOERR:
		puts("No error, Ouch!");
		break;
	case CCD_OPEN_HCCD:
		puts("No free handles");
		break;
	case CCD_OPEN_CFGNAME:
		puts("Configuration filename error");
		break;
	case CCD_OPEN_CFGDATA:
		puts("Configuration file missing or illegal");
		break;
	case CCD_OPEN_LOOPTST:
		puts("Camera interface not detected");
		break;
	case CCD_OPEN_MODE:
		puts("Error setting mode bits");
		break;
	case CCD_OPEN_TEST:
		puts("Error setting test bits");
		break;
	case CCD_OPEN_LOAD:
		puts("Error loading ccd configuration");
		break;
	case CCD_OPEN_NTIO:
		puts("Windows NT driver error");
		break;
	case CCD_OPEN_CABLE:
		puts("Error setting cable bit");
		break;
	case CCD_OPEN_CAMREG:
		puts("Error setting camera register");
		break;
	default:
		puts("Unknown error, Ouch!");
		break;
	}
}

static void prompt(char *s)
{
	char szStr[80];

	printf("%s [y]/n ? ", s);
	fgets(szStr,80,stdin);
	if ((*szStr == 'n') || (*szStr == 'N'))
		exit(0);
}

static void check_prompt(char *s)
{
	if (bPrompt)
		prompt(s);
}

static void syntax(void)
{
	puts("syntax : apitest [-reset] [-<opt>=<val>]");
	puts(" where <opt> = e - val=exposure time (msec) [200]");
	puts("               c - val=use caching [T]|F");
	puts("               d - val=output raw data T|[F]");
	puts("               f - val=flush between images [T]|F");
	puts("               n - val=frame count [1]");
	puts("               x - val=column count [max]");
	puts("               y - val=row count [max]");
	puts("               z - call test_image");
	puts("               t - check temperature only");
	puts("               t - val=temperature (deg)");
	puts("               a - goto ambient temperature");
	puts("               reset - reset controller");
	puts("               prompt - prompt before major DLL calls");
	exit (0);
}

static int hexval(const CHAR *s)
{
	int val = 0;
	CHAR ch;

	while ((ch=*s++) != 0)
		if ((ch >= '0') && (ch <= '9'))
			val = val * 16 + (int) (ch - '0');
		else if ((ch >= 'a') && (ch <= 'f'))
			val = val * 16 + (int) (ch - 'a' + 10);
		else if ((ch >= 'A') && (ch <= 'F'))
			val = val * 16 + (int) (ch - 'A' + 10);
		else
			break;
	return (val);
}

static int atoh(const CHAR *s)
{
	USHORT val = 0;
	
	if ((strlen(s) >= 2) && (toupper(s[1]) == 'X')) {
		s += 2;
		val = hexval(s);
	} else
		val = atoi(s);

	return (val);
}

int main(int argc, char *argv[])
{
	STATUS ok;
	PDATA pData;
	USHORT max_cols, max_rows;
	FILE *fp;
	USHORT cache;
	size_t pix_cnt;
	USHORT ver, os, maj, min;
	unsigned int frames = 1;
	USHORT cols = 9000;
	USHORT rows = 9000;
	USHORT nFrame;
	USHORT uStatus;
	DWORD dwStart;
	BOOL fCache = TRUE;
	BOOL fFlush = TRUE;
	BOOL fTest = FALSE;
	BOOL fReset = FALSE;
	BOOL fGotoTemp = FALSE;
	BOOL fCheckTemp = FALSE;
	BOOL fAmbient = FALSE;
	BOOL fRawOutput = FALSE;
	double dTemp=0.0;
	double dNow;
	DWORD etime = 20;
	double dPPS;
	int narg;
	char *cp;
	char szStr[80];
    int prior,old_prior;
	int i,j;
        short *data;

	InitTicks();

    if(ioperm(0,1000,1) !=0) {
     fprintf(stderr ,"io permission failure\n");
     exit(-2);
     }


	printf("Apogee API test, version %s\n", VERSION);

	for (narg=1; narg<argc; narg++) {
		cp = argv[narg];
		if (*cp++ == '-') {
			switch (toupper(*cp++)) {
			case 'A':
				fAmbient = TRUE;
				break;
			case 'T':
				while (*cp && (*cp != '=')) cp++;
				if (*cp) {
					dTemp = atof(++cp);
					fGotoTemp = TRUE;
				} else
					fCheckTemp = TRUE;
				break;
			case 'C':
				while (*cp && (*cp != '=')) cp++;
				if (toupper(*++cp) == 'F')
					fCache = FALSE;
				break;
			case 'D':
				while (*cp && (*cp != '=')) cp++;
				if (toupper(*++cp) == 'T')
					fRawOutput = TRUE;
				break;
			case 'Z':
				fTest = TRUE;
				break;
			case 'F':
				while (*cp && (*cp != '=')) cp++;
				if (toupper(*++cp) == 'F')
					fFlush = FALSE;
				break;
			case 'N':
				while (*cp && (*cp != '=')) cp++;
				frames = (unsigned int) atoh(++cp);
				frames = MAX(1,frames);
				break;
			case 'E':
				while (*cp && (*cp != '=')) cp++;
				etime = (atoh(++cp)+5)/10;
				etime = MAX(2L,etime);
				break;
			case 'X':
				while (*cp && (*cp != '=')) cp++;
				cols = atoh(++cp);
				cols = MAX(1,cols);
				break;
			case 'Y':
				while (*cp && (*cp != '=')) cp++;
				rows = atoh(++cp);
				rows = MAX(1,rows);
				break;
			case 'R':
				if (!strcmp(cp,"eset"))
					fReset = TRUE;
				else
					syntax();
				break;
			case 'P':
				bPrompt = TRUE;
				break;
			default:
				syntax();
			}
		} else
			syntax();
	}

	// Always display and check library version
	ver = get_version();
	os = (ver&0xf00)>>8;
	maj = (ver&0xf0)>>4;
	min = (ver&0xf);
	printf("DLL version number is %x.%x.%x\n", os, maj, min);
	/*if (maj != 4)
		prompt("Incompatible DLL, continue");*/

	// Open camera
	sprintf(szStr, "open_camera(mode=IGNORE,test=IGNORE,\"%s\")", INI_FILE);
	check_prompt(szStr);
	hccd = open_camera(IGNORE,IGNORE,INI_FILE);
	if (hccd == 0) {
		hccd = 1;
		show_errors();
		exit (0);
	}

	// Get camera structure
	check_prompt("get_camdata(&cd)");
	ok = get_camdata(hccd, &cdLocal);
	if (ok != CCD_OK) {
		puts("Couldn't get CAMDATA structure");
		close_camera(hccd);
		exit (0);
	}

	if (fReset) {
		check_prompt("reset controller - outpw(base,0)");
		outw(0,cdLocal.base);
		check_prompt("close_camera()");
		close_camera(hccd);
		exit(0);
	}

	// Report current chip state/temp
	check_prompt("get_temp(&stat,&temp)");
	ok = get_temp(hccd, &uStatus, &dNow);
	if (ok != CCD_OK)
		puts("get_temp error");
	else {
		printf("Temperature controller status is : %s\n", temp_status(uStatus));
		printf("Current reported temperature is  : %0.2f\n", dNow);
	}
	
    // Handle temperature control functions and exit
    if (fGotoTemp || fAmbient) {
		sprintf(szStr, "set_temp(temp=%f,%s)", dTemp, fGotoTemp?"CCD_TMP_SET":"CCD_TMP_AMB");
		check_prompt(szStr);
   		ok = set_temp(hccd, dTemp, (USHORT)(fGotoTemp?CCD_TMP_SET:CCD_TMP_AMB));
   		if (ok != CCD_OK)
   			puts("set_temp error");
   		fCheckTemp = TRUE;
   	}
   	if (fCheckTemp) {
		check_prompt("close_camera()");
		close_camera(hccd);
		exit(0);
    }

	max_cols = cdLocal.imgcols;
	cols = MIN(cols,max_cols);
	max_rows = cdLocal.imgrows;
	rows = MIN(rows,max_rows);
	cache = fCache && cdLocal.caching;

	// Allocate image memory
	pix_cnt = cols * rows;
	pData = (PDATA) malloc((pix_cnt+1)*2);
	if (!pData) {
		puts("Image memory allocation didn't work");
		check_prompt("close_camera()");
		close_camera(hccd);
		exit (0);
	}


    old_prior = getpriority(PRIO_PROCESS,0);
    
    printf("Caching will be %s\n", cache?"ON":"OFF");
	printf("After image flushing will be %s\n", fFlush?"ON":"OFF");
	printf("Frame count is %d\n", frames);
	printf("Collecting (%dx%d) image(s)\n", cols, rows);
	printf("Exposure time is %ld msec\n", etime*10L);
  

    	
    setpriority(PRIO_PROCESS,0,-20);
    prior = getpriority(PRIO_PROCESS,0);
    printf("Current Priority is %d\n",prior);
    
// Tell camera which part of the chip to transfer
	sprintf(szStr, "config_camera(bic=%u,bir=%u,cols=%u,rows=%u,cbin=1,rbin=1,time=%lu,cable=IGNORE,ini=FALSE)",
		cdLocal.bic, cdLocal.bir,
		cols, rows,
		etime);
	check_prompt(szStr);
	ok = config_camera(hccd,
					cdLocal.bic, cdLocal.bir,
					cols, rows,
					1, 1,
					etime,
					IGNORE,
					FALSE);
	if (ok != CCD_OK) {
		puts("config_camera didn't work");
		free(pData);
		check_prompt("close_camera()");
		close_camera(hccd);
		exit (0);
	}

	dwStart = GetTickCount();
	nFrame = frames;
	while (nFrame--) {
		// Start a normal shuttered exposure
		check_prompt("start_exposure(shutter=TRUE, trigger=FALSE, block=FALSE, wait=FALSE)");
		ok = start_exposure(hccd, TRUE, FALSE, FALSE, FALSE);
		if (ok != CCD_OK) {
			puts("start_exposure didn't work");
			free(pData);
			check_prompt("close_camera()");
			close_camera(hccd);
			exit (0);
	   	}

		// Wait for exposure to complete
		puts("start_exposure returned CCD_OK");
		puts("waiting for exposure to complete");
		while (check_exposure(hccd) != CCD_OK)
			;
		puts("check_exposure returned CCD_OK");

		// Transfer image data use caching if available
		sprintf(szStr, "acquire_image(cache=%s,pData,flush=%s)",
			cache?"yes":"no",
			fFlush?"yes":"no");
		check_prompt(szStr);
		ok = acquire_image(hccd, (USHORT)(cache?1:0), pData, (USHORT)(fFlush?1:0));

		if (ok != CCD_OK) {
			puts("acquire_image didn't work");
			free(pData);
			check_prompt("close_camera()");
			close_camera(hccd);
			exit (0);
	   	}
                data = pData;
                for (i=0 ; i<500 ; i++)
                  for (j=0 ; j< 500 ; j++) {
                     data++;
                     if (i%100==0 && j%100==0) fprintf(stderr,"%d %d %hd %hu\n",i,j,*data,*data);
                  } 
	}
	dwStart = GetTickCount() - dwStart;
	printf("Collected %d frames in %ld msec\n", frames, dwStart);
	dwStart -= etime * 10 * frames;
	printf("Image data transfer time was %ld msec\n", dwStart);
	dPPS = ((double)pix_cnt * frames)/((double)dwStart)/1.024;
	printf("Data throughput is %.0f Kp/s\n", dPPS);

	// Close camera
	check_prompt("close_camera()");
	close_camera(hccd);

	if (fTest){
		(void) test_image(cols, rows, pData);
       }
	// Store raw image in external disk file
	if (fRawOutput) {
		fp = fopen("test.dat", "wb");
		if (!fp) {
			puts("fopen didn't work");
			free(pData);
			exit (0);
		}
		if (fwrite(pData, 2, pix_cnt, fp) != pix_cnt)
			puts("fwrite error");

		// Clean up
	    	fclose(fp);
	      }
    setpriority(PRIO_PROCESS,0,old_prior);
    prior = getpriority(PRIO_PROCESS,0);
    printf("Current Priority is %d\n",prior);
	free(pData);

	exit(0);
}

