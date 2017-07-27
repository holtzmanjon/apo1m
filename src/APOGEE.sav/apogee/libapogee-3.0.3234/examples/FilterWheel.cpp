#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <stdexcept>

#include "apogee/ApogeeFilterWheel.h"
#include "apogee/FindDeviceUsb.h"
#include "apogee/CameraInfo.h"
#include "apgSampleCmn.h"

////////////////////////////
// MAIN
int main()
{
	try
	{
        std::cout << "Apogee Filter Wheel Sample" << std::endl;
                
        // for this example only looking for only one usb device
        FindDeviceUsb look4FilterWheel;
        std::string msg = look4FilterWheel.Find();
                
		if( !apgSampleCmn::IsDeviceFilterWheel( msg ) )
		{
			std::string errMsg = "Device not a filter wheel = " + msg;
			std::runtime_error except( errMsg );
			throw except;
		}
		
		std::cout << "USB filter wheel found" << std::endl;
		
		std::string addr = apgSampleCmn::GetUsbAddress( msg );
		
		ApogeeFilterWheel fw;
		
		// the user must supply the filter wheel type
		const ApogeeFilterWheel::Type type = ApogeeFilterWheel::FW50_7S;
		
		// initalize the filter wheel
		fw.Init( type, addr );
		
		std::cout << "Filter wheel Initalized" << std::endl;
		
		std::cout << "Filter wheel *" << fw.GetName().c_str();
		std::cout << "* supports " << fw.GetMaxPositions();
		std::cout << " positions." << std::endl;

		std::cout << "Current filter wheel position is ";
		std::cout << fw.GetPosition() << std::endl;
		
		const uint16_t newPos = 4;
		
		std::cout << "Moving filter wheel to position ";
		std::cout << newPos << std::endl;
		
		fw.SetPosition( newPos );
		 
		while( fw.GetStatus() == ApogeeFilterWheel::ACTIVE )
		{
			std::cout << "Filter moving...." << std::endl;
		}

		if( fw.GetStatus() != ApogeeFilterWheel::READY)
		{
			std::string errMsg( "Error invalid filter wheel status" );
			std::runtime_error except( errMsg );
			throw except;
		}

		std::cout << "Current filter wheel position is ";
		std::cout << fw.GetPosition() << std::endl;

		std::cout << "Closing connection to filter wheel" << std::endl;
		fw.Close();

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
