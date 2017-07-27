// +-----------------------------------------------------------------------------------+
// |  File:  ArcAPI35Ex_1.cpp                                                          |
// +-----------------------------------------------------------------------------------+
// |  Description:  This file demonstrates a simple use of the ARC API 3.5 for both    |
// |  the PCI and PCIe interfaces. The first device found is used to setup an attached |
// |  controller and take an exposure.                                                 |
// |                                                                                   |
// |  Author:       Scott Streit @ Astronomical Research Cameras, Inc.                 |
// |  Date:         March 26, 2012                                                     |
// +-----------------------------------------------------------------------------------+

#include <iostream>
#include <iomanip>
#include <string>

#include "CArcDevice.h"
#include "CArcPCIe.h"
#include "CArcPCI.h"
#include "CArcDeinterlace.h"
#include "CArcFitsFile.h"
#include "CExpIFace.h"

using namespace std;
using namespace arc;
using namespace arc::device;
using namespace arc::deinterlace;
using namespace arc::fits;


#define USAGE( x ) \
            ( cout << endl << "Usage: " << x \
                        << " [PCI | PCIe] [options]" << endl << endl \
                        << "\toptions:" << endl \
                        << "\t---------------------------------------" << endl \
                        << "\t-f [DSP lod filename : Default=tim.lod] " << endl \
                        << "\t-e [exp time (s) : Default=0.5]" << endl \
                        << "\t-r [rows : Default=1024] " << endl \
                        << "\t-c [cols : Default=1200]" << endl \
                        << "\t-d [deint alg : Default=CDeinterlace::DEINTERLACE_NONE]" \
                        << endl << endl << "\tDeinterlace Values:" \
                        << "\n\t---------------------------------------" \
                        << "\n\t0: None\n\t1: Parallel\n\t2: Serial" \
                        << "\n\t3: CCD Quad\n\t4: IR Quad\n\t5: CDS IR QUAD" \
                        << "\n\t6: Hawaii RG" \
                        << "\n\t7: STA1600" << endl << endl )


// ------------------------------------------------------
//  Function prototypes
// ------------------------------------------------------
std::string SetDots( const char *cStr );


// ------------------------------------------------------
//  Exposure Callback Class
// ------------------------------------------------------
class CExposeListener : public CExpIFace
{
      void ExposeCallback( float fElapsedTime )
      {
            cout << "Elapsed Time: " << fElapsedTime << endl;
}
      void ReadCallback( int dPixelCount )
      {
            cout << "Pixel Count: " << dPixelCount << endl;
} };


