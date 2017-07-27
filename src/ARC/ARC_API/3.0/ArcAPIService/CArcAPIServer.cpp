#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "CArcAPIServer.h"
#include "CArcPCIe.h"
#include "CArcPCI.h"
#include "ArcOSDefs.h"
#include "ArcDefs.h"
#include "CArcTools.h"
#include "ArcCltSrvStr.h"

#ifdef WIN32
#include <process.h>
#else
#include <pthread.h>
#include <sys/stat.h>
#endif

using namespace std;
using namespace arc;


#define SERVER_VERSION		3.0

#define ASSERT_CLASS_PTR( funcStr, ptrStr, ptr ) \
				if ( m_pCArcDev == NULL ) \
				{ \
					throw runtime_error( "Exception: " + \
					string( funcStr ) + \
					" - Invalid class handle ( " + \
					string( ptrStr ) + " is NULL )!" ); \
				}


CArcAPIServer::CArcAPIServer()
{
#ifdef WIN32
	WSADATA wsaData;

	//
	// Initialize Winsock
	//
	int dError = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );

	if ( dError )
	{
		ThrowException( "CArcAPIServer",
						"WSAStartup returned error: " +
						 GetSystemMessage( GetError() ) );
	}

#endif

	m_tBindSocket	= INVALID_SOCKET;
	m_tAcceptSocket	= INVALID_SOCKET;
	m_pCDeinterlace = NULL;
	m_pCArcDev		= new CArcPCIe();
	m_pCFitsFile	= NULL;
	m_pCTiffFile	= NULL;
	m_pCImage		= NULL;

	m_bLog = false;
}

CArcAPIServer::~CArcAPIServer()
{
	if ( m_pCDeinterlace != NULL )
	{
		delete m_pCDeinterlace;
	}

	if ( m_pCFitsFile != NULL )
	{
		delete m_pCFitsFile;
	}

	if ( m_pCTiffFile != NULL )
	{
		delete m_pCTiffFile;
	}

	if ( m_pCImage != NULL )
	{
		delete m_pCImage;
	}

	CloseAllConnections();

#ifdef WIN32
	WSACleanup();
#endif
}

void CArcAPIServer::Start( bool bLog )
{
	m_bLog = bLog;

	int nret = listen( m_tBindSocket, 10 );

	if ( nret == SOCKET_ERROR )
	{
		CloseAllConnections();

		ThrowException( "Start",
						"listen() returned error: " +
						 GetSystemMessage( GetError() ) );
	}

	SOCKADDR_IN tInSockAddr;
	socklen_t dInAddrSize = sizeof( SOCKADDR );

	while( true )
	{
		clog << "Waiting for a connection" << endl;

		m_tAcceptSocket = accept( m_tBindSocket,
								  ( SOCKADDR* )&tInSockAddr,
								  &dInAddrSize );

		if ( m_tAcceptSocket == INVALID_SOCKET )
		{
			ThrowException( "Start",
							"accept() returned error: " +
							 GetSystemMessage( GetError() ) );
		}

		clog << "Received connection from "
			 << inet_ntoa( tInSockAddr.sin_addr )
			 << endl;
#ifdef WIN32
		CreateThread( 0, 0, ( LPTHREAD_START_ROUTINE )&threadProc, ( void * )this , 0, 0 );
#else
		pthread_create( &m_tThread, NULL, &CArcAPIServer::threadProc, ( void * )this );
#endif

	}
}

void CArcAPIServer::CloseAllConnections()
{
#ifdef WIN32
	closesocket( m_tAcceptSocket );
	closesocket( m_tBindSocket );
#else
	close( m_tAcceptSocket );
	close( m_tBindSocket );
#endif

	m_tBindSocket	= INVALID_SOCKET;
	m_tAcceptSocket	= INVALID_SOCKET;
}

void CArcAPIServer::CloseConnection( SOCKET& tSocket )
{
#ifdef WIN32
	closesocket( tSocket );
#else
	close( tSocket );
#endif

	tSocket = INVALID_SOCKET;
}

// /////////////////////////////////////////////////////////////////////////////
// /  Server Bind Method                                                       /
// /////////////////////////////////////////////////////////////////////////////
void CArcAPIServer::Bind( unsigned short usPort )
{
	// Create the socket
	m_tBindSocket = socket( AF_INET,			// Go over TCP/IP
							SOCK_STREAM,		// This is a stream-oriented socket
							IPPROTO_TCP );		// Use TCP rather than UDP

	if ( m_tBindSocket == INVALID_SOCKET )
	{
		CloseAllConnections();

		ThrowException( "Bind",
						"socket() returned error: " +
						 GetSystemMessage( GetError() ) );
	}

	// Fill a SOCKADDR_IN struct with address information
	SOCKADDR_IN serverInfo;

	ZeroMem( &( serverInfo.sin_zero ), 8 );

	serverInfo.sin_family      = AF_INET;
	serverInfo.sin_addr.s_addr = INADDR_ANY ;
	serverInfo.sin_port        = htons( usPort );		// Change to network-byte order and
														// insert into port field

	// Bind the server
	int nret = bind( m_tBindSocket,
					 ( LPSOCKADDR )&serverInfo,
					 sizeof( SOCKADDR ) );

	if ( nret == SOCKET_ERROR )
	{
		CloseAllConnections();

		ThrowException( "Bind",
						"bind() returned error: " +
						 GetSystemMessage( GetError() ) );
	}
}

void CArcAPIServer::ThrowException( string sMethodName, string sMsg )
{
	ostringstream oss;

	oss << "( CArcAPIServer::"
		<< ( sMethodName.empty() ? "???" : sMethodName )
		<< "() ): "
		<< sMsg
		<< ends;

	throw runtime_error( oss.str() );
}

void CArcAPIServer::SendException( const char* szMsg )
{
	ostringstream oss;
	oss << ERROR_STRING << ": " << szMsg << endl << ends;
	Send( oss.str().c_str() );
}

void CArcAPIServer::SendOK()
{
	ostringstream oss;
	oss << API_OK_STRING << endl << ends;
	Send( oss.str().c_str() );
}

// +----------------------------------------------------------------------------
// |  GetSystemMessage
// +----------------------------------------------------------------------------
// |  Internal method that converts a system error code into a string. Uses
// |  ::GetLastError on windows and strerror on linux/solaris.
// |
// |  <IN>  -> dCode - The system dependent error code, ::GetLastError,
// |                   WSAGetLastError, or errno.
// |  <OUT> -> The error code message
// +----------------------------------------------------------------------------
string CArcAPIServer::GetSystemMessage( int dCode )
{
	ostringstream oss;

#ifdef WIN32

	char szBuffer[ 256 ];

	FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM |
				    FORMAT_MESSAGE_IGNORE_INSERTS,
				    NULL,
				    ( DWORD )dCode,
				    MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
					szBuffer,
				    256,
				    NULL );

	oss << "[ " << dCode << " ] - " << szBuffer << ends;

#else

	if ( dCode != -1 )
	{
		oss << "( errno: " << dCode << " ) - " << strerror( dCode );
	}

#endif

	return oss.str();
}

// +----------------------------------------------------------------------------
// |  GetError
// +----------------------------------------------------------------------------
// |  Returns the last error that occured. Uses ::WSAGetLastError on windows
// |  and errno on linux/solaris.
// +----------------------------------------------------------------------------
int CArcAPIServer::GetError()
{
#ifdef WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

// +----------------------------------------------------------------------------
// |  ZeroMem
// +----------------------------------------------------------------------------
// |  Zero out the specified buffer. Uses ::ZeroMemory on windows and memset
// |  on linux/solaris.
// |
// |  <IN> -> pBuf  - The buffer to clear/set to zero.
// |  <IN> -> dSize - The size of the buffer in bytes.
// +----------------------------------------------------------------------------
void CArcAPIServer::ZeroMem( void* pBuf, int dSize )
{
#ifdef WIN32
	ZeroMemory( pBuf, dSize );
#else
	memset( pBuf, 0, dSize );
#endif
}

// /////////////////////////////////////////////////////////////////////////////
// /  Starts the Main Control Thread                                           /
// /////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
unsigned __stdcall CArcAPIServer::threadProc( void* a_param )
{
    CArcAPIServer* pThread = static_cast<CArcAPIServer*>( a_param );
    return unsigned( pThread->Run() );
}
#else
void* CArcAPIServer::threadProc( void* a_param )
{
    CArcAPIServer* pThread = static_cast<CArcAPIServer*>( a_param );
    return ( void * )pThread->Run();
}
#endif

// /////////////////////////////////////////////////////////////////////////////
// /  Main Control Thread                                                      /
// /////////////////////////////////////////////////////////////////////////////
int CArcAPIServer::Run()
{
	while ( true )
	{
		try
		{
			string str = Recv();

			if ( m_bLog )
			{
				clog << "Received: '" << str << "'" << endl;
			}

			if ( str.find( CLASS_CArcDevice ) != string::npos )
			{
				HandleCArcDevice( str );
			}

			else if ( str.find( CLASS_CDeinterlace ) != string::npos )
			{
				HandleCDeinterlace( str );
			}

			else if ( str.find( CLASS_CFitsFile ) != string::npos )
			{
				HandleCFitsFile( str );
			}

			else if ( str.find( CLASS_CTiffFile ) != string::npos )
			{
				HandleCTiffFile( str );
			}

			else if ( str.find( CLASS_CImage ) != string::npos )
			{
				HandleCImage( str );
			}

			else if ( str.find( CLASS_CArcAPIServer ) != string::npos )
			{
				HandleArcAPIServer( str );
			}
		}
		catch ( exception &e )
		{
			clog << e.what() << endl;

			if ( string( e.what() ).empty() )
			{
				clog << "Possibly invalid parameters sent!" << endl;
			}

			CloseConnection( m_tAcceptSocket );
			break;
		}
		catch ( ... )
		{
			clog << "System Exception Caught!" << endl;

			CloseConnection( m_tAcceptSocket );
			break;
		}
	}

	//
	//  WSACloseEvent( hEvent );
	//
	if ( m_pCDeinterlace != NULL )
	{
		delete m_pCDeinterlace;
		m_pCDeinterlace = NULL;
	}

	if ( m_pCFitsFile != NULL )
	{
		delete m_pCFitsFile;
		m_pCFitsFile = NULL;
	}

	if ( m_pCTiffFile != NULL )
	{
		delete m_pCTiffFile;
		m_pCTiffFile = NULL;
	}

	if ( m_pCImage != NULL )
	{
		delete m_pCImage;
		m_pCImage = NULL;
	}

	clog << "______________________________________________" << endl
		 << " Current Server Thread Terminated!" << endl
		 << "----------------------------------------------" << endl;

	return 0;
}

// /////////////////////////////////////////////////////////////////////////////
// /  ArcAPIServer Handler                                                     /
// /////////////////////////////////////////////////////////////////////////////
void CArcAPIServer::HandleArcAPIServer( string sInData )
{
	string sTemp;

	const string sCmd = StripCmd( sInData );

	try
	{
		// +---------------------------------------------------+
		// |  GetDirListing                                    |
		// +---------------------------------------------------+
		if ( sCmd.compare( "GetDirListing" ) == 0 )
		{
			//string sTargetDir;

			//istringstream iss( sInData.c_str() );
			//iss >> sTemp >> sTargetDir;

			//SendOK();

			//vector<string> vecDirList;
			//CArcUtils::GetDirListing( sTargetDir, vecDirList );

			//string sList;
			//for ( string::size_type i=0; i<vecDirList.size(); i++ )
			//{
			//	sList.append( vecDirList.at( i ) );

			//	if ( i < ( vecDirList.size() - 1 ) )
			//	{
			//		sList.append( "|" );
			//	}
			//}

			////Send( int( sList.length() ) );

			////if ( sList.length() > 0 )
			////{
			////	if ( ( sTemp = Recv() ).find( "OK" ) == string::npos )
			////	{
			////		ThrowException( "GetDirListing",
			////						"Invalid message received from client: "
			////						+ sTemp + " ( Expected: \"OK\" )" );
			////	}

			//	Send( sList.c_str() );
			//	//Send( sList.c_str(), sList.length() );
			////}
		}

		// +---------------------------------------------------+
		// |  Find                                             |
		// +---------------------------------------------------+
		else if ( sCmd.compare( "Find" ) == 0 )
		{
			Send( "Found SERVER!" );
		}

		// +---------------------------------------------------+
		// |  LogMsgOnServer                                   |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_LogMsgOnServer ) == 0 )
		{
			string sMsg;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> sMsg;

			 while ( iss.good() )
			 {
				 iss >> sTemp;

				 sMsg = sMsg.append( " " ).append( sTemp );
			 }

			SendOK();
		}

		// +---------------------------------------------------+
		// |  IsServerLogging                                  |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_IsServerLogging ) == 0 )
		{
			Send( static_cast<int>( m_bLog ) );
		}

		// +---------------------------------------------------+
		// |  EnableServerLog                                  |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_EnableServerLog ) == 0 )
		{
			int dLog = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dLog;

			m_bLog = ( dLog != 0 );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  ServerVersion                                    |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetServerVersion ) == 0 )
		{
			Send( SERVER_VERSION );
		}
	}
	catch ( exception &e )
	{
		clog << e.what() << endl;
		SendException( e.what() );
	}
	catch ( ... )
	{
		clog << "System Exception Caught!" << endl;
		SendException( "System Exception Caught!" );
	}
}

