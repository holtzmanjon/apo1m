// ArcCAPI3.5Example.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include "ArcDeviceCAPI.h"
#include "ArcDeinterlaceCAPI.h"
//#include "ArcFitsFileCAPI.h"


// +----------------------------------------------------------------------------+
// |  Define true/false constants                                               |
// +----------------------------------------------------------------------------+
#define true	1
#define false	0


// +----------------------------------------------------------------------------+
// |  Define row and column constants                                           |
// +----------------------------------------------------------------------------+
#define gROWS	1024
#define gCOLS	1200


// +----------------------------------------------------------------------------+
// |  Define filename constants                                                 |
// +----------------------------------------------------------------------------+
#define gTIM	"C:\\Users\\streit\\Documents\\Dsplib\\V1.8\\MyCamera\\tim.lod"
#define gFITS	"C:\\Users\\streit\\Documents\\FITS\\Image.fit"


// +----------------------------------------------------------------------------+
// |  Define synthetic image use constant                                       |
// +----------------------------------------------------------------------------+
#define gSYN	0


// +----------------------------------------------------------------------------+
// |  Define deinterlace algorithm                                              |
// +----------------------------------------------------------------------------+
#define gDEINT_ALG	DEINTERLACE_SERIAL


// +----------------------------------------------------------------------------+
// |  Define expose callback                                                    |
// +----------------------------------------------------------------------------+
void ExposeCallback( float fElapsedTime )
{
	printf( "\tElapsed Time: %f\r", fElapsedTime );
}


// +----------------------------------------------------------------------------+
// |  Define readout callback                                                   |
// +----------------------------------------------------------------------------+
void ReadCallback( int dPixelCount )
{
	printf( "\tPixel Count: %d\r", dPixelCount );
}



