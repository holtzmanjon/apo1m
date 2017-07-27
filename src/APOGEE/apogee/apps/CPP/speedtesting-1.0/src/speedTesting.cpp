#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <stdexcept>
#include <fstream>
#include <tr1/memory>

#include "apogee/Alta.h"
#include "apogee/Ascent.h"
#include "apogee/FindDeviceUsb.h"
#include "apogee/CameraInfo.h" 

#include "ApgTimer.h" 

namespace
{
    ///////////////////////////
    // MAKE	  TOKENS
    std::vector<std::string> MakeTokens(const std::string &str, const std::string &separator)
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

    //------------------------
    //	GET    ITEM    FROM     FIND       STR
    std::string GetItemFromFindStr( const std::string & msg,
                                    const std::string & item )
    {

	    //search the single device input string for the requested item
        std::vector<std::string> params =  MakeTokens( msg, "," );
	    std::vector<std::string>::iterator iter;

	    for(iter = params.begin(); iter != params.end(); ++iter)
	    {
	       if( std::string::npos != (*iter).find( item ) )
	       {
		     std::string result =  MakeTokens( (*iter), "=" ).at(1);
		     
		     return result;
	       }
	    } //for

	    std::string noOp;
	    return noOp;
    }

    ////////////////////////////
    //	GET		ADDRESS
    std::string GetAddress( const std::string & msg )
    {
        return GetItemFromFindStr( msg, "address=" );
    }

    ////////////////////////////
    //	GET		ID
    uint16_t GetID( const std::string & msg )
    {
        std::string str = GetItemFromFindStr( msg, "id=" );
        uint16_t id = 0;
        std::stringstream ss;
        ss << std::hex << std::showbase << str.c_str();
        ss >> id;
      
        return id;
    }

    ////////////////////////////
    //	GET		FRMWR       REV
    uint16_t GetFrmwrRev( const std::string & msg )
    {
        std::string str = GetItemFromFindStr(  msg, "firmwareRev=" );
       
        uint16_t rev = 0;
        std::stringstream ss;
        ss << std::hex << std::showbase << str.c_str();
        ss >> rev;

        return rev;
    }
    
    
    ////////////////////////////
    //	GET		AND     OPEN   CAM
    std::tr1::shared_ptr<ApogeeCam> GetAndOpenCam( const std::string & msg )
    {  
        std::string addr = GetAddress( msg );
        uint16_t id = GetID( msg );
        uint16_t frmwrRev = GetFrmwrRev( msg );
        const CamModel::PlatformType type = CamModel::GetPlatformType( id, false );
        
        std::tr1::shared_ptr<ApogeeCam> cam;

        switch( type )
        {
          case CamModel::ALTAU:
            cam = std::tr1::shared_ptr<ApogeeCam>( new Alta() );
          break;
  
          case CamModel::ASCENT:
            cam = std::tr1::shared_ptr<ApogeeCam>( new Ascent() );
          break;          

          default:
          {
            std::runtime_error except( "invalid camera type" );
            throw except;
          }
          break;
        }

        cam->OpenConnection( "usb", addr, frmwrRev, id );

        return cam;
    }

    void SpeedTesting( std::tr1::shared_ptr<ApogeeCam> & cam )
    {
        uint16_t binning = 1;
        std::cout << "Entering the binning value 1 - " << cam->GetMaxBinCols() << ":" << std::endl;
        std::cin >> binning;

        if( binning < 1 || binning > cam->GetMaxBinCols() )
        {
            std::runtime_error except( "invalid binning value" );
            throw except;
        }

        const uint16_t rows = cam->GetMaxImgRows() / binning;
        cam->SetRoiStartRow( 0 );
        cam->SetRoiBinRow( binning );
        cam->SetRoiNumRows( rows );

        const uint16_t cols = cam->GetMaxImgCols() / binning;
        cam->SetRoiStartCol( 0 );
        cam->SetRoiBinCol( binning );
        cam->SetRoiNumCols( cols );

        //create timer here, so it
		    //doesn't mess up timing
		    ApgTimer theTimer;
        Apg::Status status = Apg::Status_Flushing;

		    cam->SetImageCount( 1 );

        const double EXP_TIME = 0.001;
        std::cout << "Starting " <<  EXP_TIME << " sec exposure" << std::endl;

		    cam->StartExposure(EXP_TIME, true);
   
			  while( Apg::Status_ImageReady !=  status )
			  {
				  status = cam->GetImagingStatus();
        }
      
        std::cout << "START get image" << std::endl;
        theTimer.Start();
        std::vector<uint16_t> data;
        cam->GetImage( data );
        theTimer.Stop();
        std::cout << "COMPLETED get image" << std::endl;

        std::cout << "start row = " << cam->GetRoiStartRow();
        std::cout << "\tstart col = " << cam->GetRoiStartCol() << std::endl;
        std::cout << "# rows = " << cam->GetRoiNumRows();
        std::cout << "\t# cols = " << cam->GetRoiNumCols() << std::endl;
        std::cout << "# binned rows = " << cam->GetRoiBinCol();
        std::cout << "\t# binned cols = " << cam->GetRoiBinCol() << std::endl;
        std::cout << "Number of pixels = " << data.size() << std::endl;
        std::cout << "get image took " << theTimer.GetTimeInMs() << " ms." << std::endl;
        double PixelPerSec = ((double)data.size() / 1000000.0) / theTimer.GetTimeInSec();
        std::cout << "MPixles per sec = " << PixelPerSec << std::endl;
    }
}

////////////////////////////
// MAIN
int main()
{
	try
	{
        //only looking for usb device
        FindDeviceUsb lookUsb;
        std::string msg = lookUsb.Find();

        //create the camera object
        std::tr1::shared_ptr<ApogeeCam> cam = GetAndOpenCam( msg );
        
        cam->Init();
         
        //print out some interesting information
        std::cout << "Cam Info: " << std::endl;
        std::cout << cam->GetInfo().c_str() << std::endl;
        std::cout << "# rows = " << cam->GetMaxImgRows();
        std::cout << "\t# cols = " << cam->GetMaxImgCols() << std::endl;
	      
		    bool runTest = true;
		
		    while( runTest )
		    {
          SpeedTesting( cam );

          std::string answer;
          std::cout << "Continue testing? [y/n]" << std::endl;
          std::cin >> answer;
		
          runTest = ( 0 == answer.compare("y") ? true : false );
        }

        cam->CloseConnection();
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