// /////////////////////////////////////////////////////////////////////////////
// /  CArcDevice Handler                                                       /
// /////////////////////////////////////////////////////////////////////////////
void CArcAPIServer::HandleCArcDevice( string sInData )
{
	string sTemp;

	const string sCmd = StripCmd( sInData );

	try
	{
		// +---------------------------------------------------+
		// |  GetDeviceList                                    |
		// +---------------------------------------------------+
		if ( sCmd.compare( METHOD_GetDeviceList ) == 0 )
		{
			char**		pszPCIDevList	= NULL;
			char**		pszPCIeDevList	= NULL;
			CArcPCIe	cArcPCIe;
			CArcPCI		cArcPCI;
			string		sRecv;

			try
			{
				//
				//  Get the PCI device list
				// -------------------------------------------------
				CArcPCI::FindDevices();
				
				//
				// Get the PCI device string list. Ignore NULL list,
				// will handle down below.
				//
				pszPCIDevList = ( char ** )CArcPCI::GetDeviceStringList();
			}
			catch ( ... ) {}
	
			try
			{
				//
				//  Get the PCIe device list
				// -------------------------------------------------
				CArcPCIe::FindDevices();
				
				//
				// Get the PCIe device string list. Ignore NULL list,
				// will handle down below.
				//
				pszPCIeDevList = ( char ** )CArcPCIe::GetDeviceStringList();
			}
			catch ( ... ) {}

			try
			{
				if ( ( pszPCIDevList == NULL ) && ( pszPCIeDevList == NULL ) )
				{
					ThrowException( METHOD_GetDeviceList,
									"Failed to retrieve device string list!" );
				}

				//
				// Get the available device count
				//
				int dDeviceCount = CArcPCI::DeviceCount() + CArcPCIe::DeviceCount();

				if ( dDeviceCount > 0 )
				{
					Send( dDeviceCount );

					for ( int i=0; i<CArcPCIe::DeviceCount(); i++ )
					{
						sRecv = Recv();

						if ( sRecv.find( "OK" ) != string::npos )
						{
							Send( "%s", pszPCIeDevList[ i ] );
						}
						else
						{
							ThrowException( METHOD_GetDeviceList,
											"Invalid client message: "
											+ sRecv );
						}
					}

					for ( int i=0; i<CArcPCI::DeviceCount(); i++ )
					{
						sRecv = Recv();

						if ( sRecv.find( "OK" ) != string::npos )
						{
							Send( "%s", pszPCIDevList[ i ] );
						}
						else
						{
							ThrowException( METHOD_GetDeviceList,
											"Invalid client message: "
											+ sRecv );
						}
					}
				}
				else
				{
					cArcPCIe.FreeDeviceStringList();
					cArcPCI.FreeDeviceStringList();
						
					ThrowException( METHOD_GetDeviceList,
									"Failed create array for device list!" );
				}
					
				cArcPCIe.FreeDeviceStringList();
				cArcPCI.FreeDeviceStringList();
			}
			catch ( std::exception &e )
			{
				cArcPCIe.FreeDeviceStringList();
				cArcPCI.FreeDeviceStringList();

				ThrowException( METHOD_GetDeviceList, e.what() );
			}
		}

		// +---------------------------------------------------+
		// |  UseDevices                                       |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_UseDevices ) == 0 )
		{
			int dNameCount = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp >> dNameCount;

			char** pszDeviceNames = new char*[ dNameCount ];

			if ( pszDeviceNames == NULL )
			{
				ThrowException(	METHOD_UseDevices,
								"Failed to allocate space for device names" );
			}

			for ( int i=0; i<dNameCount; i++ )
			{
				pszDeviceNames[ i ] = new char[ 50 ];

				iss >> pszDeviceNames[ i ];
			}

			string sDevName = string( m_pCArcDev->ToString() );

			if ( sDevName.find( "PCIe" ) != string::npos )
			{
				CArcPCIe::UseDevices( ( const char ** )pszDeviceNames,
									   dNameCount );
			}
			else
			{
				CArcPCI::UseDevices( ( const char ** )pszDeviceNames,
									  dNameCount );
			}

			delete [] pszDeviceNames;

			SendOK();
		}

		// +---------------------------------------------------+
		// |  IsOpen                                           |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_IsOpen ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_IsOpen,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				static_cast<int>(
						m_pCArcDev->IsOpen() ) );
		}

		// +---------------------------------------------------+
		// |  Open                                             |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_Open ) == 0 )
		{
			string sDeviceName;
			int dBytes = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp >> sDeviceName;

			if ( iss.good() )
			{
				iss >> dBytes;
			}

			FixMultiStringData( sDeviceName );

			CArcTools::CTokenizer cTokenizer;
			cTokenizer.Victim( sDeviceName );

			string sDevice = cTokenizer.Next();

			if ( sDevice.compare( "PCI" ) == 0 )
			{
				if ( m_pCArcDev != NULL )
				{
					delete m_pCArcDev;
					m_pCArcDev = NULL;
				}
				
				CArcPCI::FindDevices();
				
				m_pCArcDev = new CArcPCI();
			}
			else if ( sDevice.compare( "PCIe" ) == 0 )
			{
				if ( m_pCArcDev != NULL )
				{
					delete m_pCArcDev;
					m_pCArcDev = NULL;
				}
				
				CArcPCIe::FindDevices();
				
				m_pCArcDev = new CArcPCIe();
			}
			else
			{
				ThrowException( METHOD_Open,
								"Invalid device: " + sDevice );
			}
			
			cTokenizer.Next();		// Dump "Device" Text
			
			int dDevNum = atoi( cTokenizer.Next().c_str() );

			if ( dBytes > 0 )
			{
				m_pCArcDev->Open( dDevNum, dBytes );
			}
			else
			{
				m_pCArcDev->Open( dDevNum );
			}

			SendOK();
		}

		// +---------------------------------------------------+
		// |  Close                                            |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_Close ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_Close,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->Close();

			SendOK();
		}

		// +---------------------------------------------------+
		// |  Reset                                            |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_Reset ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_Reset,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->Reset();

			SendOK();
		}

		// +---------------------------------------------------+
		// |  MapCommonBuffer                                  |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_MapCommonBuffer ) == 0 )
		{
			int dBytes = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dBytes;

			ASSERT_CLASS_PTR( METHOD_MapCommonBuffer,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->MapCommonBuffer( dBytes );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  UnMapCommonBuffer                                |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_UnMapCommonBuffer ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_UnMapCommonBuffer,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->UnMapCommonBuffer();

			SendOK();
		}

		// +---------------------------------------------------+
		// |  ReMapCommonBuffer                                |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_ReMapCommonBuffer ) == 0 )
		{
			int dBytes = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dBytes;

			ASSERT_CLASS_PTR( METHOD_ReMapCommonBuffer,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->ReMapCommonBuffer( dBytes );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  GetCommonBufferProperties                        |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetCommonBufferProperties ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetCommonBufferProperties,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				static_cast<int>(
						m_pCArcDev->GetCommonBufferProperties() ) );
		}

		// +---------------------------------------------------+
		// |  FillCommonBuffer                                 |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_FillCommonBuffer ) == 0 )
		{
			unsigned short u16Value = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp >> u16Value;

			ASSERT_CLASS_PTR( METHOD_FillCommonBuffer,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->FillCommonBuffer( u16Value );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  CommonBufferVA                                   |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_CommonBufferVA ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_CommonBufferVA,
							  "CArcDevice",
							  m_pCArcDev )

			Send( ULONG_PTR( m_pCArcDev->CommonBufferVA() ) );
		}

		// +---------------------------------------------------+
		// |  CommonBufferPA                                   |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_CommonBufferPA ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_CommonBufferPA,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->CommonBufferPA() );
		}

		// +---------------------------------------------------+
		// |  CommonBufferSize                                 |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_CommonBufferSize ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_CommonBufferSize,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->CommonBufferSize() );
		}

		// +---------------------------------------------------+
		// |  CommonBufferPixels                               |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_CommonBufferPixels ) == 0 )
		{
			int dSize = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dSize;

			ASSERT_CLASS_PTR( METHOD_CommonBufferPixels,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->CommonBufferVA(), dSize );
		}

		// +---------------------------------------------------+
		// |  GetId                                            |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetId ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetId,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->GetId() );
		}

		// +---------------------------------------------------+
		// |  GetStatus                                        |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetStatus ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetStatus,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->GetStatus() );
		}

		// +---------------------------------------------------+
		// |  ClearStatus                                      |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_ClearStatus ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_ClearStatus,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->ClearStatus();

			SendOK();
		}

		// +---------------------------------------------------+
		// |  Set2xFOTransmitter                               |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_Set2xFOTransmitter ) == 0 )
		{
			bool bOnOff = false;

			istringstream iss( sInData.c_str() );

			iss >> sTemp >> bOnOff;

			ASSERT_CLASS_PTR( METHOD_Set2xFOTransmitter,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->Set2xFOTransmitter( bOnOff );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  LoadDeviceFile                                   |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_LoadDeviceFile ) == 0 )
		{
			char szFile[ 200 ];

			istringstream iss( sInData.c_str() );

			iss >> sTemp >> szFile;

			ASSERT_CLASS_PTR( METHOD_LoadDeviceFile,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->LoadDeviceFile( szFile );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  GetCfgSpByte                                     |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetCfgSpByte ) == 0 )
		{
			int dOffset = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dOffset;

			ASSERT_CLASS_PTR( METHOD_GetCfgSpByte,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				dynamic_cast<CArcPCIBase *>
					( m_pCArcDev )->GetCfgSpByte( dOffset ) );
		}

		// +---------------------------------------------------+
		// |  GetCfgSpWord                                     |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetCfgSpWord ) == 0 )
		{
			int dOffset = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dOffset;

			ASSERT_CLASS_PTR( METHOD_GetCfgSpWord,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				dynamic_cast<CArcPCIBase *>
					( m_pCArcDev )->GetCfgSpWord( dOffset ) );
		}

		// +---------------------------------------------------+
		// |  GetCfgSpDWord                                    |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetCfgSpDWord ) == 0 )
		{
			int dOffset = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dOffset;

			ASSERT_CLASS_PTR( METHOD_GetCfgSpDWord,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				dynamic_cast<CArcPCIBase *>
					( m_pCArcDev )->GetCfgSpDWord( dOffset ) );
		}

		// +---------------------------------------------------+
		// |  SetCfgSpByte                                     |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SetCfgSpByte ) == 0 )
		{
			int dOffset = 0;
			int dValue  = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dOffset >> dValue;

			ASSERT_CLASS_PTR( METHOD_SetCfgSpByte,
							  "CArcDevice",
							  m_pCArcDev )

			dynamic_cast<CArcPCIBase *>
				( m_pCArcDev )->SetCfgSpByte( dOffset, dValue );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  SetCfgSpWord                                     |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SetCfgSpWord ) == 0 )
		{
			int dOffset = 0;
			int dValue  = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dOffset >> dValue;

			ASSERT_CLASS_PTR( METHOD_SetCfgSpWord,
							  "CArcDevice",
							  m_pCArcDev )

			dynamic_cast<CArcPCIBase *>
				( m_pCArcDev )->SetCfgSpWord( dOffset, dValue );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  SetCfgSpDWord                                    |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SetCfgSpDWord ) == 0 )
		{
			int dOffset = 0;
			int dValue  = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dOffset >> dValue;

			ASSERT_CLASS_PTR( METHOD_SetCfgSpDWord,
							  "CArcDevice",
							  m_pCArcDev )

			dynamic_cast<CArcPCIBase *>
				( m_pCArcDev )->SetCfgSpDWord( dOffset, dValue );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  Command                                          |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_Command ) == 0 )
		{
			int dBrdId = 0, dCmd = 0, dArg[ 4 ] = { -1, -1, -1, -1 };

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dBrdId
				>> dCmd;

			for ( int i=0, j=0; i<4; i++ )
			{
				if ( iss.good() )
				{
					iss >> dArg[ j ];
					j++;
				}
			}

			ASSERT_CLASS_PTR( METHOD_Command,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->Command( dBrdId,
									   dCmd,
									   dArg[ 0 ],
									   dArg[ 1 ],
									   dArg[ 2 ],
									   dArg[ 3 ] ) );
		}

		// +---------------------------------------------------+
		// |  GetControllerId                                  |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetControllerId ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetControllerId,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->GetControllerId() );
		}

		// +---------------------------------------------------+
		// |  ResetController                                  |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_ResetController ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_ResetController,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->ResetController();

			SendOK();
		}

		// +---------------------------------------------------+
		// |  IsControllerConnected                            |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_IsControllerConnected ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_IsControllerConnected,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				static_cast<int>(
					m_pCArcDev->IsControllerConnected() ) );
		}

		// +---------------------------------------------------+
		// |  SetupController                                  |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SetupController ) == 0 )
		{
			string sTimFile = "", sUtilFile = "", sPciFile = "";
			bool bReset = false, bTdl = false, bPower = false;
			int dRows = 0, dCols = 0;

			bool bTimFile = false, bUtlFile = false, bPciFile = false;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> bReset
				>> bTdl
				>> bPower
				>> dRows
				>> dCols
				>> bTimFile
				>> bUtlFile
				>> bPciFile;

			if ( bTimFile )
			{
				sTimFile = RecvDspFile( "TIM" );
			}

			if ( bUtlFile )
			{
				sUtilFile = RecvDspFile( "UTIL" );
			}

			if ( bPciFile )
			{
				sPciFile = RecvDspFile( "PCI" );

			}

			Send( "DONE" );

			ASSERT_CLASS_PTR( METHOD_SetupController,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->SetupController( bReset,
										 bTdl,
										 bPower,
										 dRows,
										 dCols,
										 sTimFile.c_str(),
										 ( !sUtilFile.empty() ? sUtilFile.c_str() : ( const char * )NULL ),
										 ( !sPciFile.empty()  ? sPciFile.c_str()  : ( const char * )NULL ) );
#ifdef WIN32
			if ( bTimFile ) { DeleteFileA( sTimFile.c_str() );  }
			if ( bUtlFile ) { DeleteFileA( sUtilFile.c_str() ); }
			if ( bPciFile ) { DeleteFileA( sPciFile.c_str() );  }
#else
			if ( bTimFile ) { remove( sTimFile.c_str() );  }
			if ( bUtlFile ) { remove( sUtilFile.c_str() ); }
			if ( bPciFile ) { remove( sPciFile.c_str() );  }
#endif
			SendOK();
		}

		// +---------------------------------------------------+
		// |  LoadControllerFile                               |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_LoadControllerFile ) == 0 )
		{
			bool bValidate = false;
			int  dLength   = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dLength
				>> bValidate;

			if ( dLength > 0 )
			{
				SendOK();
			}
			else
			{
				ostringstream oss;

				oss << "Invalid file length: "
					<< dLength << " bytes";

				ThrowException( METHOD_LoadControllerFile,
								oss.str() );
			}

			string sFileName = GetTempFileName();

			RecvFile( sFileName, dLength );

			ASSERT_CLASS_PTR( METHOD_LoadControllerFile,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->LoadControllerFile( sFileName.c_str(),
											bValidate );
#ifdef WIN32
			DeleteFileA( sFileName.c_str() );
#else
			remove( sFileName.c_str() );
#endif

			SendOK();
		}

		// +---------------------------------------------------+
		// |  SetImageSize                                     |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SetImageSize ) == 0 )
		{
			int dRows = 0;
			int dCols = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dRows
				>> dCols;

			ASSERT_CLASS_PTR( METHOD_SetImageSize,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->SetImageSize( dRows, dCols );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  GetImageRows                                     |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetImageRows ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetImageRows,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->GetImageRows() );
		}

		// +---------------------------------------------------+
		// |  GetImageCols                                     |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetImageCols ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetImageCols,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->GetImageCols() );
		}

		// +---------------------------------------------------+
		// |  GetCCParams                                      |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetCCParams ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetCCParams,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->GetCCParams() );
		}

		// +---------------------------------------------------+
		// |  IsCCParamSupported                               |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_IsCCParamSupported ) == 0 )
		{
			int dParam = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dParam;

			ASSERT_CLASS_PTR( METHOD_IsCCParamSupported,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				static_cast<int>(
					m_pCArcDev->IsCCParamSupported( dParam ) ) );
		}

		// +---------------------------------------------------+
		// |  IsCCD                                            |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_IsCCD ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_IsCCD,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				static_cast<int>( m_pCArcDev->IsCCD() ) );
		}

		// +---------------------------------------------------+
		// |  IsBinningSet                                     |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_IsBinningSet ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_IsBinningSet,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				static_cast<int>(
					m_pCArcDev->IsBinningSet() ) );
		}

		// +---------------------------------------------------+
		// |  SetBinning                                       |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SetBinning ) == 0 )
		{
			int dRows      = 0;
			int dCols      = 0;
			int dRowFactor = 0;
			int dColFactor = 0;
			int dBinRows   = 0;
			int dBinCols   = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dRows
				>> dCols
				>> dRowFactor
				>> dColFactor;

			ASSERT_CLASS_PTR( METHOD_SetBinning,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->SetBinning( dRows,
									  dCols,
									  dRowFactor,
									  dColFactor,
									  &dBinRows,
									  &dBinCols );

			Send( "%d %d", dBinRows, dBinCols );
		}

		// +---------------------------------------------------+
		// |  UnSetBinning                                     |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_UnSetBinning ) == 0 )
		{
			int dRows = 0;
			int dCols = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dRows
				>> dCols;

			ASSERT_CLASS_PTR( METHOD_UnSetBinning,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->UnSetBinning( dRows, dCols );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  SetSubArray                                      |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SetSubArray ) == 0 )
		{
			int dRow        = 0;
			int dCol        = 0;
			int dOldRows    = 0;
			int dOldCols    = 0;
			int dSubRows    = 0;
			int dSubCols    = 0;
			int dBiasOffset = 0;
			int dBiasWidth  = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dOldRows
				>> dOldCols
				>> dRow
				>> dCol
				>> dSubRows
				>> dSubCols
				>> dBiasOffset
				>> dBiasWidth;

			ASSERT_CLASS_PTR( METHOD_SetSubArray,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->SetSubArray( dOldRows,
									   dOldCols,
									   dRow,
									   dCol,
									   dSubRows,
									   dSubCols,
									   dBiasOffset,
									   dBiasWidth );

			Send( "%d %d", dOldRows, dOldCols );
		}

		// +---------------------------------------------------+
		// |  UnSetSubArray                                    |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_UnSetSubArray ) == 0 )
		{
			int dRows = 0;
			int dCols = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dRows
				>> dCols;

			ASSERT_CLASS_PTR( METHOD_UnSetSubArray,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->UnSetSubArray( dRows, dCols );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  IsSyntheticImageMode                             |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_IsSyntheticImageMode ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_IsSyntheticImageMode,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				static_cast<int>(
					m_pCArcDev->IsSyntheticImageMode() ) );
		}

		// +---------------------------------------------------+
		// |  SetSyntheticImageMode                            |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SetSyntheticImageMode ) == 0 )
		{
			bool bMode = false;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> bMode;

			ASSERT_CLASS_PTR( METHOD_SetSyntheticImageMode,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->SetSyntheticImageMode( bMode );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  SetOpenShutter                                   |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SetOpenShutter ) == 0 )
		{
			bool bShouldOpen = false;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> bShouldOpen;

			ASSERT_CLASS_PTR( METHOD_SetOpenShutter,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->SetOpenShutter( bShouldOpen );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  Expose                                           |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_Expose ) == 0 )
		{
			float fExpTime  = 0.f;
			int   dRows     = 0;
			int   dCols     = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> fExpTime
				>> dRows
				>> dCols;

			ASSERT_CLASS_PTR( METHOD_Expose,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->Expose( fExpTime,
								dRows,
								dCols );
			SendOK();
		}

		// +---------------------------------------------------+
		// |  StopExposure                                     |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_StopExposure ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_StopExposure,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->StopExposure();

			SendOK();
		}

		// +---------------------------------------------------+
		// |  Continuous                                       |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_Continuous ) == 0 )
		{
			float fExpTime     = 0.f;
			int   dNumOfFrames = 0;
			int   dRows        = 0;
			int   dCols        = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dRows
				>> dCols
				>> dNumOfFrames
				>> fExpTime;

			ASSERT_CLASS_PTR( METHOD_Continuous,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->Continuous( dRows,
									dCols,
									dNumOfFrames,
									fExpTime );
			SendOK();
		}

		// +---------------------------------------------------+
		// |  StopContinuous                                   |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_StopContinuous ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_StopContinuous,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->StopContinuous();

			SendOK();
		}

		// +---------------------------------------------------+
		// |  IsReadout                                        |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_IsReadout ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_IsReadout,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				static_cast<int>(
						m_pCArcDev->IsReadout() ) );
		}

		// +---------------------------------------------------+
		// |  GetPixelCount                                    |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetPixelCount ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetPixelCount,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->GetPixelCount() );
		}

		// +---------------------------------------------------+
		// |  GetCRPixelCount                                  |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetCRPixelCount ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetCRPixelCount,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->GetCRPixelCount() );
		}

		// +---------------------------------------------------+
		// |  GetFrameCount                                    |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetFrameCount ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetFrameCount,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->GetFrameCount() );
		}

		// +---------------------------------------------------+
		// |  GetArrayTemperature                              |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetArrayTemperature ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetArrayTemperature,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				static_cast<double>(
					m_pCArcDev->GetArrayTemperature() ) );
		}

		// +---------------------------------------------------+
		// |  GetArrayTemperatureDN                            |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetArrayTemperatureDN ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetArrayTemperatureDN,
							  "CArcDevice",
							  m_pCArcDev )

			Send(
				static_cast<double>(
					m_pCArcDev->GetArrayTemperatureDN() ) );
		}

		// +---------------------------------------------------+
		// |  SetArrayTemperature                              |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SetArrayTemperature ) == 0 )
		{
			double gTempVal = 0.0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> gTempVal;

			ASSERT_CLASS_PTR( METHOD_SetArrayTemperature,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->SetArrayTemperature( gTempVal );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  LoadTemperatureCtrlData                          |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_LoadTemperatureCtrlData ) == 0 )
		{
			string sFilename = "";

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> sFilename;

			ASSERT_CLASS_PTR( METHOD_LoadTemperatureCtrlData,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->LoadTemperatureCtrlData( sFilename.c_str() );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  SaveTemperatureCtrlData                          |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SaveTemperatureCtrlData ) == 0 )
		{
			string sFilename = "";

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> sFilename;

			ASSERT_CLASS_PTR( METHOD_SaveTemperatureCtrlData,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->SaveTemperatureCtrlData( sFilename.c_str() );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  GetNextLoggedCmd                                 |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetNextLoggedCmd ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetNextLoggedCmd,
							  "CArcDevice",
							  m_pCArcDev )

			const char* szMsg = m_pCArcDev->GetNextLoggedCmd();

			if ( szMsg != NULL )
			{
				Send( szMsg );
			}
			else
			{
				Send( "No messages in log queue!" );
			}
		}

		// +---------------------------------------------------+
		// |  GetLoggedCmdCount                                |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetLoggedCmdCount ) == 0 )
		{
			ASSERT_CLASS_PTR( METHOD_GetLoggedCmdCount,
							  "CArcDevice",
							  m_pCArcDev )

			Send( m_pCArcDev->GetLoggedCmdCount() );
		}

		// +---------------------------------------------------+
		// |  SetLogCmds                                       |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SetLogCmds ) == 0 )
		{
			bool bOnOff = false;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> bOnOff;

			ASSERT_CLASS_PTR( METHOD_SetLogCmds,
							  "CArcDevice",
							  m_pCArcDev )

			m_pCArcDev->SetLogCmds( bOnOff );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  Invalid Command                                  |
		// +---------------------------------------------------+
		else
		{
			ostringstream oss;

			oss << "Received invalid command. Offending data: "
				<< sInData << endl;

			clog << oss.str();

			SendException( oss.str().c_str() );
		}
	}
	catch ( exception &e )
	{
		clog << e.what() << endl;

		SendException( e.what() );
	}
	catch ( ... )
	{
		clog << "System Exception Caught!" << endl;

		SendException( "System Exception Caught!" );
	}
}


