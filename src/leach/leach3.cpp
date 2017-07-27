// +----------------------------------------------------------------------------------+
// | File: ArcAPIEx3Simple.cpp
// +----------------------------------------------------------------------------------+
// | Description: This file demonstates a simple use of the ARC API 3.0 for both
// | the PCI and PCIe interfaces. The first device found is used to setup an attached
// | controller and take an exposure.
// |
// | Author: Scott Streit
// | Date: March 3, 2011
// +----------------------------------------------------------------------------------+
#include <iostream>
#include <iomanip>
#include <string>
#include "CArcDevice.h"
#include "CArcPCIe.h"
#include "CArcPCI.h"
#include "CDeinterlace.h"
#include "CFitsFile.h"
#include "CExpIFace.h"
using namespace std;
using namespace arc;
#define USAGE( x ) \
    ( cout << endl << "Usage: " << x \
    << " [PCI | PCIe] [options]" << endl << endl \
    << "\toptions:" << endl \
    << "\t---------------------------------------" << endl \
     << "\t-f [DSP lod filename : Default=tim.lod] " << endl \
    << "\t-e [exp time (s) : Default=0.5]" << endl \
    << "\t-r [rows : Default=512] " << endl \
    << "\t-c [cols : Default=600]" << endl \
    << "\t-d [deint alg : Default=CDeinterlace::DEINTERLACE_NONE]" \
    << endl << endl << "\tDeinterlace Values:" \
    << "\n\t---------------------------------------" \
    << "\n\t0: None\n\t1: Parallel\n\t2: Serial" \
    << "\n\t3: CCD Quad\n\t4: IR Quad\n\t5: CDS IR QUAD" \
    << "\n\t6: Hawaii RG" \
    << "\n\t7: STA1600" << endl << endl )
// ------------------------------------------------------
// Function prototypes
// ------------------------------------------------------
std::string SetDots( const char *cStr );
// ------------------------------------------------------
// Exposure Callback Class
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
    }
};
// ------------------------------------------------------
// Main program
// ------------------------------------------------------
int main( int argc, char **argv )
{
std::string sTimFile = "tim.lod";
float fExpTime = 0.5;
long lNumOfFrames = 100;
long lRows = 512;
long lCols = 600;
long lDeintAlg = CDeinterlace::DEINTERLACE_NONE;
bool bAbort = false;
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
        lRows = atol( argv[ i + 1 ] );
        cout << "Number Of Rows Set: " << lRows << endl;
    }
    else if ( sArgv.compare( "-c" ) == 0 && argc >= ( i + 1 ) )
    {
        lCols = atol( argv[ i + 1 ] );
        cout << "Number Of Cols Set: " << lCols << endl;
    }
    else if ( sArgv.compare( "-d" ) == 0 && argc >= ( i + 1 ) )
    {
        lDeintAlg = atol( argv[ i + 1 ] );
        cout << "Deinterlace Set: " << lDeintAlg << endl;
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
if ( sDev.compare( "PCIe" ) == 0 ) { CArcPCIe::FindDevices(); }
else { CArcPCI::FindDevices(); }
cout << "done!" << endl;
//
// Open a driver/device connection
//
cout << SetDots( "Opening first device" );
pArcDev.get()->Open( 0, 4200 * 4200 * sizeof( unsigned short ) );
cout << "done! Image Buffer Size: " << pArcDev.get()->CommonBufferVA()
 << endl;
//
// Setup the controller
//
cout << SetDots( "Setting up controller" );
pArcDev.get()->SetupController( true, // Reset Controller
 true, // Test Data Link
 true, // Power On
 lRows, // Image row size
 lCols, // Image col size
 sTimFile.c_str() ); // DSP timing file
cout << "done!" << endl;
//
// Expose
//
pArcDev.get()->Expose( fExpTime, lRows, lCols, bAbort, &cExposeListener );
//
// Deinterlace the image
//
cout << SetDots( "Deinterlacing image" );
CDeinterlace cDlacer;
cDlacer.RunAlg( pArcDev.get()->CommonBufferVA(),
lRows,
lCols,
lDeintAlg );
cout << "done!" << endl;
//
// Save the image to FITS
//
cout << SetDots( "Writing FITS" );
CFitsFile cFits( "Image.fit", lRows, lCols );
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
// | SetDots
// +--------------------------------------------------------------------------------
// | This function just prints dots (...) for the output.
// +--------------------------------------------------------------------------------
std::string SetDots( const char *cStr )
{
std::string sStr( cStr );
for ( int i=sStr.length(); i<40; i++ )
sStr.append( "." );
return sStr;
}