// +----------------------------------------------------------------------------+
// |  Main function                                                             |
// +----------------------------------------------------------------------------+
int main()
{
	const char**	pszDevList		= NULL;
	void*			pBuf			= NULL;
	int				dStatus			= ARC_STATUS_ERROR;
	int				i				= 0;

	printf( "\n" );
	printf( "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n" );
	printf( "+----------------------------------------------------------------------------+\n" );
	printf( "|  ARC API C Interface Example Program                                       |\n" );
	printf( "+----------------------------------------------------------------------------+\n" );
	printf( "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n" );
	printf( "\n" );

	// +----------------------------------------------------------------------------+
	// |  Read device list and open a device                                        |
	// +----------------------------------------------------------------------------+
	printf( "[] Finding all devices ... " );
	ArcDevice_FindDevices( &dStatus );
	printf( "done! Found %d devices!\n", ArcDevice_DeviceCount() );

	if ( dStatus == ARC_STATUS_ERROR )
	{
		printf( "FindDevices failed!\n" );

		exit( 1 );
	}

	printf( "[] Reading device list ... " );
	pszDevList = ArcDevice_GetDeviceStringList( &dStatus );
	printf( "done!\n" );

	if ( dStatus == ARC_STATUS_ERROR )
	{
		printf( "GetDeviceStringList failed!\n" );

		exit( 1 );
	}

	printf( "\n" );

	for ( i=0; i<ArcDevice_DeviceCount(); i++ )
	{
		printf( "\tDEV[ %d ] -> %s\n", i, pszDevList[ i ] );
	}

	printf( "\n" );

	printf( "[] Opening device #%d ( %s ) ... ", 0, pszDevList[ 0 ] );
	ArcDevice_Open_I( 0, ( 2200 * 2200 * 2 ), &dStatus );
	printf( "done!\n" );

	ArcDevice_FreeDeviceStringList();

	if ( dStatus == ARC_STATUS_ERROR )
	{
		printf( "Open failed!\n" );

		exit( 1 );
	}

	printf( "\n" );

	// +----------------------------------------------------------------------------+
	// |  Print image buffer info ( virtual address, physical address, size, etc )  |
	// +----------------------------------------------------------------------------+
	pBuf = ArcDevice_CommonBufferVA( &dStatus );

	if ( dStatus == ARC_STATUS_OK )
	{
		printf( "\tBuffer VA ..... 0x%lX\n", ( unsigned long )pBuf );
		printf( "\tBuffer PA ..... 0x%lX\n", ArcDevice_CommonBufferPA( &dStatus ) );
	}

	if ( dStatus == ARC_STATUS_OK )
	{
		printf( "\tBuffer Size ... %d bytes\n", ArcDevice_CommonBufferSize( &dStatus ) );
	}

	printf( "\n" );

	// +----------------------------------------------------------------------------+
	// |  Initialize the controller hardware                                        |
	// +----------------------------------------------------------------------------+
	if (dStatus == ARC_STATUS_OK)
	{
		int dReset = true;
		int dTDLs = true;
		int dPowerOn = true;

		printf( "[] Controller setup ... " );
		ArcDevice_SetupController(  dReset,
									dTDLs,
									dPowerOn,
									gROWS,
									gCOLS,
									gTIM,
									NULL,
									NULL,
									&dStatus );

		if ( dStatus == ARC_STATUS_OK )
		{
			printf( "done!\n" );
			printf( "\n\tReset: %d\n\tTDLs: %d\n\tPowerOn: %d\n\tRows: %d\n\tCols: %d\n\tTimFile: %s\n",
					dReset, dTDLs, dPowerOn, gROWS, gCOLS, gTIM );
		}

		else
		{
			printf( "failed!\n" );
			printf( "\n\tERROR: %s\n", ArcDevice_GetLastError() );
		}
	}

	// +----------------------------------------------------------------------------+
	// |  Toggle synthetic image mode                                               |
	// +----------------------------------------------------------------------------+
	if ( gSYN && dStatus == ARC_STATUS_OK )
	{
		int dSynMode = true;

		printf( "[] Setting Synthetic Image Mode ... " );
		ArcDevice_SetSyntheticImageMode( dSynMode, &dStatus );

		if ( dStatus == ARC_STATUS_OK )
		{
			printf( "done!\n" );
			printf( "\n\tSynthetic Image Mode: %d\n", dSynMode );
		}

		else
		{
			printf( "failed!\n" );
			printf( "\n\tERROR: %s\n", ArcDevice_GetLastError() );
		}
	}

	printf( "\n" );

	// +----------------------------------------------------------------------------+
	// |  Take an image ( start exposure )                                          |
	// +----------------------------------------------------------------------------+
	if ( dStatus == ARC_STATUS_OK )
	{
		printf( "[] Exposing ... \n\n" );
		ArcDevice_Expose( 3.6f,
			gROWS,
			gCOLS,
			ExposeCallback,
			ReadCallback,
			false,
			&dStatus );

		if ( dStatus == ARC_STATUS_OK )
		{
			printf( "\n\n" );
		}

		else
		{
			printf( "\n\tERROR: %s\n", ArcDevice_GetLastError() );
		}
	}

	// +----------------------------------------------------------------------------+
	// |  Deinterlace the image                                                     |
	// +----------------------------------------------------------------------------+
	if ( dStatus == ARC_STATUS_OK )
	{
		printf( "[] Deinterlacing ... " );
		ArcDeinterlace_RunAlg( pBuf,
							   gROWS,
							   gCOLS,
							   gDEINT_ALG,
							   &dStatus );

		if ( dStatus == ARC_STATUS_OK )
		{
			printf( "done!\n" );
			printf( "\n\tDeinterlacing Alg: %d\n", gDEINT_ALG );
		}

		else
		{
			printf( "failed!\n" );
			printf( "\n\tERROR: %s\n", ArcDevice_GetLastError() );
		}

		printf( "\n\n" );
	}

	printf( "\n" );

	// +----------------------------------------------------------------------------+
	// |  Write the image to a FITS file                                            |
	// +----------------------------------------------------------------------------+
	if ( dStatus == ARC_STATUS_OK )
	{

		printf( "[] Saving FITS ... " );
		//ArcFitsFile_Create( gFITS, gROWS, gCOLS, FITS_BPP16, false, &dStatus );

		if ( dStatus == ARC_STATUS_OK )
		{
			//ArcFitsFile_Write( pBuf, &dStatus );

			printf( "done!\n" );

			printf( "\n\tFITS File: %s\n", gFITS );
		}

		else
		{
			printf( "failed!\n" );
			printf( "\n\tERROR: %s\n", ArcDevice_GetLastError() );
		}
	}

	printf( "\n" );

	// +----------------------------------------------------------------------------+
	// |  Close the device                                                          |
	// +----------------------------------------------------------------------------+
	printf( "[] Closing device ... " );
	ArcDevice_Close();
	printf( "done!\n" );

	printf( "\n" );

	return 0;
}