// /////////////////////////////////////////////////////////////////////////////
// /  CDeinterlace Handler                                                     /
// /////////////////////////////////////////////////////////////////////////////
void CArcAPIServer::HandleCDeinterlace( string sInData )
{
	string sTemp;

	const string sCmd = StripCmd( sInData );

	try
	{
		// +---------------------------------------------------+
		// |  RunAlg                                           |
		// +---------------------------------------------------+
		if ( sCmd.compare( METHOD_RunAlg ) == 0 )
		{
			int dPixOffset	=  0;
			int dRows       =  0;
			int dCols       =  0;
			int dAlgorithm  =  0;
			int dArg        = -1;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dRows
				>> dCols
				>> dAlgorithm
				>> dArg
				>> dPixOffset;

			if ( m_pCDeinterlace == NULL )
			{
				m_pCDeinterlace = new CDeinterlace();
			}

			ushort* pBuffer =
					static_cast<ushort *>(
							m_pCArcDev->CommonBufferVA() ) + dPixOffset;

			m_pCDeinterlace->RunAlg( ( void * )pBuffer,
									  dRows,
									  dCols,
									  dAlgorithm,
									  dArg );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  RunAlgFits                                       |
		// +---------------------------------------------------+
		if ( sCmd.compare( METHOD_RunAlgFits ) == 0 )
		{
			string sFilename  = "";
			int    dAlgorithm =  0;
			int    dArg       = -1;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> sFilename
				>> dAlgorithm
				>> dArg;

			if ( m_pCDeinterlace == NULL )
			{
				m_pCDeinterlace = new CDeinterlace();
			}

			long naxes[ 3 ] = { 0, 0, 0 };

			CFitsFile cFitsFile( sFilename.c_str(), CFitsFile::READWRITEMODE );
			cFitsFile.GetParameters( naxes );

			if ( naxes[ CFitsFile::NAXES_ROW ] <= 0 && naxes[ CFitsFile::NAXES_COL ] <= 0 )
			{
				ThrowException( METHOD_RunAlgFits,
								"Failed to read image dimensions from FITS: "
								+ sFilename );
			}

			unsigned short *pUShBuf =
							( unsigned short * )cFitsFile.Read();

			if ( pUShBuf != NULL )
			{
				m_pCDeinterlace->RunAlg( pUShBuf,
										 naxes[ CFitsFile::NAXES_ROW ],
										 naxes[ CFitsFile::NAXES_COL ],
										 dAlgorithm,
										 dArg );
			}

			cFitsFile.Write( pUShBuf );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  RunAlgFits3D                                     |
		// +---------------------------------------------------+
		if ( sCmd.compare( METHOD_RunAlgFits3D ) == 0 )
		{
			string sFilename  = "";
			int    dAlgorithm =  0;
			int    dArg       = -1;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> sFilename
				>> dAlgorithm
				>> dArg;

			if ( m_pCDeinterlace == NULL )
			{
				m_pCDeinterlace = new CDeinterlace();
			}

			long naxes[ 3 ] = { 0, 0, 0 };

			CFitsFile cFitsFile( sFilename.c_str(), CFitsFile::READWRITEMODE );
			cFitsFile.GetParameters( naxes );

			if ( naxes[ CFitsFile::NAXES_ROW ] <= 0 && naxes[ CFitsFile::NAXES_COL ] <= 0 )
			{
				ThrowException( METHOD_RunAlgFits3D,
								"Failed to read image dimensions from FITS: "
								+ sFilename );
			}

			for ( long i=0; i<naxes[ CFitsFile::NAXES_NOF ]; i++ )
			{
				unsigned short *pUShBuf =
							( unsigned short * )cFitsFile.Read();

				if ( pUShBuf != NULL )
				{
					m_pCDeinterlace->RunAlg( pUShBuf,
											 naxes[ CFitsFile::NAXES_ROW ],
											 naxes[ CFitsFile::NAXES_COL ],
											 dAlgorithm,
											 dArg );
				}

				cFitsFile.ReWrite3D( pUShBuf, i );
			}

			SendOK();
		}
	}
	catch ( bad_alloc &ba )
	{
		clog << ba.what() << endl;
		SendException( ba.what() );
	}
	catch ( exception &e )
	{
		clog << e.what() << endl;
		SendException( e.what() );
	}
	catch ( ... )
	{
		clog << "System Exception Caught!" << endl;
		SendException( "System Exception Caught!" );
	}
}


// /////////////////////////////////////////////////////////////////////////////
// /  CFitsFile Handler                                                        /
// /////////////////////////////////////////////////////////////////////////////
void CArcAPIServer::HandleCFitsFile( string sInData )
{
	string sTemp;

	const string sCmd = StripCmd( sInData );

	try
	{
		// +---------------------------------------------------+
		// |  CFitsFile Constructor                            |
		// +---------------------------------------------------+
		if ( sCmd.compare( METHOD_CFitsFile ) == 0 )
		{
			string sFilename    = "";
			int    dParams[ 4 ] = { 0, 0, 0, 0 };
			int    dParamCount  = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> sFilename;

			for ( int i=0; i<4; i++ )
			{
				if ( iss.good() )
				{
					iss >> dParams[ i ];
					dParamCount++;
				}
			}

			if ( m_pCFitsFile != NULL )
			{
				delete m_pCFitsFile;
			}

			if ( dParamCount > 1 )
			{
				m_pCFitsFile = new CFitsFile( sFilename.c_str(),
											  dParams[ 0 ],
											  dParams[ 1 ],
											  dParams[ 2 ],
											  ( dParams[ 3 ] != 0 ) );
			}
			else
			{
				m_pCFitsFile = new CFitsFile( sFilename.c_str(),
											  dParams[ 0 ] );
			}

			SendOK();
		}

		// +---------------------------------------------------+
		// |  CloseFits                                        |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_CloseFits ) == 0 )
		{
			if ( m_pCFitsFile == NULL )
			{
				ThrowException( METHOD_CloseFits,
								"No open FITS file!" );
			}

			delete m_pCFitsFile;
			m_pCFitsFile = NULL;

			SendOK();
		}

		// +---------------------------------------------------+
		// |  GetFitsFilename                                  |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetFitsFilename ) == 0 )
		{
			if ( m_pCFitsFile == NULL )
			{
				ThrowException(	METHOD_GetFitsFilename,
								"No open FITS file!" );
			}

			Send( "%s", m_pCFitsFile->GetFilename().c_str() );
		}

		// +---------------------------------------------------+
		// |  GetFitsHeader                                    |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetFitsHeader ) == 0 )
		{
			string sRecv = "";

			if ( m_pCFitsFile == NULL )
			{
				ThrowException(	METHOD_GetFitsHeader,
								"No open FITS file!" );
			}

			int dKeyCount = 0;
			char** pszHeader = m_pCFitsFile->GetHeader( &dKeyCount );

			Send( ( const char * )"%d", dKeyCount );

			for ( int i=0; i<dKeyCount; i++ )
			{
				sRecv = Recv();

				if ( sRecv.find( "OK" ) != string::npos )
				{
					Send( "%s", pszHeader[ i ] );
				}
				else
				{
					ThrowException( METHOD_GetFitsHeader,
									"Invalid client message: "
									+ sRecv );
				}
			}
		}

		// +---------------------------------------------------+
		// |  WriteFitsKeyword                                 |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_WriteFitsKeyword ) == 0 )
		{
			if ( m_pCFitsFile == NULL )
			{
				ThrowException( METHOD_WriteFitsKeyword,
								"No open FITS file!" );
			}

			string sKeyVal  = "";
			string sKey     = "";
			string sComment = "";
			int    dValType = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> sKey
				>> sKeyVal
				>> dValType
				>> sComment;

			//
			// Remove any '+-+' symbols from the string. These are
			// added by the client library for strings with spaces.
			//
			FixMultiStringData( sKeyVal );

			//
			// Remove any '+-+' symbols from the string. These are
			// added by the client library for strings with spaces.
			//
			if ( !sComment.empty() )
			{
				FixMultiStringData( sComment );
			}

			switch ( dValType )
			{
				case CFitsFile::FITS_STRING_KEY:
				{
					m_pCFitsFile->WriteKeyword( ( char * )sKey.c_str(),
												( void * )sKeyVal.c_str(),
												 dValType,
												( char * )sComment.c_str() );
				}
				break;

				case CFitsFile::FITS_COMMENT_KEY:
				case CFitsFile::FITS_HISTORY_KEY:
				{
					m_pCFitsFile->WriteKeyword( ( char * )NULL,
												( void * )sKeyVal.c_str(),
												 dValType,
												( char * )NULL );
				}
				break;

				case CFitsFile::FITS_DATE_KEY:
				{
					m_pCFitsFile->WriteKeyword( ( char * )NULL,
												( void * )NULL,
												 dValType,
												( char * )NULL );
				}
				break;

				case CFitsFile::FITS_INTEGER_KEY:
				case CFitsFile::FITS_LOGICAL_KEY:
				{
					int dKeyVal = atoi( sKeyVal.c_str() );

					m_pCFitsFile->WriteKeyword( ( char * )sKey.c_str(),
												( void * )&dKeyVal,
												 dValType,
												( char * )sComment.c_str() );
				}
				break;

				case CFitsFile::FITS_DOUBLE_KEY:
				{
					double gKeyVal = atof( sKeyVal.c_str() );

					m_pCFitsFile->WriteKeyword( ( char * )sKey.c_str(),
												( void * )&gKeyVal,
												 dValType,
												( char * )sComment.c_str() );
				}
				break;

				default:
				{
					ostringstream oss;

					oss << "Invalid FITS keyword type: "
						<< dValType;

					ThrowException( METHOD_WriteFitsKeyword,
									oss.str() );
				}
			}

			SendOK();
		}

		// +---------------------------------------------------+
		// |  UpdateFitsKeyword                                |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_UpdateFitsKeyword ) == 0 )
		{
			if ( m_pCFitsFile == NULL )
			{
				ThrowException( METHOD_UpdateFitsKeyword,
								"No open FITS file!" );
			}

			string sKeyVal  = "";
			string sKey     = "";
			string sComment = "";
			int    dValType = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> sKey
				>> sKeyVal
				>> dValType
				>> sComment;

			//
			// Remove any '+-+' symbols from the string. These are
			// added by the client library for strings with spaces.
			//
			FixMultiStringData( sKeyVal );

			//
			// Remove any '+-+' symbols from the string. These are
			// added by the client library for strings with spaces.
			//
			if ( !sComment.empty() )
			{
				FixMultiStringData( sComment );
			}

			switch ( dValType )
			{
				case CFitsFile::FITS_STRING_KEY:
				{
					m_pCFitsFile->UpdateKeyword( ( char * )sKey.c_str(),
												 ( void * )sKeyVal.c_str(),
												  dValType,
												 ( char * )sComment.c_str() );
				}
				break;

				case CFitsFile::FITS_COMMENT_KEY:
				case CFitsFile::FITS_HISTORY_KEY:
				{
					m_pCFitsFile->UpdateKeyword( ( char * )NULL,
												 ( void * )sKeyVal.c_str(),
												  dValType,
												 ( char * )NULL );
				}
				break;

				case CFitsFile::FITS_DATE_KEY:
				{
					m_pCFitsFile->UpdateKeyword( ( char * )NULL,
												 ( void * )NULL,
												  dValType,
												 ( char * )NULL );
				}
				break;

				case CFitsFile::FITS_INTEGER_KEY:
				case CFitsFile::FITS_LOGICAL_KEY:
				{
					int dKeyVal = atoi( sKeyVal.c_str() );

					m_pCFitsFile->UpdateKeyword( ( char * )sKey.c_str(),
												 ( void * )&dKeyVal,
												  dValType,
												 ( char * )sComment.c_str() );
				}
				break;

				case CFitsFile::FITS_DOUBLE_KEY:
				{
					double gKeyVal = atof( sKeyVal.c_str() );

					m_pCFitsFile->UpdateKeyword( ( char * )sKey.c_str(),
												 ( void * )&gKeyVal,
												  dValType,
												 ( char * )sComment.c_str() );
				}
				break;

				default:
				{
					ostringstream oss;

					oss << "Invalid FITS keyword type: "
						<< dValType;

					ThrowException( METHOD_UpdateFitsKeyword,
									oss.str() );
				}
			}

			SendOK();
		}

		// +---------------------------------------------------+
		// |  GetFitsParameters                                |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetFitsParameters ) == 0 )
		{
			if ( m_pCFitsFile == NULL )
			{
				ThrowException(	METHOD_GetFitsParameters,
								"No open FITS file!" );
			}

			long lNAxes[ 3 ] = { 0, 0, 0 };
			int  dNAxis      = 0;
			int  dBpp        = 0;

			m_pCFitsFile->GetParameters( lNAxes, &dNAxis, &dBpp );

			Send( "%l %l %l %d %d",
				   lNAxes[ 0 ],
				   lNAxes[ 1 ],
				   lNAxes[ 2 ],
				   dNAxis,
				   dBpp );
		}

		// +---------------------------------------------------+
		// |  GenerateFitsTestData                             |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GenerateFitsTestData ) == 0 )
		{
			if ( m_pCFitsFile == NULL )
			{
				ThrowException(	METHOD_GenerateFitsTestData,
								"No open FITS file!" );
			}

			m_pCFitsFile->GenerateTestData();

			SendOK();
		}

		// +---------------------------------------------------+
		// |  ReOpenFits                                       |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_ReOpenFits ) == 0 )
		{
			if ( m_pCFitsFile == NULL )
			{
				ThrowException(	METHOD_ReOpenFits,
								"No open FITS file!" );
			}

			m_pCFitsFile->ReOpen();

			SendOK();
		}

		// +---------------------------------------------------+
		// |  CompareFits                                      |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_CompareFits ) == 0 )
		{
			ThrowException(
				METHOD_CompareFits,
				"Sorry, but this method is not available remotely!" );
		}

		// +---------------------------------------------------+
		// |  ResizeFits                                       |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_ResizeFits ) == 0 )
		{
			if ( m_pCFitsFile == NULL )
			{
				ThrowException(	METHOD_ResizeFits,
								"No open FITS file!" );
			}

			int dRows = 0;
			int dCols = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp >> dRows >> dCols;

			m_pCFitsFile->Resize( dRows, dCols );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  WriteFitsSubImage                                |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_WriteFitsSubImage ) == 0 )
		{
			if ( m_pCFitsFile == NULL )
			{
				ThrowException( METHOD_WriteFitsSubImage,
								"No open FITS file!" );
			}

			ULONG_PTR pData  = 0;
			int       dLLRow = 0;
			int       dLLCol = 0;
			int       dURRow = 0;
			int       dURCol = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dLLRow
				>> dLLCol
				>> dURRow
				>> dURCol
				>> pData;

			m_pCFitsFile->WriteSubImage( dLLRow,
										 dLLCol,
										 dURRow,
										 dURCol,
										 ( void * )pData );
			SendOK();
		}

		// +---------------------------------------------------+
		// |  WriteFits                                        |
		// +---------------------------------------------------+
		// |  This must come after WriteSubImage because they  |
		// |  both have "Write" in them and "Write" is found   |
		// |  first.                                           |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_WriteFits ) == 0 )
		{
			if ( m_pCFitsFile == NULL )
			{
				ThrowException(	METHOD_WriteFits,
								"No open FITS file!" );
			}

			unsigned int udBytesToSkip  = 0;
			unsigned int udBytesToWrite = 0;
			int dFPixl = -1;

			istringstream iss( sInData.c_str() );

			iss >> sTemp;

			if ( iss.good() ) { iss >> udBytesToSkip; }
			if ( iss.good() ) { iss >> udBytesToWrite; }
			if ( iss.good() ) { iss >> dFPixl; }

			if ( udBytesToWrite == 0 )
			{
				m_pCFitsFile->Write( m_pCArcDev->CommonBufferVA() );
			}
			else
			{
				unsigned char *pUCharBuf =
							( ( unsigned char * )m_pCArcDev->CommonBufferVA() )
							+ udBytesToSkip;

				if ( pUCharBuf == NULL )
				{
					ThrowException(
						METHOD_WriteFits,
						"Invalid image buffer ( NULL pointer )!" );
				}

				m_pCFitsFile->Write( pUCharBuf,
									 udBytesToWrite,
									 dFPixl );
			}

			SendOK();
		}

		// +---------------------------------------------------+
		// |  ReadFitsSubImage                                 |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_ReadFitsSubImage ) == 0 )
		{
			ThrowException( METHOD_ReadFitsSubImage,
				"Sorry, but this method is currently not available remotely!" );
		}

		// +---------------------------------------------------+
		// |  ReadFits                                         |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_ReadFits ) == 0 )
		{
			ThrowException(
				METHOD_ReadFits,
				"Sorry, but this method is currently not available remotely!" );
		}

		// +---------------------------------------------------+
		// |  WriteFits3D                                      |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_WriteFits3D ) == 0 )
		{
			if ( m_pCFitsFile == NULL )
			{
				ThrowException( METHOD_WriteFits3D,
								"No open FITS file!" );
			}

			unsigned int udByteOffset = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> udByteOffset;

			unsigned char *pUCharBuf =
						( ( unsigned char * )m_pCArcDev->CommonBufferVA() )
						+ udByteOffset;

			if ( pUCharBuf == NULL )
			{
				ThrowException(
					METHOD_WriteFits3D,
					"Invalid image buffer ( NULL pointer )!" );
			}

			m_pCFitsFile->Write3D( ( void * )pUCharBuf );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  ReWriteFits3D                                    |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_ReWriteFits3D ) == 0 )
		{
			if ( m_pCFitsFile == NULL )
			{
				ThrowException( METHOD_ReWriteFits3D,
								"No open FITS file!" );
			}

			int dData = 0;
			int dImageNumber = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dData >> dImageNumber;

			m_pCFitsFile->ReWrite3D( ( void * )dData, dImageNumber );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  ReadFits3D                                       |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_ReadFits3D ) == 0 )
		{
			ThrowException(
				METHOD_ReadFits3D,
				"Sorry, but this method is currently not available remotely!" );
		}
	}
	catch ( bad_alloc &ba )
	{
		clog << ba.what() << endl;
		SendException( ba.what() );
	}
	catch ( exception &e )
	{
		clog << e.what() << endl;
		SendException( e.what() );
	}
	catch ( ... )
	{
		clog << "System Exception Caught!" << endl;
		SendException( "System Exception Caught!" );
	}
}

