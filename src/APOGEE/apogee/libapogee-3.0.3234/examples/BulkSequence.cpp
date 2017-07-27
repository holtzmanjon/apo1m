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
        std::cout << "Apogee Alta Bulk Sequence Sample" << std::endl;
        
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

        // Query user for number of images in the sequence
        uint16_t NumImages = 0;
        std::cout << "Number of images in the sequence:  " << std::endl;
        std::cin >> NumImages;
        
        // Set the image count
        cam.SetImageCount( NumImages );
        std::cout <<  "Preparing sequence of " <<  cam.GetImageCount() <<" images." << std::endl;

		// row*cols*num images*2 must be less than 32MBytes
		uint16_t startRow = 0;
		std::cout << "Starting row:" << std::endl;
        std::cin >> startRow;
		cam.SetRoiStartRow( startRow );
		
		uint16_t numRoiRows = 0;
		std::cout << "Number of ROI rows:" << std::endl;
        std::cin >> numRoiRows;
		cam.SetRoiNumRows( numRoiRows );
		
		uint16_t startCol = 0;
		std::cout << "Starting col:" << std::endl;
        std::cin >> startCol;
		cam.SetRoiStartCol( startCol );
		
		uint16_t numRoiCols = 0;
		std::cout << "Number of ROI cols:" << std::endl;
        std::cin >> numRoiCols;
		cam.SetRoiNumCols( numRoiCols );
		
        // Toggle the sequence download variable
        cam.SetBulkDownload( true );
        
		// For visual clarification, enable an LED light to see when the camera is
		// waiting on a trigger to arrive.  Enable the other LED to see when the 
		// camera is flushing.
		cam.SetLedMode( Apg::LedMode_EnableAll );
		cam.SetLedAState( Apg::LedState_ImageActive );
		cam.SetLedBState( Apg::LedState_Flushing );
		
        // fetch the default image size
		const uint16_t rows = cam.GetRoiNumRows();
		const uint16_t cols = cam.GetRoiNumCols();
        const uint16_t z = cam.GetImageCount();
		
		//allocate vector for the image data
		std::vector<uint16_t> img( rows*cols*z );
	
	   // Do a 0.1s light frame
       std::cout << "Starting camera exposure..." << std::endl;
       cam.StartExposure( 0.1, true );
       
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
	
        // Write the test images to different output file (overwrite if it already exists)
        // In this process, we are dividing the single image buffer into the separate
        // images that comprise the bulk data set.
        std::cout << "Writing " << z << " images to files " << std::endl;
        const uint32_t frameSize = rows*cols;
        
        for( uint16_t i=0; i<z; ++i )
        {
            std::stringstream ss;
            ss << "BulkImage" << i << ".bin";
            int offset = i*frameSize;
            
            apgSampleCmn::saveImg2File( (char*)&img[offset], ss.str(),
                         frameSize*sizeof(uint16_t) );
            
        }
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
