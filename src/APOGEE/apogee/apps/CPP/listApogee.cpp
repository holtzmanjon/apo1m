#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include <sys/time.h>


#include "Alta.h"
#include "AltaF.h"
#include "Ascent.h"
#include "Aspen.h"
#include "Quad.h"
#include "CameraInfo.h"
#include "FindDeviceUsb.h"
#include "apgSampleCmn.h"
//#include "Apogee.h"

//
//  Build with  g++ -g -O2 apgSampleCmn.o  listApogee.cpp   -o listApogee 
//		-I/opt/apogee/include/libapogee-3.0/apogee 
//		-I/opt/apogee/libapogee -I. -L/opt/apogee/lib -lapogee -lusb-1.0
//
//
////////////////////////////////////
//
class Timer
{
	public:
		Timer()
		{

		}

		~Timer()
		{

		}

		void Start()
		{
			gettimeofday( &m_start, NULL);
		}

		void Stop()
		{
			gettimeofday( &m_end, NULL);
		}

		long GetTimeInMs()
		{
			long seconds  = m_end.tv_sec  - m_start.tv_sec;
			long useconds = m_end.tv_usec - m_start.tv_usec;
			long mtime = ( (seconds * 1000) + useconds/1000.0) + 0.5;
			return mtime;
		}

		double GetTimeInSec()
		{
			long seconds  = m_end.tv_sec  - m_start.tv_sec;
			long useconds = m_end.tv_usec - m_start.tv_usec;
			double mtime = (double)seconds + ((double)useconds/1000000.0);
			return mtime;
		}

	private:
		struct timeval m_start;
		struct timeval m_end;

};

///////////////////////////
// MAKE	  TOKENS
std::vector<std::string> makeTokens(const std::string &str, const std::string &separator)
{
	std::vector<std::string> returnVector;
	std::string::size_type start = 0;
	std::string::size_type end = 0;

	while( (end = str.find(separator, start)) != std::string::npos)
	{
		returnVector.push_back (str.substr (start, end-start));
		start = end + separator.size();
	}

	returnVector.push_back( str.substr(start) );

	return returnVector;
}

////////////////////////////
//	GET		ADDRESS
std::string GetAddress( const std::string & msg )
{

	//search the find string for an attached camera address
	std::vector<std::string> params = makeTokens( msg, "," );
	std::vector<std::string>::iterator iter;

	for(iter = params.begin(); iter != params.end(); ++iter)
	{
	   if( std::string::npos != (*iter).find("address=") )
	   {
		 std::string DeviceAddr = makeTokens( (*iter), "=" ).at(1);
		 //only looking for one camera
		 return DeviceAddr ;
	   }
	} //for

	std::string noOp;
	return noOp;
}

////////////////////////////
// MAIN
int main()
{
	try
	{
		//only looking for usb device

        	std::string ioInterface("usb");
        	FindDeviceUsb look4cam;
        	std::string msg = look4cam.Find();
		std::cout << msg << std::endl;
                

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