// /////////////////////////////////////////////////////////////////////////////
// /  CTiffFile Handler                                                        /
// /////////////////////////////////////////////////////////////////////////////
void CArcAPIServer::HandleCTiffFile( string sInData )
{
	string sTemp;

	const string sCmd = StripCmd( sInData );

	try
	{
		// +---------------------------------------------------+
		// |  CTiffFile Constructor                            |
		// +---------------------------------------------------+
		if ( sCmd.compare( METHOD_CTiffFile ) == 0 )
		{
			string sFilename = "";
			int    dMode     = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> sFilename
				>> dMode;

			if ( m_pCTiffFile != NULL )
			{
				delete m_pCTiffFile;
			}

			m_pCTiffFile = new CTiffFile( sFilename.c_str(),
										  dMode );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  CloseTiff                                        |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_CloseTiff ) == 0 )
		{
			if ( m_pCTiffFile == NULL )
			{
				ThrowException( METHOD_CloseTiff,
								"No open TIFF file!" );
			}

			if ( m_pCTiffFile != NULL )
			{
				delete m_pCTiffFile;
			}

			m_pCTiffFile = NULL;

			SendOK();
		}

		// +---------------------------------------------------+
		// |  GetTiffRows                                      |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetTiffRows ) == 0 )
		{
			if ( m_pCTiffFile == NULL )
			{
				ThrowException( METHOD_GetTiffRows,
								"No open TIFF file!" );
			}

			Send( m_pCTiffFile->GetRows() );
		}

		// +---------------------------------------------------+
		// |  GetTiffCols                                      |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetTiffCols ) == 0 )
		{
			if ( m_pCTiffFile == NULL )
			{
				ThrowException( METHOD_GetTiffCols,
								"No open TIFF file!" );
			}

			Send( m_pCTiffFile->GetCols() );
		}

		// +---------------------------------------------------+
		// |  WriteTiff                                        |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_WriteTiff ) == 0 )
		{
			if ( m_pCTiffFile == NULL )
			{
				ThrowException( METHOD_WriteTiff,
								"No open TIFF file!" );
			}

			int  dRows = 0;
			int  dCols = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dRows
				>> dCols;

			m_pCTiffFile->Write( m_pCArcDev->CommonBufferVA(),
								  dRows,
								  dCols );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  ReadTiff                                         |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_ReadTiff ) == 0 )
		{
			ThrowException(
				METHOD_ReadTiff,
				"Sorry, but this method is currently not available remotely!" );
		}
	}
	catch ( bad_alloc &ba )
	{
		clog << ba.what() << endl;
		SendException( ba.what() );
	}
	catch ( exception &e )
	{
		clog << e.what() << endl;
		SendException( e.what() );
	}
	catch ( ... )
	{
		clog << "System Exception Caught!" << endl;
		SendException( "System Exception Caught!" );
	}
}

