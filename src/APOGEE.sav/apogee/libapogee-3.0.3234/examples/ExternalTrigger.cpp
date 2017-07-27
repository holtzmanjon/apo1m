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
        std::cout << "Apogee Alta External Trigger Sample" << std::endl;
        
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
		
		///////////////////////////////////////////////////////////////////
		// Single Hardware Trigger Example
		///////////////////////////////////////////////////////////////////
		
		// Get the image size
		const uint16_t rows = cam.GetRoiNumRows();
		const uint16_t cols = cam.GetRoiNumCols();
		
		//allocate vector for the image data
		std::vector<uint16_t> img( rows*cols );
		
		// First we'll do a single triggered exposure.  This requires the 
		// "TriggerNormalGroup" property to be enabled
		cam.SetExternalTrigger( true,
							   Apg::TriggerMode_Normal,
							   Apg::TriggerType_Group );

		// We will make our exposure a 0.1s light frame.  Even though this
		// is a triggered exposure, we still need the Expose() method to be
		// called to set up our exposure
		const double exposeTime = 0.1;
		std::cout << "Starting a single triggered exposure of ";
		std::cout << exposeTime << " sec." << std::endl;
		cam.StartExposure( exposeTime, true );
		
		std::cout << "Waiting on trigger..." << std::endl;
	
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
		std::cout << "Retrieving image from camera" << std::endl;
		
		cam.GetImage( img );

		std::cout << "Writing image to file" << std::endl;
		
		// Write the test image to an output file (overwrite if it already exists)
		apgSampleCmn::saveImg2File( (char*)&img[0], "ExtTriggerData.bin",
					 img.size()*sizeof(uint16_t) );
	
		std::cout << "Write complete" << std::endl;
		
		// We're going to set this to "true" again in the next example, but 
		// for good form, we'll return our state to non-triggered images
		cam.SetExternalTrigger( false,
							   Apg::TriggerMode_Normal,
							   Apg::TriggerType_Group );

		///////////////////////////////////////////////////////////////////
		// Sequenced (Internal) Hardware Trigger Example
		///////////////////////////////////////////////////////////////////
	
		// NOTE: The following example uses the camera engine's internal
		// capability to do sequences of images.  An application could 
		// also easily do a loop of single triggered images in order to 
		// achieve the same result, but driven by software/the application.
	
		// Do a sequence of triggered exposures.  This requires both the 
		// "TriggerNormalGroup" and "TriggerNormalEach" properties to be 
		// enabled.  TriggerNormalGroup will enable a trigger for the first
		// image.  TriggerNormalEach will enable a trigger for each 
		// subsequent image in the sequence.
		cam.SetExternalTrigger( true,
							   Apg::TriggerMode_Normal,
							   Apg::TriggerType_Group );
		
		cam.SetExternalTrigger( true,
							   Apg::TriggerMode_Normal,
							   Apg::TriggerType_Each );
		
		// Toggle the sequence download variable
		cam.SetBulkDownload( false );
		
		//Query user for number of images in the sequence
        uint16_t NumImages = 0;
        std::cout << "Number of images in the sequence:  " << std::endl;
        std::cin >> NumImages;
        
        // Set the image count
        cam.SetImageCount( NumImages );
        std::cout <<  "Preparing sequence of " <<  cam.GetImageCount() <<" images." << std::endl;

		// For visual clarification, enable an LED light to see when the camera is
		// waiting on a trigger to arrive.  Enable the other LED to see when the 
		// camera is flushing.
		cam.SetLedMode( Apg::LedMode_EnableAll );
		cam.SetLedAState( Apg::LedState_ExtTriggerWaiting );
		cam.SetLedBState( Apg::LedState_ImageActive );
		
		// As with single exposures, we must start the trigger process with the
		// Expose method.  We only need to call Expose once to kick off the entire
		// sequence process.
		std::cout << "Starting sequence of triggered exposures of ";
		std::cout << exposeTime << " sec." << std::endl;
		cam.StartExposure( exposeTime, true );

		for( uint16_t i=1; i<=NumImages; ++i )
		{
			std::cout << "Waiting on trigger for image " << i  << std::endl;
	
			// For sequences of images, the correct usage is to use the 
			// SequenceCounter property in order to correctly determine when 
			// an image is ready for download.
			while ( cam.GetImgSequenceCount() != i )
			{
				//no op on purpose	
			}
	
			// Get the image data from the camera
			std::cout << "Retrieving image data from camera (Image #";
			std::cout << i << ")" << std::endl;
        
			cam.GetImage( img );
		
			std::stringstream ss;
            ss << "ExtTriggerData" << i << ".bin";
			
			std::cout << "Writing image " << i << " to file" << std::endl;
			
			apgSampleCmn::saveImg2File( (char*)&img[0], ss.str(),
                         img.size()*sizeof(uint16_t) );
			
			std::cout << "Write complete" << std::endl;
		}

		// Return our state to non-triggered images
		cam.SetExternalTrigger( false,
								   Apg::TriggerMode_Normal,
								   Apg::TriggerType_Group );
			
		cam.SetExternalTrigger( false,
								   Apg::TriggerMode_Normal,
								   Apg::TriggerType_Each );
		
		// Reset our image count to 1
		cam.SetImageCount( 1 );
		
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