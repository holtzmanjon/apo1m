#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <stdexcept>

#ifdef WIN_OS
#include <memory>
#else
#include <tr1/memory>
#endif

#include "apogee/Alta.h"
#include "apogee/ApogeeCam.h"
#include "apogee/FindDeviceUsb.h"
//#include "apogee/FindDeviceEthernet.h"
#include "apogee/CameraInfo.h"
#include "apgSampleCmn.h"

namespace
{
	// break up the larger device string into a vector of individual devices
	std::vector<std::string> GetDeviceVector( const std::string & msg )
	{
		std::vector<std::string> devices;
		const std::string startDelim("<d>");
		const std::string stopDelim("</d>");
	
		size_t pos = 0;
		bool find = true;
		while( find )
		{
			size_t posStart = msg.find( startDelim, pos );
			if( std::string::npos == posStart )
			{
				break;
			}
	
			size_t posStop = msg.find( stopDelim, posStart+1 );
			if( std::string::npos == posStop )
			{
				break;
			}
	
			size_t strLen = (posStop - posStart) - startDelim.size();
	
			std::string sub = msg.substr( posStart+startDelim.size(), strLen );
	
			devices.push_back( sub );
	
			pos = 1+posStop;
		}

		return devices;
	}
}
////////////////////////////
// MAIN
int main()
{
	try
	{
        std::cout << "Apogee Multi USB Camera Sample" << std::endl;
        
        // for this example only looking for cameras on the
		// usb bus
        std::string ioInterface("usb");
        FindDeviceUsb look4cam;
        std::string msg = look4cam.Find();
		
		std::vector< std::string> deviceStrings = GetDeviceVector( msg );
		 
		if( deviceStrings.size() < 2 )
		{
			std::string errMsg("Error less than 2 cameras attached");
			std::runtime_error except( errMsg );
			throw except;
		}
		 
		std::vector< std::tr1::shared_ptr<ApogeeCam> > camVector;
		 
		std::vector<std::string>::iterator iter;
		int i =0;
		for( iter = deviceStrings.begin(); iter != deviceStrings.end(); ++iter, ++i )
		{
			if( apgSampleCmn::IsDeviceFilterWheel( (*iter) ) )
			{
				std::string errMsg = "Device is a filter wheel = " + (*iter);
				std::runtime_error except( errMsg );
				throw except;
			}
		
			std::string addr = apgSampleCmn::GetUsbAddress( (*iter) );
			uint16_t id = apgSampleCmn::GetID( (*iter) );
			uint16_t frmwrRev = apgSampleCmn::GetFrmwrRev( (*iter) );
			
			//create the camera object and conneting to the camera
			// assuming alta camera for this example, use the Ascent
			// camera object for Ascent cameras.
			std::tr1::shared_ptr<ApogeeCam> cam( new Alta() );
			cam->OpenConnection( ioInterface, addr, frmwrRev, id );
			cam->Init();
			
			// print basic camera info
			std::cout << "Alta camera " << i << " connectd and initialized" << std::endl;
			apgSampleCmn::printInfo( cam->GetModel(),  cam->GetMaxImgRows(),
									cam->GetMaxImgCols() );
			
			camVector.push_back( cam );
		}

		// loop through the camera and take an exposure with
		//each one
		std::vector< std::tr1::shared_ptr<ApogeeCam> >::iterator camIter;
		i = 0;
		for( camIter = camVector.begin(); camIter != camVector.end(); ++camIter, ++i )
		{
			// fetch the default image size
			const uint16_t rows = (*camIter)->GetRoiNumRows();
			const uint16_t cols = (*camIter)->GetRoiNumCols();
			
			//allocate vector for the image data
			std::vector<uint16_t> img( rows*cols );
			
			// start 0.1 sec light (shutter will open) expousre
			const double exposeTime = 0.1;
			std::cout << "Starting a " << exposeTime << " sec light exposure" << std::endl;
			(*camIter)->StartExposure( exposeTime, true );
			
			// Check camera status to make sure image data is ready
			Apg::Status status = Apg::Status_Flushing;
			while( Apg::Status_ImageReady !=  status )
			{
				status = (*camIter)->GetImagingStatus();
				//make sure there isn't an error
				//throw here if there is
				apgSampleCmn::checkStatus( status );
			}

			std::cout << "Retrieving image data from camera " << i << std::endl;

			(*camIter)->GetImage( img );
	
			std::cout << "Writing image to file" << std::endl;
			
			// Write the test image to an output file (overwrite if it already exists)
			std::stringstream ss;
            ss << "CamImage" << i << ".bin";
			
			apgSampleCmn::saveImg2File( (char*)&img[0], ss.str(),
						 img.size()*sizeof(uint16_t) );
		}

		// close the connection to all the cameras
		i = 0;
		for( camIter = camVector.begin(); camIter != camVector.end(); ++camIter, ++i )
		{
			std::cout << "Closing connection to camera " << i << std::endl;
			(*camIter)->CloseConnection();
		}
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