// /////////////////////////////////////////////////////////////////////////////
// /  CImage Handler                                                           /
// /////////////////////////////////////////////////////////////////////////////
void CArcAPIServer::HandleCImage( string sInData )
{
	string sTemp;

	const string sCmd = StripCmd( sInData );

	try
	{
		// +---------------------------------------------------+
		// |  Histogram                                        |
		// +---------------------------------------------------+
		if ( sCmd.compare( METHOD_Histogram ) == 0 )
		{
			ULONG_PTR pMem = 0;
			int       dHistSize = 0;
			int       adParams[ 6 ] = { 0, 0, 0, 0, 0, 0 };
			int*      pHist = NULL;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> pMem
				>> adParams[ 0 ]
				>> adParams[ 1 ]
				>> adParams[ 2 ];

			for ( int i=3; i<6; i++ )
			{
				if ( iss.good() )
				{
					iss >> adParams[ i ];
				}
				else
				{
					break;
				}
			}

			if ( m_pCImage == NULL )
			{
				m_pCImage = new CImage();

				if ( m_pCImage == NULL )
				{
					throw runtime_error(
								"( CImage ): Invalid pointer!" );
				}
			}

			try
			{
				if ( adParams[ 5 ] > 0 )
				{
					pHist = m_pCImage->Histogram( dHistSize,
												  ( void * )pMem,
												  adParams[ 0 ],
												  adParams[ 1 ],
												  adParams[ 2 ],
												  adParams[ 3 ],
												  adParams[ 4 ],
												  adParams[ 5 ] );
				}
				else
				{
					pHist = m_pCImage->Histogram( dHistSize,
												  ( void * )pMem,
												  adParams[ 0 ],
												  adParams[ 1 ],
												  adParams[ 2 ] );
				}

				Send( dHistSize );

				string sMsgRecvd = Recv();

				if ( sMsgRecvd.find( "OK" ) != string::npos )
				{
					Send( pHist, ( dHistSize * sizeof( int ) ) );
				}
				else
				{
					ThrowException( METHOD_Histogram,
									"Invalid message received: "
									+ sMsgRecvd );
				}

				m_pCImage->FreeHistogram();
			}
			catch ( exception & )
			{
				m_pCImage->FreeHistogram();

				throw;
			}
		}

		// +---------------------------------------------------+
		// |  GetImageRow                                      |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetImageRow ) == 0 )
		{
			unsigned short* pBuf = NULL;
			int dRow  = 0;
			int dRows = 0;
			int dCol1 = 0;
			int dCol2 = 0;
			int dCols = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dRow
				>> dCol1
				>> dCol2
				>> dRows
				>> dCols;

			int dElemCount = 0;

			if ( m_pCImage == NULL )
			{
				m_pCImage = new CImage();

				if ( m_pCImage == NULL )
				{
					throw runtime_error(
								"( CImage ): Invalid pointer!" );
				}
			}

			try
			{
				pBuf = ( unsigned short * )
						m_pCImage->GetImageRow( m_pCArcDev->CommonBufferVA(),
												dRow,
											    dCol1,
												dCol2,
												dRows,
												dCols,
												dElemCount );
				Send( dElemCount );

				string sRecv = Recv();

				if ( sRecv.find( "OK" ) != string::npos )
				{
					Send( pBuf, ( dElemCount * sizeof( unsigned short ) ) );
				}
				else
				{
					ThrowException(	METHOD_GetImageRow,
									"Invalid client message: "
									+ sRecv );
				}

				m_pCImage->Free( pBuf );
			}
			catch ( exception& )
			{
				m_pCImage->Free( pBuf );

				throw;
			}
		}

		// +---------------------------------------------------+
		// |  GetImageCol                                      |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetImageCol ) == 0 )
		{
			unsigned short* pBuf = NULL;
			int dCol  = 0;
			int dRow1 = 0;
			int dRow2 = 0;
			int dRows = 0;
			int dCols = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dCol
				>> dRow1
				>> dRow2
				>> dRows
				>> dCols;

			int dElemCount = 0;

			if ( m_pCImage == NULL )
			{
				m_pCImage = new CImage();

				if ( m_pCImage == NULL )
				{
					throw runtime_error(
								"( CImage ): Invalid pointer!" );
				}
			}

			try
			{
				pBuf = ( unsigned short * )
							m_pCImage->GetImageCol( m_pCArcDev->CommonBufferVA(),
													dCol,
												    dRow1,
													dRow2,
													dRows,
													dCols,
													dElemCount );
				Send( dElemCount );

				string sRecv = Recv();

				if ( sRecv.find( "OK" ) != string::npos )
				{
					Send( pBuf, ( dElemCount * sizeof( unsigned short ) ) );
				}
				else
				{
					ThrowException( METHOD_GetImageCol,
									"Invalid client message: "
									+ sRecv );
				}

				m_pCImage->Free( pBuf );
			}
			catch ( exception& )
			{
				m_pCImage->Free( pBuf );

				throw;
			}
		}

		// +---------------------------------------------------+
		// |  GetImageRowArea                                  |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetImageRowArea ) == 0 )
		{
			float* pBuf = NULL;
			int dRow1 = 0;
			int dRow2 = 0;
			int dRows = 0;
			int dCol1 = 0;
			int dCol2 = 0;
			int dCols = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dRow1
				>> dRow2
				>> dCol1
				>> dCol2
				>> dRows
				>> dCols;

			int dElemCount = 0;

			if ( m_pCImage == NULL )
			{
				m_pCImage = new CImage();

				if ( m_pCImage == NULL )
				{
					throw runtime_error(
								"( CImage ): Invalid pointer!" );
				}
			}

			try
			{
				pBuf = ( float * )m_pCImage->GetImageRowArea( m_pCArcDev->CommonBufferVA(),
															  dRow1,
															  dRow2,
															  dCol1,
															  dCol2,
															  dRows,
															  dCols,
															  dElemCount );
				Send( dElemCount );

				string sRecv = Recv();

				if ( sRecv.find( "OK" ) != string::npos )
				{
					Send( pBuf, ( dElemCount * sizeof( float ) ) );
				}
				else
				{
					ThrowException(	METHOD_GetImageRowArea,
									"Invalid client message: "
									+ sRecv );
				}

				m_pCImage->Free( pBuf, sizeof( float ) );
			}
			catch ( std::exception& )
			{
				m_pCImage->Free( pBuf, sizeof( float ) );

				throw;
			}
		}

		// +---------------------------------------------------+
		// |  GetImageColArea                                  |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetImageColArea ) == 0 )
		{
			float* pBuf = NULL;
			int dRow1 = 0;
			int dRow2 = 0;
			int dRows = 0;
			int dCol1 = 0;
			int dCol2 = 0;
			int dCols = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dRow1
				>> dRow2
				>> dCol1
				>> dCol2
				>> dRows
				>> dCols;

			int dElemCount = 0;

			if ( m_pCImage == NULL )
			{
				m_pCImage = new CImage();

				if ( m_pCImage == NULL )
				{
					throw runtime_error(
								"( CImage ): Invalid pointer!" );
				}
			}

			try
			{
				pBuf = ( float * )m_pCImage->GetImageColArea( m_pCArcDev->CommonBufferVA(),
															  dRow1,
															  dRow2,
															  dCol1,
															  dCol2,
															  dRows,
															  dCols,
															  dElemCount );
				Send( dElemCount );

				string sRecv = Recv();

				if ( sRecv.find( "OK" ) != string::npos )
				{
					Send( pBuf, ( dElemCount * sizeof( float ) ) );
				}
				else
				{
					ThrowException(	METHOD_GetImageColArea,
									"Invalid client message: "
									+ sRecv );
				}

				m_pCImage->Free( pBuf, sizeof( float ) );
			}
			catch ( std::exception& )
			{
				m_pCImage->Free( pBuf, sizeof( float ) );

				throw;
			}
		}

		// +---------------------------------------------------+
		// |  SubtractImageHalves                              |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_SubtractImageHalves ) == 0 )
		{
			int dRows = 0;
			int dCols = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> dRows
				>> dCols;

			if ( m_pCImage == NULL )
			{
				m_pCImage = new CImage();

				if ( m_pCImage == NULL )
				{
					throw runtime_error(
								"( CImage ): Invalid pointer!" );
				}
			}

			m_pCImage->SubtractImageHalves(
									m_pCArcDev->CommonBufferVA(),
									dRows,
									dCols );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  VerifyImageAsSynthetic                           |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_VerifyImageAsSynthetic ) == 0 )
		{
			int dRows = 0;
			int dCols = 0;

			istringstream iss( sInData.c_str() );
			iss >> sTemp >> dRows >> dCols;

			if ( m_pCImage == NULL )
			{
				m_pCImage = new CImage();
	
				if ( m_pCImage == NULL )
				{
					throw runtime_error(
								"( CImage ): Invalid pointer!" );
				}
			}

			m_pCImage->VerifyImageAsSynthetic(
									m_pCArcDev->CommonBufferVA(),
									dRows,
									dCols );

			SendOK();
		}

		// +---------------------------------------------------+
		// |  GetStats                                         |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetStats ) == 0 )
		{
			ULONG_PTR pMem = 0;
			int adParams[ 6 ] = { 0, 0, 0, 0, 0, 0 };
			CImage::CImgStats cImgStats;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> pMem
				>> adParams[ 0 ]
				>> adParams[ 1 ]
				>> adParams[ 2 ];

			for ( int i=3; i<6; i++ )
			{
				if ( iss.good() )
				{
					iss >> adParams[ i ];
				}
				else
				{
					break;
				}
			}

			if ( m_pCImage == NULL )
			{
				m_pCImage = new CImage();
	
				if ( m_pCImage == NULL )
				{
					throw runtime_error(
								"( CImage ): Invalid pointer!" );
				}
			}

			if ( adParams[ 5 ] > 0 )
			{
				cImgStats = m_pCImage->GetStats( ( void * )pMem,
												   adParams[ 0 ],
												   adParams[ 1 ],
												   adParams[ 2 ],
												   adParams[ 3 ],
												   adParams[ 4 ],
												   adParams[ 5 ] );
			}
			else
			{
				cImgStats = m_pCImage->GetStats( ( void * )pMem,
												   adParams[ 0 ],
												   adParams[ 1 ],
												   adParams[ 2 ] );
			}

			Send( &cImgStats, sizeof( CImage::CImgStats ) );
		}

		// +---------------------------------------------------+
		// |  GetFitsStats                                     |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetFitsStats ) == 0 )
		{
			string sFilename = "";
			int    dRow1     = 0;
			int    dRow2     = 0;
			int    dCol1     = 0;
			int    dCol2     = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> sFilename
				>> dRow1
				>> dRow2
				>> dCol1
				>> dCol2;

			if ( m_pCImage == NULL )
			{
				m_pCImage = new CImage();
	
				if ( m_pCImage == NULL )
				{
					throw runtime_error(
								"( CImage ): Invalid pointer!" );
				}
			}

			//  Open the FITS file #1 and read the image data
			// +-----------------------------------------------------+
			CFitsFile cFitsFile( sFilename.c_str() );

			void* pBuffer = cFitsFile.ReadSubImage( dRow1,
													dCol1,
													dRow2,
													dCol2 );

			if ( pBuffer == NULL )
			{
				ThrowException( METHOD_GetFitsStats,
								"Failed to read image data from FITS: "
								+ sFilename );
			}

			CImage cImage;
			CImage::CImgStats cImgStats = cImage.GetStats( pBuffer,
														   ( dRow2 - dRow1 ),
														   ( dCol2 - dCol1 ) );

			Send( &cImgStats, sizeof( CImage::CImgStats ) );
		}

		// +---------------------------------------------------+
		// |  GetDifStats                                      |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetDiffStats ) == 0 )
		{
			ULONG_PTR pMem1 = 0;
			ULONG_PTR pMem2 = 0;
			int adParams[ 6 ] = { 0, 0, 0, 0, 0, 0 };
			CImage::CImgDifStats cImgDifStats;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> pMem1
				>> pMem2
				>> adParams[ 0 ]
				>> adParams[ 1 ]
				>> adParams[ 2 ];

			for ( int i=3; i<6; i++ )
			{
				if ( iss.good() )
				{
					iss >> adParams[ i ];
				}
				else
				{
					break;
				}
			}

			if ( m_pCImage == NULL )
			{
				m_pCImage = new CImage();
	
				if ( m_pCImage == NULL )
				{
					throw runtime_error(
								"( CImage ): Invalid pointer!" );
				}
			}

			if ( adParams[ 5 ] > 0 )
			{
				cImgDifStats = m_pCImage->GetDiffStats( ( void * )pMem1,
														( void * )pMem2,
														 adParams[ 0 ],
														 adParams[ 1 ],
														 adParams[ 2 ],
														 adParams[ 3 ],
														 adParams[ 4 ],
														 adParams[ 5 ] );
			}
			else
			{
				cImgDifStats = m_pCImage->GetDiffStats( ( void * )pMem1,
														( void * )pMem2,
														 adParams[ 0 ],
														 adParams[ 1 ],
														 adParams[ 2 ] );
			}

			Send( &cImgDifStats, sizeof( CImage::CImgDifStats ) );
		}

		// +---------------------------------------------------+
		// |  GetDifStats                                      |
		// +---------------------------------------------------+
		else if ( sCmd.compare( METHOD_GetFitsDiffStats ) == 0 )
		{
			string sFilename1 = "";
			string sFilename2 = "";
			int    dRow1      = 0;
			int    dRow2      = 0;
			int    dCol1      = 0;
			int    dCol2      = 0;

			istringstream iss( sInData.c_str() );

			iss >> sTemp
				>> sFilename1
				>> sFilename2
				>> dRow1
				>> dRow2
				>> dCol1
				>> dCol2;

			if ( m_pCImage == NULL )
			{
				m_pCImage = new CImage();
	
				if ( m_pCImage == NULL )
				{
					throw runtime_error(
								"( CImage ): Invalid pointer!" );
				}
			}

			void *pBuffer1 = NULL, *pBuffer2 = NULL;

			//  Open the FITS file #1 and read the image data
			// +-----------------------------------------------------+
			CFitsFile cFitsFile1( sFilename1.c_str() );

			pBuffer1 = cFitsFile1.ReadSubImage( dRow1,
												dCol1,
												dRow2,
												dCol2 );
			if ( pBuffer1 == NULL )
			{
				ThrowException(	METHOD_GetFitsDiffStats,
								"Failed to read image data from FITS: "
								+ sFilename1 );
			}

			//  Open the FITS file #2 and read the image data
			// +-----------------------------------------------------+
			CFitsFile cFitsFile2( sFilename2.c_str() );

			pBuffer2 = cFitsFile2.ReadSubImage( dRow1,
												dCol1,
												dRow2,
												dCol2 );
			if ( pBuffer2 == NULL )
			{
				ThrowException( METHOD_GetFitsDiffStats,
								"Failed to read image data from FITS: "
								+ sFilename2 );
			}

			CImage::CImgDifStats cImgDifStats =
								m_pCImage->GetDiffStats( pBuffer1,
														 pBuffer2,
														 ( dRow2 - dRow1 ),
														 ( dCol2 - dCol1 ) );

			Send( &cImgDifStats, sizeof( CImage::CImgDifStats ) );
		}
	}
	catch ( bad_alloc &ba )
	{
		clog << ba.what() << endl;
		SendException( ba.what() );
	}
	catch ( exception &e )
	{
		clog << e.what() << endl;
		SendException( e.what() );
	}
	catch ( ... )
	{
		clog << "System Exception Caught!" << endl;
		SendException( "System Exception Caught!" );
	}
}

