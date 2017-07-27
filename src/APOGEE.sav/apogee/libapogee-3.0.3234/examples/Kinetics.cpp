#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <stdexcept>

#include "apogee/Alta.h"
#include "apogee/FindDeviceUsb.h"
//#include "apogee/FindDeviceEthernet.h"
#include "apogee/CameraInfo.h"
#include "apgSampleCmn.h"

////////////////////////////
// MAIN
int main()
{
	try
	{
        std::cout << "Apogee Alta Kinects Sample" << std::endl;
        
        // use this for ethernet cameras
        // std::string ioInterface("ethernet");
        // FindDeviceEthernet look4cam;
        // std::string subnet = "192.168.0.255";
        // std::string msg = look4cam.Find( subnet );
		// std::string addr = apgSampleCmn::GetEthernetAddress( msg );
        
        // for this example only looking for only one usb device
        std::string ioInterface("usb");
        FindDeviceUsb look4cam;
        std::string msg = look4cam.Find();
        std::string addr = apgSampleCmn::GetUsbAddress( msg );
		
		
        uint16_t id = apgSampleCmn::GetID( msg );
        uint16_t frmwrRev = apgSampleCmn::GetFrmwrRev( msg );
        
        //create the camera object and conneting to the camera
		// assuming alta camera for this example, use the Ascent
		// camera object for Ascent cameras.
        Alta cam;
        cam.OpenConnection( ioInterface, addr, frmwrRev, id );
		
		// initalize the camrea
        cam.Init();
        std::cout << "Camera Initalized" << std::endl;
		
        // print basic camera info
		apgSampleCmn::printInfo( cam.GetModel(),  cam.GetMaxImgRows(),
								cam.GetMaxImgCols() );
        
		// For visual clarification, enable an LED light to see when the camera is
		// waiting on a trigger to arrive.  Enable the other LED to see when the 
		// camera is flushing.
		cam.SetLedMode( Apg::LedMode_EnableAll );
		cam.SetLedAState( Apg::LedState_ImageActive );
		cam.SetLedBState( Apg::LedState_Flushing );
		
        // fetch the default image size
		const uint16_t rows = cam.GetRoiNumRows();
		const uint16_t cols = cam.GetRoiNumCols();
		
		//allocate vector for the image data
		std::vector<uint16_t> img( rows*cols );

		// Set the camera mode to Kinetics
		cam.SetCameraMode( Apg::CameraMode_Kinetics );
	
		// 4 sections
		const uint16_t NumSections = 4;
	
		// Set our section variables
		cam.SetKineticsSections( NumSections );
		cam.SetKineticsSectionHeight( rows/NumSections );
	
		// Set the section rate...using 0.2s arbitrarily
		cam.SetKineticsShiftInterval( 0.2 );

		// Begin the exposure process
		std::cout << "Starting imaging process..." << std::endl;
		cam.StartExposure( 0.001, false );
		
		// Tell the user something informative...
		std::cout << "Kinetics imaging in progress..." << std::endl;

		// Check camera status to make sure image data is ready
		Apg::Status status = Apg::Status_Flushing;
		while( Apg::Status_ImageReady !=  status )
		{
			status = cam.GetImagingStatus();
			//make sure there isn't an error
			//throw here if there is
			apgSampleCmn::checkStatus( status );
		}

		// Get the image data from the camera
		std::cout << "Retrieving image data from camera..." << std::endl;
		
		cam.GetImage( img );

		std::cout << "Writing image to file" << std::endl;
		
		// Write the test image to an output file (overwrite if it already exists)
		apgSampleCmn::saveImg2File( (char*)&img[0], "KineticsData.bin",
					 img.size()*sizeof(uint16_t) );
		
		std::cout << "Write complete" << std::endl;
		
		// Set the camera mode back to Normal
		cam.SetCameraMode( Apg::CameraMode_Normal );

		std::cout << "Closing connection to camera " << std::endl;
		cam.CloseConnection();
	}
	catch( std::exception & err )
	{
		std::cout << "std exception what = " << err.what() << std::endl;
		return 1;
	}
	catch( ... )
	{
		std::cout << "Unknown exception caught." << std::endl;
		return 1;
	}

	return 0;
}