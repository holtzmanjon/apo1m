#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <stdexcept>
#include <algorithm>

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
        std::cout << "Apogee Alta Streaming TDI Sample" << std::endl;
        
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

		// Query user for number of TDI rows
		uint16_t NumTdiRows = 0;
        std::cout << "Number of TDI Rows:  " << std::endl;
        std::cin >> NumTdiRows;
	
		// Set the rows
        cam.SetTdiRows( NumTdiRows );
        std::cout <<  "Image to contain " <<  cam.GetTdiRows() <<" rows." << std::endl;

		// Query user for TDI rate
		double TdiRate = 0.0;
        std::cout << "Interval between rows (TDI rate):  " << std::endl;
        std::cin >> TdiRate;

		// Set the rate
        cam.SetTdiRate( TdiRate );
        std::cout <<  "TDI rate set to " <<  cam.GetTdiRate() <<" seconds." << std::endl;

		// Toggle the camera mode for TDI
		cam.SetCameraMode( Apg::CameraMode_TDI );

		// Toggle the sequence download variable
        cam.SetBulkDownload( false );
	
		// Set the image size
		const uint16_t cols = cam.GetRoiNumCols();
		const uint16_t rows = 1;		// Since SequenceBulkDownload = false
		std::vector<uint16_t> line( rows*cols );
		
		std::vector<uint16_t> fullImg( cam.GetTdiRows()*cols );
		
		// Do a 1s light frame
		std::cout << "Starting camera exposure..." << std::endl;
		cam.StartExposure( 1.0, true );
		
		for ( int i = 1; i <= cam.GetTdiRows(); ++i )
		{
			while ( cam.GetTdiCounter() != i )
			{
				// no op on purpose
			}
			
			// Get the line data from the camera
			std::cout << "Retrieving TDI row " << i << " from camera" << std::endl;
			cam.GetImage( line );
			
			//copy the line into the full buffer
			uint16_t offset = i * cols;
			std::vector<uint16_t>::const_iterator start = line.begin();
			std::vector<uint16_t>::const_iterator end = line.end();
			std::vector<uint16_t>::iterator fullImgStart = fullImg.begin() + offset;
			std::copy( start, end, fullImgStart );
		}

		std::cout << "Writing image to file" << std::endl;
		
		// Write the test image to an output file (overwrite if it already exists)
		apgSampleCmn::saveImg2File( (char*)&fullImg[0], "StreamingTdiData.bin",
					 fullImg.size()*sizeof(uint16_t) );
	
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