const string CArcAPIServer::GetTempFileName()
{
	char szTempFileName[ MAX_PATH ];

#ifdef WIN32
	char szPathBuffer[ MAX_PATH ];

	// Get the temp path
	DWORD dwRetVal = GetTempPathA( MAX_PATH,	// length of the buffer
								   szPathBuffer );	// buffer for path

	if ( dwRetVal > MAX_PATH || ( dwRetVal == 0 ) )
	{
		DWORD dwError = GetLastError();

		ostringstream oss;
		oss << "GetTempPath failed with error " << dwError
		    << " - " << GetSystemMessage( int( dwError ) );

		ThrowException( "GetTempFileName", oss.str() );
	}

	UINT uRetVal = GetTempFileNameA( szPathBuffer,		// directory for tmp files
									 "lod",		// temp file name prefix 
									 0,			// create unique name 
									 szTempFileName );	// buffer for name 
	if ( uRetVal == 0 )
	{
		ThrowException( "GetTempFileName",
			 "Error occurred creating unique filename!" );
	}
#else
	string sTmp( P_tmpdir );
 
	if ( sTmp.empty() )
	{
        	struct stat st;
 
	       	if ( stat( "/tmp", &st ) == 0 )
                {
			sTmp = sTmp.assign( "/tmp" );
		}

		if ( sTmp.empty() )
		{
			sTmp = sTmp.assign( "." );
		}
	}

	sTmp = sTmp.append( "/" );

	clock_t tClock = clock();

	sprintf( szTempFileName,
			 "%slodfile%d",
			 sTmp.c_str(),
			 int( tClock ) );
#endif

	return string( szTempFileName );
}

