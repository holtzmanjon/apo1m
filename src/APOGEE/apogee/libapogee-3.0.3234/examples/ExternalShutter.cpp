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
        std::cout << "Apogee Alta External Shutter Sample" << std::endl;
        
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
        std::cout << "Connected to Alta camera " << std::endl;
		apgSampleCmn::printInfo( cam.GetModel(),  cam.GetMaxImgRows(),
								cam.GetMaxImgCols() );

		// For visual clarification, enable an LED light to see when the camera is
		// waiting on a trigger to arrive.  Enable the other LED to see when the 
		// camera is flushing.
		cam.SetLedMode( Apg::LedMode_EnableAll );
		cam.SetLedAState( Apg::LedState_ExtTriggerWaiting );
		cam.SetLedBState( Apg::LedState_ImageActive );
		
		// Get the image size
		const uint16_t rows = cam.GetRoiNumRows();
		const uint16_t cols = cam.GetRoiNumCols();
		
		//allocate vector for the image data
		std::vector<uint16_t> img( rows*cols );
		
		// External operations
		cam.SetExternalTrigger( true,
							   Apg::TriggerMode_ExternalShutter,
							   Apg::TriggerType_Group );
		
		cam.SetExternalTrigger( false,
							   Apg::TriggerMode_ExternalReadoutIo,
							   Apg::TriggerType_Group) ;
		
		cam.SetIoPortAssignment( 0x08 );

		// Do a light frame 
		std::cout << "Starting camera exposure 1" << std::endl;
		cam.StartExposure( 0.001, true );
		
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
		std::cout << "Retrieving first image from camera" << std::endl;
		
		cam.GetImage( img );

		std::cout << "Writing image to file" << std::endl;
		
		// Write the test image to an output file (overwrite if it already exists)
		apgSampleCmn::saveImg2File( (char*)&img[0], "ExternalShutterData1.bin",
					 img.size()*sizeof(uint16_t) );
	
		std::cout << "Write complete" << std::endl;
		
		// Do a light frame 
		std::cout << "Starting camera exposure 2" << std::endl;
		cam.StartExposure( 0.001, true );
		
		// Check camera status to make sure image data is ready
		status = Apg::Status_Flushing;
		while( Apg::Status_ImageReady !=  status )
		{
			status = cam.GetImagingStatus();
			//make sure there isn't an error
			//throw here if there is
			apgSampleCmn::checkStatus( status );
		}


		// Get the image data from the camera
		std::cout << "Retrieving second image from camera." << std::endl;
		
		cam.GetImage( img );

		std::cout << "Writing image 2 file" << std::endl;
		
		// Write the test image to an output file (overwrite if it already exists)
		apgSampleCmn::saveImg2File( (char*)&img[0], "ExternalShutterData2.bin",
					 img.size()*sizeof(uint16_t) );
		
		std::cout << "Write complete" << std::endl;
		
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
