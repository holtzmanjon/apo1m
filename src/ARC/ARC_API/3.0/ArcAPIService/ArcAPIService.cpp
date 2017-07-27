//#include "CArcAPIServer.h"	// This MUST be first for winsock!
//
//#include <windows.h>
//#include <iostream>
//#include <fstream>
//
//using namespace std;
//using namespace arc;
//
//#define SERVICE_NAME	"ArcAPIService3.0"
//
//SERVICE_STATUS          ServiceStatus;
//SERVICE_STATUS_HANDLE   hStatus;
//CArcAPIServer			cServer;
//
//int InitService();
//void  ServiceMain( int argc, char** argv );
//void  ControlHandler( DWORD request );
//void DoUpdateSvcDesc();
//
//
//
//int WriteToLog(char* str)
//{
//	ofstream log( "C:\\MyServices\\APIServiceLog.txt", ios_base::app );
//	log << str << endl;
//	log.close();
//	return 0;
//}
//
//
//void main()
//{
//	SERVICE_TABLE_ENTRY ServiceTable[ 2 ];
//
//	ServiceTable[ 0 ].lpServiceName = LPWSTR( SERVICE_NAME );
//	ServiceTable[ 0 ].lpServiceProc = ( LPSERVICE_MAIN_FUNCTION )ServiceMain;
//
//	ServiceTable[ 1 ].lpServiceName = NULL;
//	ServiceTable[ 1 ].lpServiceProc = NULL;
//
//	//
//	// Start the control dispatcher thread for our service
//	//
//	StartServiceCtrlDispatcher( ServiceTable );
//}
//
//
////
//// Service initialization
////
//int InitService()
//{
//	int result = WriteToLog( "Monitoring started." );
//	return( result );
//}
//
//
//void ServiceMain( int argc, char** argv )
//{ 
//	int error;
//
//	ServiceStatus.dwServiceType				= SERVICE_WIN32;
//	ServiceStatus.dwCurrentState			= SERVICE_START_PENDING;
//	ServiceStatus.dwControlsAccepted		= ( SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN );
//	ServiceStatus.dwWin32ExitCode			= 0;
//	ServiceStatus.dwServiceSpecificExitCode	= 0;
//	ServiceStatus.dwCheckPoint				= 0;
//	ServiceStatus.dwWaitHint				= 0;
// 
//	hStatus = RegisterServiceCtrlHandler( LPCWSTR( SERVICE_NAME ),
//										 ( LPHANDLER_FUNCTION )ControlHandler );
//
//	//
//	// Registering Control Handler failed
//	//
//	if ( hStatus == ( SERVICE_STATUS_HANDLE )0 )
//	{
//		return;
//	}
//
//	//
//	// Initialize Service
//	//
//	error = InitService();
//
//	//
//	// Initialization failed
//	//
//	if ( error )
//	{
//		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
//		ServiceStatus.dwWin32ExitCode = -1;
//		SetServiceStatus( hStatus, &ServiceStatus );
//		return;
//   }
//
//	//
//	// We report the running status to SCM
//	//
//	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
//	SetServiceStatus( hStatus, &ServiceStatus );
//
//	//
//	// Set the service description
//	//
////	DoUpdateSvcDesc();
//
//	//
//	// The worker loop of a service
//	//
//	while ( ServiceStatus.dwCurrentState == SERVICE_RUNNING )
//	{
//		int  dPort = 5000;
//		bool bLog  = false;
//
//		if ( argc == 2 )
//		{
//			if ( isdigit( argv[ 1 ][ 0 ] ) )
//			{
//				dPort = atoi( argv[ 1 ] );
//			}
//			else
//			{
//				if ( string( argv[ 1 ] ).compare( "true" ) == 0 )
//				{
//					bLog = true;
//				}
//				else if ( string( argv[ 1 ] ).compare( "false" ) == 0 )
//				{
//					bLog = false;
//				}
//			}
//		}
//
//		try
//		{
//			WriteToLog( "Starting server!" );
//
//			cServer.Bind( dPort );
//			cServer.Start( bLog );
//		}
//		catch ( exception &e )
//		{
//			WriteToLog( ( char * )e.what() );
//
//			ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
//			ServiceStatus.dwWin32ExitCode = -1;
//			SetServiceStatus( hStatus, &ServiceStatus );
//
//			break;
//		}
//	}
//
//	return;
//}
//
//
//void ControlHandler( DWORD request )
//{
//   switch( request )
//   {
//      case SERVICE_CONTROL_STOP:
//         WriteToLog( "Monitoring stopped." );
//
//         ServiceStatus.dwWin32ExitCode = 0;
//         ServiceStatus.dwCurrentState = SERVICE_STOPPED;
//         SetServiceStatus( hStatus, &ServiceStatus );
//         return;
//
//      case SERVICE_CONTROL_SHUTDOWN:
//         WriteToLog( "Monitoring stopped." );
//
//         ServiceStatus.dwWin32ExitCode = 0;
//         ServiceStatus.dwCurrentState = SERVICE_STOPPED;
//         SetServiceStatus( hStatus, &ServiceStatus );
//         return;
//
//      default:
//         break;
//    }
//
//    // Report current status
//    SetServiceStatus( hStatus, &ServiceStatus );
//
//    return;
//}
//
//
////
//// Purpose: 
////   Updates the service description to "This is a test description".
////
//// Parameters:
////   None
//// 
//// Return value:
////   None
////
//void DoUpdateSvcDesc()
//{
//	SC_HANDLE schSCManager;
//	SC_HANDLE schService;
//	SERVICE_DESCRIPTION sd;
//	LPTSTR szDesc = TEXT( "ARC API Server 3.0 - Allows Remote Access to PCI/PCIe" );
//
//	//
//	// Get a handle to the SCM database
//	//
//	schSCManager = OpenSCManager( NULL,                     // local computer
//								  NULL,                     // ServicesActive database
//								  SC_MANAGER_ALL_ACCESS );  // full access rights
// 
//	if ( NULL == schSCManager )
//	{
//		WriteToLog( "OpenSCManager failed" );
//		return;
//	}
//
//	//
//	// Get a handle to the service
//	//
//	schService = OpenService( schSCManager,            // SCM database
//							  LPCWSTR( SERVICE_NAME ), // name of service
//							  SERVICE_CHANGE_CONFIG ); // need change config access
//
//	if ( schService == NULL )
//	{
//		WriteToLog( "OpenService failed" );
//		CloseServiceHandle( schSCManager );
//		return;
//	}
//
//	//
//	// Change the service description
//	//
//	sd.lpDescription = szDesc;
//
//	if( !ChangeServiceConfig2( schService,                 // handle to service
//							   SERVICE_CONFIG_DESCRIPTION, // change: description
//							   &sd ) )                     // new description
//	{
//		WriteToLog( "ChangeServiceConfig2 failed" );
//	}
//	else
//	{
//		WriteToLog( "Service description updated successfully." );
//	}
//
//	CloseServiceHandle( schService );
//	CloseServiceHandle( schSCManager );
//}



#include <iostream>
#include <cstdlib>
#include "CArcAPIServer.h"

using namespace std;
using namespace arc;


int main( int argc, char **argv )
{
	CArcAPIServer cServer;

	int  dPort = 5000;
	bool bLog  = false;

	if ( argc == 2 )
	{
		if ( isdigit( argv[ 1 ][ 0 ] ) )
		{
			dPort = atoi( argv[ 1 ] );
		}
		else
		{
			if ( string( argv[ 1 ] ).compare( "true" ) == 0 )
			{
				bLog = true;
			}
			else if ( string( argv[ 1 ] ).compare( "false" ) == 0 )
			{
				bLog = false;
			}
		}
	}

	try
	{
		cServer.Bind( dPort );
		cServer.Start( bLog );
	}
	catch ( exception &e )
	{
		cerr << endl << e.what() << endl;
	}

	return 0;
}
