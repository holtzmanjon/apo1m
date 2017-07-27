#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <stdexcept>

#include "apogee/Ascent.h"
#include "apogee/FindDeviceUsb.h"
#include "apogee/CameraInfo.h"
#include "apogee/ApogeeFilterWheel.h"
#include "apgSampleCmn.h"

////////////////////////////
// MAIN
int main()
{
	try
	{
        std::cout << "Apogee Ascent Example" << std::endl;
        
              
        // for this example only looking for only one usb device
        std::string ioInterface("usb");
        FindDeviceUsb look4cam;
        std::string msg = look4cam.Find();
                
		if( !apgSampleCmn::IsAscent( msg ) )
		{
			std::runtime_error except( "Ascent camera not connected" );
			throw except;
		}
        std::string addr = apgSampleCmn::GetUsbAddress( msg );
        uint16_t id = apgSampleCmn::GetID( msg );
        uint16_t frmwrRev = apgSampleCmn::GetFrmwrRev( msg );
        
        //create the camera object and conneting to the camera
		// assuming alta camera for this example, use the Ascent
		// camera object for Ascent cameras.
        Ascent cam;
        cam.OpenConnection( ioInterface, addr, frmwrRev, id );
		cam.Init();
		
        // print basic camera info
        std::cout << "Connected to Ascent camera " << std::endl;
		apgSampleCmn::printInfo( cam.GetModel(),  cam.GetMaxImgRows(),
								cam.GetMaxImgCols() );
        
		std::string fwAnswer;
		std::cout << "Is a filter wheel connected? [y/n]" << std::endl;
        std::cin >> fwAnswer;
		
		const bool fwPresent = ( 0 == fwAnswer.compare("y" ) ? true : false );
		
		if( fwPresent )
		{
			std::cout << "Enter the filter wheel type [CFW25_6R=0, CFW31_8R=1]" << std::endl;
			int32_t result = 0;
			std::cin >> result;
			
			Ascent::FilterWheelType type = Ascent::FW_UNKNOWN_TYPE;
			switch( result )
			{
				case 0:
					 type = Ascent::CFW25_6R;
				break;
			
				case 1:
					 type = Ascent::CFW31_8R;
				break;
			
				default:
				{
					std::runtime_error except( "Invalid filter wheel type entered." );
					throw except;
				}
				break;
			}
			
			// set the fw type
			cam.FilterWheelOpen( type );
		}
		
		
		
        // fetch the default image size
		const uint16_t rows = cam.GetRoiNumRows();
		const uint16_t cols = cam.GetRoiNumCols();
		
		//allocate vector for the image data
		std::vector<uint16_t> img( rows*cols );
		
		
		std::vector<uint16_t> fwPos;
		fwPos.push_back( 2 );
		fwPos.push_back( 4 );
		//take 2 pictures
		for( int i = 0; i < 2; ++i )
		{
			//move the filter wheel
			if( fwPresent )
			{
				std::cout << "Moving the filter wheel to postion " << fwPos.at(i) << std::endl;
				cam.SetFilterWheelPos( fwPos.at(i) );
				
				ApogeeFilterWheel::Status fwStatus = ApogeeFilterWheel::UNKNOWN_STATUS;
				
				while( ApogeeFilterWheel::READY != fwStatus )
				{
					fwStatus = cam.GetFilterWheelStatus();
				}
				
				if( cam.GetFilterWheelPos() != fwPos.at(i) )
				{
					std::runtime_error except( "Filter wheel moved to incorrect position" );
					throw except;
				}
				
				std::cout << "Filter wheel ready at position " << cam.GetFilterWheelPos() << std::endl;
			}
			
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
            ss << "AscentImage" << i << ".bin";
			apgSampleCmn::saveImg2File( (char*)&img[0], ss.str(),
						 img.size()*sizeof(uint16_t) );
		}
		
        std::cout << "Closing connection to camera " << std::endl;
		
		if( fwPresent )
		{
			cam.FilterWheelClose();
		}
		
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
