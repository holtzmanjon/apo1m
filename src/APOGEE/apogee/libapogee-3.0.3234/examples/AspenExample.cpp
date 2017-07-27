#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <stdexcept>

#include "apogee/Aspen.h"
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
        std::cout << "Apogee Aspen Example" << std::endl;
        
        // use this for ethernet connections
        // std::string ioInterface("ethernet");
        // FindDeviceEthernet look4cam;
        // std::string subnet = "192.168.0.255";
        // std::string msg = look4cam.Find( subnet );
				// std::string addr = apgSampleCmn::GetEthernetAddress( msg );
        
              
        // for this example only looking for only one usb device
        std::string ioInterface("usb");
        FindDeviceUsb look4cam;
        std::string msg = look4cam.Find();
        
		if( !apgSampleCmn::IsAspen( msg ) )
		{
			std::runtime_error except( "Aspen camera not connected" );
			throw except;
		}
        std::string addr = apgSampleCmn::GetUsbAddress( msg );
        uint16_t id = apgSampleCmn::GetID( msg );
        uint16_t frmwrRev = apgSampleCmn::GetFrmwrRev( msg );
        
        //create the camera object and conneting to the camera
		// assuming Aspen camera for this example
        Aspen cam;
        cam.OpenConnection( ioInterface, addr, frmwrRev, id );
		cam.Init();
		
        // print basic camera info
        std::cout << "Connected to Aspen camera " << std::endl;
		apgSampleCmn::printInfo( cam.GetModel(),  cam.GetMaxImgRows(),
								cam.GetMaxImgCols() );
        
        // fetch the default image size
		const uint16_t rows = cam.GetRoiNumRows();
		const uint16_t cols = cam.GetRoiNumCols();
		
		//allocate vector for the image data
		std::vector<uint16_t> img( rows*cols );
		
		
		//take 2 pictures
		for( int i = 0; i < 2; ++i )
		{
			// start 0.1 sec light (shutter will open) expousre
			const double exposeTime = 0.1;
			std::cout << "Starting a " << exposeTime << " sec light exposure" << std::endl;
			cam.StartExposure( exposeTime, true );
			
			// Check camera status to make sure image data is ready
			Apg::Status status = Apg::Status_Flushing;
			while( Apg::Status_ImageReady !=  status )
			{
				status = cam.GetImagingStatus();
				//make sure there isn't an error
				//throw here if there is
				apgSampleCmn::checkStatus( status );
			}
	
			std::cout << "Retrieving image data from camera" << std::endl;
	
			cam.GetImage( img );
	
			std::cout << "Writing image to file" << std::endl;
			
			// Write the test image to an output file (overwrite if it already exists)
			std::stringstream ss;
            ss << "AspenImage" << i << ".bin";
			apgSampleCmn::saveImg2File( (char*)&img[0], ss.str(),
						 img.size()*sizeof(uint16_t) );
		}
		
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