// ------------------------------------------------------
//  Main program
// ------------------------------------------------------
int main( int argc, char **argv )
{
	std::string sTimFile		= "tim.lod";
	float       fExpTime		= 0.5;
	int			dRows			= 1024;
	int	        dCols			= 1200;
	int	        dDeintAlg		= CArcDeinterlace::DEINTERLACE_NONE;
	bool        bAbort			= false;

	CExposeListener cExposeListener;

	//
	// Set host device
	//
	if ( argc < 2 )
	{
		  cout << "Error: Invalid number of minimum parameters!" << endl;
		  USAGE( argv[ 0 ] );
		  cout << endl;
		  exit( EXIT_FAILURE );
	}

	string sDev = argv[ 1 ];

	if ( sDev.compare( "PCIe" ) != 0 && sDev.compare( "PCI" ) != 0 )
	{
		  cout << "Error: Invalid device parameter: " << sDev << endl;
		  USAGE( argv[ 0 ] );
		  cout << endl;
		  exit( EXIT_FAILURE );
	}

	//
	// Handle program arguments
	//
	for ( int i=2; i<argc; i++ )
	{
		  std::string sArgv = argv[ i ];

		  if ( sArgv.compare( "-f" ) == 0 && argc >= ( i + 1 ) )
		  {
				sTimFile = argv[ i + 1 ];
				cout << "Timing File Set: " << sTimFile << endl;
		  }

		  else if ( sArgv.compare( "-e" ) == 0 && argc >= ( i + 1 ) )
		  {
				fExpTime = float( atof( argv[ i + 1 ] ) );
				cout << "Exposure Time Set: " << fExpTime << endl;
		  }

		  else if ( sArgv.compare( "-r" ) == 0 && argc >= ( i + 1 ) )
		  {
				dRows = atoi( argv[ i + 1 ] );
				cout << "Number Of Rows Set: " << dRows << endl;
		  }

		  else if ( sArgv.compare( "-c" ) == 0 && argc >= ( i + 1 ) )
		  {
				dCols = atoi( argv[ i + 1 ] );
				cout << "Number Of Cols Set: " << dCols << endl;
		  }

		  else if ( sArgv.compare( "-d" ) == 0 && argc >= ( i + 1 ) )
		  {
				dDeintAlg = atoi( argv[ i + 1 ] );
				cout << "Deinterlace Set: " << dDeintAlg << endl;
		  }

		  else if ( sArgv.compare( "-h" ) == 0 )
		  {
				USAGE( argv[ 0 ] );
				exit( EXIT_FAILURE );
		  }
	}

	//
	// Create an instance of the ARC controller API
	//
	auto_ptr<CArcDevice> pArcDev( new CArcPCIe );

	if ( sDev.compare( "PCI" ) == 0 )
	{
		  pArcDev.reset( new CArcPCI );
	}

	cout << endl;

	try
	{
		//
		// Find all ARC PCI/e device
		//
		cout << SetDots( "Finding devices" );
		if ( sDev.compare( "PCIe" ) == 0 )
		{
			CArcPCIe::FindDevices();
		}
		else
		{
			CArcPCI::FindDevices();
		}
		cout << "done!" << endl;

		//
		// Open a driver/device connection
		//
		cout << SetDots( "Opening first device" );
		pArcDev.get()->Open( 0, 4200 * 4200 * sizeof( unsigned short ) );
		cout << "done! Image Buffer VA: 0x" << hex
			 << pArcDev.get()->CommonBufferVA()
			 << dec << endl;

		//
		// Setup the controller
		//
		cout << SetDots( "Setting up controller" );
		pArcDev.get()->SetupController( true,
										true,
										true,
										dRows,
										dCols,
										sTimFile.c_str() );
		cout << "done!" << endl;

		//
		// Expose
		//
		pArcDev.get()->Expose( fExpTime, dRows, dCols, bAbort, &cExposeListener );

		//
		// Deinterlace the image
		//
		cout << SetDots( "Deinterlacing image" );
		CArcDeinterlace cDlacer;
		cDlacer.RunAlg( pArcDev.get()->CommonBufferVA(),
						dRows,
						dCols,
						dDeintAlg );
		cout << "done!" << endl;

		//
		// Save the image to FITS
		//
		cout << SetDots( "Writing FITS" );
		CArcFitsFile cFits( "Image.fit", dRows, dCols );
		cFits.Write( pArcDev.get()->CommonBufferVA() );
		cout << "done!" << endl;

		//
		// Close the device connection
		//
		cout << SetDots( "Closing device" );
		pArcDev.get()->Close();
		cout << "done!" << endl;
	}
	catch ( std::runtime_error &e )
	{
		cout << "failed!" << endl;
		cerr << endl << e.what() << endl;

		if ( pArcDev.get()->IsReadout() )
		{
			pArcDev.get()->StopExposure();
		}

		pArcDev.get()->Close();
	}
	catch ( ... )
	{
		cerr << endl << "Error: unknown exception occurred!!!" << endl;

		if ( pArcDev.get()->IsReadout() )
		{
			pArcDev.get()->StopExposure();
		}

		pArcDev.get()->Close();
	}
}

// +--------------------------------------------------------------------------------
// |  SetDots
// +--------------------------------------------------------------------------------
// |  This function just prints dots (...) for the output.
// +--------------------------------------------------------------------------------
std::string SetDots( const char *cStr )
{
      std::string sStr( cStr );

      for ( int i=int( sStr.length() ); i<40; i++ )
	  {
            sStr.append( "." );
	  }

      return sStr;
}