void CArcAPIServer::FixMultiStringData( string& sData )
{
	//
	// Remove any '+-+' symbols from the value string. These
	// are added by the client library for strings with spaces.
	//
	size_t tPos = 0;

	for ( size_t i=0; i<sData.size(); i++ )
	{
		if ( ( tPos = sData.find( "+-+", tPos ) ) != string::npos )
			sData = sData.replace( tPos, 3, " " );
	}
}

const string CArcAPIServer::StripCmd( const string sInData )
{
	string sCmd = ( sInData.substr( sInData.rfind( "::" ) + 2 ) );
	sCmd = sCmd.substr( 0, sCmd.find( " " ) );
	return sCmd;
}

string CArcAPIServer::RecvDspFile( const string sBoard )
{
	string sFileName = "";

	Send( sBoard.c_str() );

	int dLength = atoi( Recv().c_str() );

	if ( dLength > 0 )
	{
		SendOK();
	}
	else
	{
		ostringstream oss;

		oss << "Invalid " << sBoard << " file length: "
			<< dLength << " bytes";

		ThrowException( "RecvDspFile", oss.str() );
	}

	sFileName = GetTempFileName();

	RecvFile( sFileName, dLength );

	return sFileName;
}

int CArcAPIServer::Recv( void* pBuffer, int dSize )
{
	char* pBuf = ( char * )pBuffer;
	ZeroMem( pBuf, dSize );

	int nret = recv( m_tAcceptSocket,
					 pBuf,
					 dSize,				// Complete size of buffer
					 MSG_WAITALL );

	if ( nret == SOCKET_ERROR )
	{
#ifdef WIN32
		DWORD dwError = GetError();

		// Check that the MSG_WAITALL flag is supported!
		if ( dwError == WSAEOPNOTSUPP )
		{
			nret = recv( m_tAcceptSocket,
						 pBuf,
						 dSize,				// Complete size of buffer
						 0 );

			if ( nret == SOCKET_ERROR )
			{
				ThrowException( "Recv",
								"recv() return error: " +
								 GetSystemMessage( GetError() ) );
			}
		}
		else
		{
			ThrowException( "Recv",
							"recv() return error: " +
							 GetSystemMessage( GetError() ) );
		}
#else
		ThrowException( "Recv",
						"recv() return error: " +
						 GetSystemMessage( GetError() ) );
#endif
	}

	if ( nret != dSize )
	{
		ostringstream oss;

		oss << "Insufficient data transfer. Expected: "
			<< dSize << " bytes.  Received: " << nret
			<< " bytes.";

		ThrowException( "Recv", oss.str() );
	}

	return nret;
}

string CArcAPIServer::Recv()
{
	char *szBuffer = new char[ 4096 ];
	ZeroMem( szBuffer, 4096 );

	int nret = recv( m_tAcceptSocket,
					 szBuffer,
					 4096,			// Complete size of buffer
					 0 );

	string sBuffer( szBuffer );

	delete [] szBuffer;

	//   Connection terminated, recv() returns 0
	// +------------------------------------------+
	if ( nret == 0 )
	{
		ThrowException( "Recv",
			"Connection gracefully terminated!" );
	}

	//   Error occurred
	// +------------------------------------------+
	if ( nret == SOCKET_ERROR )
	{
		ThrowException( "Recv",
						"recv() return error: " +
						 GetSystemMessage( GetError() ) );
	}

	return sBuffer;
}

void CArcAPIServer::RecvFile( string sFilename, int dLength )
{
	char* pBuffer = new char[ dLength ];

	try
	{
		Recv( pBuffer, dLength );

//		clog.write( pBuffer, dLength );

		ofstream ofs( sFilename.c_str(), ios::binary );
		ofs.write( pBuffer, dLength );
		ofs.close();

		delete [] pBuffer;
	}
	catch ( exception& )
	{
		delete [] pBuffer;

		throw;
	}
}

string CArcAPIServer::Peek()
{
	char *szBuffer = new char[ 1024 ];
	ZeroMem( szBuffer, 1024 );

	int nret = recv( m_tAcceptSocket,
					 szBuffer,
					 1024,		// Complete size of buffer
					 MSG_PEEK );

	string sBuffer( szBuffer );

	delete [] szBuffer;

	//   Connection terminated, recv() returns 0
	// +------------------------------------------+
	if ( nret == 0 )
	{
		ThrowException( "Peek",
			"Connection gracefully terminated!" );
	}

	//   Error occurred
	// +------------------------------------------+
	if ( nret == SOCKET_ERROR )
	{
		ThrowException( "Peek",
						"recv() return error: " +
						 GetSystemMessage( GetError() ) );
	}

	return sBuffer;
}

void CArcAPIServer::Send( int dVal )
{
	ostringstream oss;
	oss << dVal << ends;
	Send( oss.str().c_str() );
}

void CArcAPIServer::Send( long lVal )
{
	ostringstream oss;
	oss << lVal << ends;
	Send( oss.str().c_str() );
}

void CArcAPIServer::Send( double gVal )
{
	ostringstream oss;
	oss << gVal << ends;
	Send( oss.str().c_str() );
}

void CArcAPIServer::Send( ULONG_PTR ulptr )
{
	ostringstream oss;
	oss << hex << ulptr << ends;
	Send( oss.str().c_str() );
}

void CArcAPIServer::Send( void* pBuffer, int dSize )
{
	int nret = send( m_tAcceptSocket,
					 static_cast<const char *>( pBuffer ),
					 dSize,
					 0 );						// Most often is zero, but see MSDN for other options

	if ( nret == SOCKET_ERROR )
	{
		ThrowException( "Send",
						"send() return error: " +
						 GetSystemMessage( GetError() ) );
	}
}

// +----------------------------------------------------------------------------
// |  Send
// +----------------------------------------------------------------------------
// |  Inserts a message into the log queue. It dumps the oldest message if
// |  the queue size is greater than or equal to Q_MAX.
// |
// |  <IN> -> fmt - C-printf style format (sort of):
// |				%d   -> Integer
// |				%f   -> Double
// |				%l   -> Long
// |				%s   -> Char *, std::string not accepted
// |				%x,X -> Integer as hex, produces uppercase only
// |				%e   -> Special case that produces a string message from
// |				        one of the system functions ::GetLastError or
// |				        strerror, depending on the OS.
// +----------------------------------------------------------------------------
void CArcAPIServer::Send( const char *fmt, ... )
{
	ostringstream oss;
	char *ch, *szVal;
	va_list ap;

	va_start( ap, fmt );

	for ( ch = ( char * )fmt; *ch; ch++ )
	{
		if ( *ch != '%' )
		{
			oss << *ch;
			continue;
		}

		switch ( *++ch )
		{
			case 'd':
				oss << va_arg( ap, int );
				break;

			case 'f':
				oss << va_arg( ap, double );
				break;

			case 'l':
				oss << va_arg( ap, long );
				break;

			case 's':
				for ( szVal = va_arg( ap, char * ); *szVal; szVal++ )
					oss << *szVal;
				break;

			case 'e':
				oss << GetSystemMessage( va_arg( ap, long ) );
				break;

			case 'X':
			case 'x':
				oss << uppercase << hex << va_arg( ap, int ) << dec;
				break;

			default:
				oss << *ch;
				break;
		}
	}
	va_end( ap );

	oss << ends;

	int nret = send( m_tAcceptSocket,
					 oss.str().c_str(),
					 oss.str().size(),
					 0 );						// Most often is zero, but see MSDN for other options

	if ( nret == SOCKET_ERROR )
	{
		ThrowException( "Send",
						"send() return error: " +
						 GetSystemMessage( GetError() ) );
	}
}
