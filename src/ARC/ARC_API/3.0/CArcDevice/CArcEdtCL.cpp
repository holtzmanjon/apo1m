#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <utility>
#include "CArcEdtCL.h"
#include "CArcTools.h"
#include "ArcDefs.h"

using namespace std;
using namespace arc;


//#include <fstream>
//ofstream dbgStream( "CArcEdtCL_Debug.txt" );

//#define ARC_CFG_FILE	"C:\\Users\\streit\\Documents\\Visual Studio 2008\\Projects\\ARC_API\\3.0\\CamLinkConfigFiles\\ArcDefault.cfg"


// +----------------------------------------------------------------------------
// |  Macro to swap ( reverse ) all bytes in a data word
// +----------------------------------------------------------------------------
#define SWAP( dw )	\
			( ( ( dw & 0xFF000000 ) >> 24 ) | ( ( dw & 0x00FF0000 ) >>  8 ) | \
			( ( dw & 0x0000FF00 ) <<  8 ) | ( ( dw & 0x000000FF ) << 24 ) )


// +----------------------------------------------------------------------------
// |  Macro to swap ( reverse ) all bytes in a reply word and remove the 0xAC
// |  sanity check header.
// +----------------------------------------------------------------------------
#define SWAP_REPLY( dw ) ( SWAP( dw ) & 0x00FFFFFF )


// +----------------------------------------------------------------------------
// |  Static Class Member Initialization
// +----------------------------------------------------------------------------
vector<ArcDev_t> CArcEdtCL::m_vDevList;
char** CArcEdtCL::m_pszDevList = NULL;


// +----------------------------------------------------------------------------
// |  Constructor
// +----------------------------------------------------------------------------
// |  See CArcEdtCL.h for the class definition
// +----------------------------------------------------------------------------
CArcEdtCL::CArcEdtCL()
{
	m_hDevice		= NULL;
	m_pDependent	= NULL;
	m_pszPropList	= NULL;
}

// +----------------------------------------------------------------------------
// |  Destructor
// +----------------------------------------------------------------------------
CArcEdtCL::~CArcEdtCL()
{
	if ( m_pDependent != NULL )
	{
		free( m_pDependent );
	}

	FreeDeviceStringList();

	Close();
}

// +----------------------------------------------------------------------------
// |  ToString
// +----------------------------------------------------------------------------
// |  Returns a string that represents the device controlled by this library.
// +----------------------------------------------------------------------------
const char* CArcEdtCL::ToString()
{
	return "PCIe Camera Link [ EDT PCIe8 DVa ]";
}

// +----------------------------------------------------------------------------
// |  GetCfgSpByte
// +----------------------------------------------------------------------------
// |  Returns the specified BYTE from the specified PCI configuration space
// |  register.
// |
// |  <IN> -> dOffset - The byte offset from the start of PCI config space
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
int CArcEdtCL::GetCfgSpByte( int dOffset )
{
	edt_buf tEdtBuf;

	tEdtBuf.desc = ( unsigned int )dOffset;

	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "GetCfgSpByte",
								   "Not connected to any device." );
	}

	if ( edt_ioctl( m_hDevice, EDTG_CONFIG, &tEdtBuf ) < 0 )
	{
		CArcTools::ThrowException(
						"CArcEdtCL",
						"GetCfgSpWord",
						"Reading configuration BYTE offset 0x%X failed : %s",
						 dOffset,
						 CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	}

	return int( tEdtBuf.value & 0xFF );
}

// +----------------------------------------------------------------------------
// |  GetCfgSpWord
// +----------------------------------------------------------------------------
// |  Returns the specified WORD from the specified PCI configuration space
// |  register.
// |
// |  <IN> -> dOffset - The byte offset from the start of PCI config space
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
int CArcEdtCL::GetCfgSpWord( int dOffset )
{
	edt_buf tEdtBuf;

	tEdtBuf.desc = ( unsigned int )dOffset;

	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "GetCfgSpWord",
								   "Not connected to any device." );
	}

	if ( edt_ioctl( m_hDevice, EDTG_CONFIG, &tEdtBuf ) < 0 )
	{
		CArcTools::ThrowException(
						"CArcEdtCL",
						"GetCfgSpWord",
						"Reading configuration WORD offset 0x%X failed : %s",
						 dOffset,
						 CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	}

	return int( tEdtBuf.value & 0xFFFF );
}

// +----------------------------------------------------------------------------
// |  GetCfgSpDWord
// +----------------------------------------------------------------------------
// |  Returns the specified DWORD from the specified PCI(e) configuration space
// |  register.
// |
// |  <IN> -> dOffset - The byte offset from the start of PCI(e) config space
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
int CArcEdtCL::GetCfgSpDWord( int dOffset )
{
	edt_buf tEdtBuf;

	tEdtBuf.desc = ( unsigned int )dOffset;

	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "GetCfgSpDWord",
								   "Not connected to any device." );
	}

	if ( edt_ioctl( m_hDevice, EDTG_CONFIG, &tEdtBuf ) < 0 )
	{
		CArcTools::ThrowException(
						"CArcEdtCL",
						"GetCfgSpDWord",
						"Reading configuration DWORD offset 0x%X failed : %s",
						 dOffset,
						 CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	}

	return int( tEdtBuf.value & 0xFFFFFFFF );
}

// +----------------------------------------------------------------------------
// |  SetCfgSpByte
// +----------------------------------------------------------------------------
// |  Writes the specified BYTE to the specified PCI configuration space
// |  register.
// |
// |  <IN> -> dOffset - The byte offset from the start of PCI config space
// |  <IN> -> dValue  - The BYTE value to write
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
void CArcEdtCL::SetCfgSpByte( int dOffset, int dValue )
{
	edt_buf tEdtBuf;

	tEdtBuf.desc = ( unsigned int )( dOffset & 0xFF );
	tEdtBuf.value = dValue;

	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "SetCfgSpByte",
								   "Not connected to any device." );
	}

	if ( edt_ioctl( m_hDevice, EDTS_CONFIG, &tEdtBuf ) < 0 )
	{
		CArcTools::ThrowException(
						"CArcEdtCL",
						"SetCfgSpByte",
						"Writing configuration BYTE 0x%X to offset 0x%X failed : %s",
						 dValue,
						 dOffset,
						 CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	}
}

// +----------------------------------------------------------------------------
// |  SetCfgSpWord
// +----------------------------------------------------------------------------
// |  Writes the specified WORD to the specified PCI configuration space
// |  register.
// |
// |  <IN> -> dOffset - The byte offset from the start of PCI config space
// |  <IN> -> dValue  - The WORD value to write
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
void CArcEdtCL::SetCfgSpWord( int dOffset, int dValue )
{
	edt_buf tEdtBuf;

	tEdtBuf.desc = ( unsigned int )( dOffset & 0xFFFF );
	tEdtBuf.value = dValue;

	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "SetCfgSpWord",
								   "Not connected to any device." );
	}

	if ( edt_ioctl( m_hDevice, EDTS_CONFIG, &tEdtBuf ) < 0 )
	{
		CArcTools::ThrowException(
						"CArcEdtCL",
						"SetCfgSpWord",
						"Writing configuration WORD 0x%X to offset 0x%X failed : %s",
						 dValue,
						 dOffset,
						 CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	}
}

// +----------------------------------------------------------------------------
// |  SetCfgSpDWord
// +----------------------------------------------------------------------------
// |  Writes the specified DWORD to the specified PCI configuration space
// |  register.
// |
// |  <IN> -> dOffset - The byte offset from the start of PCI config space
// |  <IN> -> dValue  - The DWORD value to write
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
void CArcEdtCL::SetCfgSpDWord( int dOffset, int dValue )
{
	edt_buf tEdtBuf;

	tEdtBuf.desc = ( unsigned int )dOffset;
	tEdtBuf.value = dValue;

	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "SetCfgSpDWord",
								   "Not connected to any device." );
	}

	if ( edt_ioctl( m_hDevice, EDTS_CONFIG, &tEdtBuf ) < 0 )
	{
		CArcTools::ThrowException(
						"CArcEdtCL",
						"SetCfgSpDWord",
						"Writing configuration DWORD 0x%X to offset 0x%X failed : %s",
						 dValue,
						 dOffset,
						 CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	}
}

// +----------------------------------------------------------------------------
// |  FindDevices
// +----------------------------------------------------------------------------
// |  Searches for available ARC, Inc camera link devices and stores the list,
// |  which can be accessed via device number ( 0,1,2... ).
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
void CArcEdtCL::FindDevices()
{
    if ( !m_vDevList.empty() )
	{
        m_vDevList.clear();

        if ( m_vDevList.size() > 0 )
		{
			CArcTools::ThrowException( "CArcEdtCL",
									   "FindDevices",
									   "Failed to free existing device list!" );
		}
	}

	EdtDev* pEdtDev = NULL;

	for ( int i=0; i<10; i++ )
	{
		if ( ( pEdtDev = edt_open_quiet( EDT_INTERFACE, i ) ) == NULL )
		{
			break;
		}

		ArcDev_t tArcDev;
		tArcDev.sName = pEdtDev->edt_devname;
		m_vDevList.push_back( tArcDev );

		edt_close( pEdtDev );
	}
}

// +----------------------------------------------------------------------------
// |  UseDevices
// +----------------------------------------------------------------------------
// |  Sets the device bindings to a specific list. Used for backward
// |  compatibility with the older astropciV1.7 driver. i.e. allows
// |  "astropci1" to be used on windows.
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
void CArcEdtCL::UseDevices( const char** pszDeviceList, int dListCount )
{
	CArcTools::ThrowException( "CArcEdtCL",
							   "UseDevices",
							   "Not used by Camera Link!" );
}

// +----------------------------------------------------------------------------
// |  DeviceCount
// +----------------------------------------------------------------------------
// |  Returns the number of items in the device list. Must be called after
// |  FindDevices().
// +----------------------------------------------------------------------------
int CArcEdtCL::DeviceCount()
{
	return m_vDevList.size();
}

// +----------------------------------------------------------------------------
// |  GetDeviceStringList
// +----------------------------------------------------------------------------
// |  Returns a string list representation of the device list. Must be called
// |  after GetDeviceList().
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
const char** CArcEdtCL::GetDeviceStringList()
{
    if ( !m_vDevList.empty() )
    {
		if ( m_pszDevList != NULL )
		{
			FreeDeviceStringList();
		}

		m_pszDevList = new char*[ m_vDevList.size() ];

		for ( string::size_type i=0; i<m_vDevList.size(); i++ )
        {
			ostringstream oss;

			oss << "EDT Camera Link "
				<< i
#ifdef WIN32
				<< ends;
#else
				<< m_vDevList.at( i ).sName
				<< ends;
#endif

			m_pszDevList[ i ] = new char[ oss.str().length() + 1 ];

			Arc_StrCopy( m_pszDevList[ i ],
						 ( oss.str().length() + 1 ),
						 oss.str().c_str() );
        }
    }
    else
    {
		string sNoDevMsg = "No Devices Found!";

		m_pszDevList = new char*[ 1 ];
		m_pszDevList[ 0 ] = new char[ sNoDevMsg.length() + 1 ];

		Arc_StrCopy( m_pszDevList[ 0 ],
					 ( sNoDevMsg.length() + 1 ),
					 sNoDevMsg.c_str() );
   }

	return ( const char ** )m_pszDevList;
}

// +----------------------------------------------------------------------------
// |  FreeDeviceStringList
// +----------------------------------------------------------------------------
// |  Frees the device string list memory. Called by destructor if user creates
// |  the list but never free's it.
// +----------------------------------------------------------------------------
void CArcEdtCL::FreeDeviceStringList()
{
	if ( m_pszDevList != NULL )
	{
		for ( int i=0; i<DeviceCount(); i++ )
		{
			delete [] m_pszDevList[ i ];
		}

		delete [] m_pszDevList;

		m_pszDevList = NULL;
	}
}

// +----------------------------------------------------------------------------
// |  IsOpen
// +----------------------------------------------------------------------------
// |  Returns 'true' if connected to a device; 'false' otherwise.
// |
// |  Throws NOTHING on error. No error handling.
// +----------------------------------------------------------------------------
bool CArcEdtCL::IsOpen()
{
	return ( ( m_hDevice != NULL ) ? true : false );
}

// +----------------------------------------------------------------------------
// |  Open
// +----------------------------------------------------------------------------
// |  Opens a connection to the device driver associated with the specified
// |  device.
// |
// |  Throws std::runtime_error on error
// |
// |  <IN>  -> dDeviceNumber - Device number
// +----------------------------------------------------------------------------
void CArcEdtCL::Open( int dDeviceNumber )
{
	if ( dDeviceNumber < 0 || dDeviceNumber > int( m_vDevList.size() ) )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "Open",
								   "Invalid device number: %d",
								    dDeviceNumber );
	}

	if ( IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "Open",
								   "Device already open, call Close() first!" );
	}

	//  Allocate the PDV dependent structure
	// +----------------------------------------------------+
	if ( ( m_pDependent = pdv_alloc_dependent() ) == NULL )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "Open",
								    CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	}

	DefaultDependent( m_pDependent );

	Arc_StrCopy( m_pDependent->camera_class,
				 sizeof( m_pDependent->camera_class ),
				 ARC_EDT_CAMERA_CLASS );

	Arc_StrCopy( m_pDependent->camera_model,
				 sizeof( m_pDependent->camera_model ),
				 ARC_EDT_CAMERA_MODEL );

	Arc_StrCopy( m_pDependent->camera_info,
				 sizeof( m_pDependent->camera_info ),
				 ARC_EDT_CAMERA_INFO );

	m_pDependent->width			= ARC_EDT_BUF_WIDTH;
	m_pDependent->height		= ARC_EDT_BUF_HEIGHT;
	m_pDependent->depth			= ARC_EDT_IMG_DEPTH;
	m_pDependent->extdepth		= ARC_EDT_IMG_DEPTH;
	m_pDependent->cl_data_path	= ARC_EDT_DATA_PATH;
	m_pDependent->serial_baud	= ARC_EDT_BAUD_RATE;
	m_pDependent->force_single	= 1;

	//  Open the device
	// +----------------------------------------------------+
	if ( ( m_hDevice = pdv_open( ( char * )EDT_INTERFACE, dDeviceNumber ) ) == NULL )
//	if ( ( m_hDevice = edt_open( EDT_INTERFACE, dDeviceNumber ) ) == NULL )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "Open",
								   "Failed to open device!" );
	}

	m_hDevice->dd_p = m_pDependent;

//	pdv_reset_serial( m_hDevice );

	edt_reg_write( m_hDevice, PDV_BRATE, 0x1 );

//	FlushSerial();


//	pdv_set_baud( m_hDevice, CArcEdtCL::BAUD_RATE );
	//if ( pdv_set_baud( m_hDevice, CArcEdtCL::BAUD_RATE ) < 0 )
	//{
	//	CArcTools::ThrowException(
	//					"CArcEdtCL",
	//					"Open",
	//					"Failed to set baud rate: %d! %s",
	//					 CArcEdtCL::BAUD_RATE,
	//					 CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	//}

	//Edtinfo tEditInfo;

	//m_pDependent = pdv_alloc_dependent();

	//if ( m_pDependent == NULL )
	//{
	//	CArcTools::ThrowException(
	//					"CArcEdtCL",
	//					"Open",
	//					"Failed to allocate PDV \"Dependent\" structure!" );
	//}

	//if ( pdv_readcfg( ARC_CFG_FILE, m_pDependent, &tEditInfo ) < 0 )
	//{
	//	CArcTools::ThrowException(
	//					"CArcEdtCL",
	//					"Open",
	//					"Failed to read configuration file: %s",
	//					 ARC_CFG_FILE );
	//}

	//if ( ( m_hDevice = pdv_open( EDT_INTERFACE, dDeviceNumber ) ) == NULL )
	//{
	//	CArcTools::ThrowException( "CArcEdtCL",
	//							   "Open",
	//							   "Failed to open device!" );
	//}

	//int dInitOk = pdv_initcam( m_hDevice,
	//						   m_pDependent,
	//						   dDeviceNumber,
	//						   &tEditInfo,
	//						   ARC_CFG_FILE,
	//						   NULL,
	//						   0 );

	//if ( dInitOk < 0 )
	//{
	//	CArcTools::ThrowException( "CArcEdtCL",
	//							   "Open",
	//							   "Failed to initialize device!" );
	//}
}

// +----------------------------------------------------------------------------
// |  Open
// +----------------------------------------------------------------------------
// |  This version first calls Open, then MapCommonBuffer if Open
// |  succeeded. Basically, this function just combines the other two.
// |
// |  Throws std::runtime_error on error
// |
// |  <IN>  -> dDeviceNumber - PCI device number
// |  <IN>  -> dBytes - The size of the kernel image buffer in bytes
// +----------------------------------------------------------------------------
void CArcEdtCL::Open( int dDeviceNumber, int dBytes )
{
	try
	{
		Open( dDeviceNumber );
		MapCommonBuffer( dBytes );
	}
	catch ( ... )
	{
		throw;
	}
}

// +----------------------------------------------------------------------------
// |  Close
// +----------------------------------------------------------------------------
// |  Closes the currently open driver that was opened with a call to
// |  Open.
// |
// |  Throws NOTHING on error. No error handling.
// +----------------------------------------------------------------------------
void CArcEdtCL::Close()
{
	//
	// Prevents access violation from code that follows
	//
	bool bOldStoreCmds = m_bStoreCmds;
	m_bStoreCmds       = false;

	FreeProperties();

	UnMapCommonBuffer();

	if ( m_hDevice != NULL )
	{
//		edt_close( m_hDevice );
		pdv_close( m_hDevice );
	}

	m_dCCParam   = 0;
 	m_hDevice    = NULL;
	m_pDependent = NULL;
	m_bStoreCmds = bOldStoreCmds;
}

// +----------------------------------------------------------------------------
// |  Reset
// +----------------------------------------------------------------------------
// |  Resets the camera link board.
// |
// |  Throws NOTHING to prevent Owl complications. No error handling.
// +----------------------------------------------------------------------------
void CArcEdtCL::Reset()
{
}

// +----------------------------------------------------------------------------
// |  MapCommonBuffer
// +----------------------------------------------------------------------------
// |  Map the device driver image buffer.
// |
// |  Throws std::runtime_error on error
// |
// |  <IN>  -> bBytes - The number of bytes to map as an image buffer. Not
// |                    used by camera link.
// +----------------------------------------------------------------------------
void CArcEdtCL::MapCommonBuffer( int dBytes )
{
	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "MapCommonBuffer",
								   "Not connected to any device." );
	}

	//	This call releases any previously allocated buffers
	// +--------------------------------------------------------+
	if ( edt_configure_ring_buffers( m_hDevice, dBytes, 1, EDT_READ, NULL ) < 0 )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "MapCommonBuffer",
								   "Failed to configure buffers!" );
	}

	int dBufSize = edt_allocated_size( m_hDevice, 0 );

	if ( dBufSize < dBytes )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "MapCommonBuffer",
								   "Expected buffer size ( %d ) doesn't match set size ( %d )",
									dBytes,
									dBufSize );
	}

	unsigned char** pBufAddr = edt_buffer_addresses( m_hDevice );

	Arc_ZeroMemory( &m_tImgBuffer, sizeof( ImgBuf_t ) );

	m_tImgBuffer.pUserAddr		= ( void * )pBufAddr[ 0 ];
	m_tImgBuffer.dSize			= dBufSize;
	m_tImgBuffer.ulPhysicalAddr	= 0;

	//int dBufSize = pdv_image_size( m_hDevice );

	//if ( dBufSize != dBytes )
	//{
	//	CArcTools::ThrowException( "CArcEdtCL",
	//							   "MapCommonBuffer",
	//							   "Expected buffer size ( %d ) doesn't match set size ( %d )",
	//								dBytes,
	//								dBufSize );
	//}

	////unsigned char* pBufAddr[ 1 ] = { NULL };

	////pBufAddr[ 0 ] = pdv_alloc( dBufSize );

	////if ( pBufAddr[ 0 ] == NULL )
	////{
	////	CArcTools::ThrowException( "CArcEdtCL",
	////							   "MapCommonBuffer",
	////							   "Failed to ALLOCATE image buffers!" );
	////}

	////if ( pdv_set_buffers( m_hDevice, 1, pBufAddr ) < 0 )
	////{
	////	CArcTools::ThrowException( "CArcEdtCL",
	////							   "MapCommonBuffer",
	////							   "Failed to SET image buffers!" );
	////}

	////m_tImgBuffer.pUserAddr		= ( void * )pBufAddr[ 0 ];
	////m_tImgBuffer.dSize			= dBufSize;
	////m_tImgBuffer.ulPhysicalAddr	= 0;

	//if ( pdv_multibuf( m_hDevice, 1 ) < 0 )
	//{
	//	CArcTools::ThrowException( "CArcEdtCL",
	//							   "MapCommonBuffer",
	//							   "Failed to allocate image buffers!" );
	//}

	//unsigned char** pBufAddr = pdv_buffer_addresses( m_hDevice );

	//m_tImgBuffer.pUserAddr		= ( void * )pBufAddr[ 0 ];
	//m_tImgBuffer.dSize			= dBufSize;
	//m_tImgBuffer.ulPhysicalAddr	= 0;
}

// +----------------------------------------------------------------------------
// |  UnMapCommonBuffer
// +----------------------------------------------------------------------------
// |  Un-Maps the device driver image buffer.
// |
// |  Throws NOTHING
// +----------------------------------------------------------------------------
void CArcEdtCL::UnMapCommonBuffer()
{
	if ( m_hDevice != NULL )
	{
		edt_disable_ring_buffers( m_hDevice );
	}

	Arc_ZeroMemory( &m_tImgBuffer, sizeof( ImgBuf_t ) );
}

// +----------------------------------------------------------------------------
// |  GetCommonBufferProperties
// +----------------------------------------------------------------------------
// |  Fills in the image buffer structure with its properties, such as
// |  physical address and size.
// |
// |  Throws NOTHING on error. No error handling.
// +----------------------------------------------------------------------------
bool CArcEdtCL::GetCommonBufferProperties()
{

	if ( m_tImgBuffer.dSize <= 0 || m_tImgBuffer.pUserAddr == NULL )
	{
		return false;
	}

	return true;
}

// +----------------------------------------------------------------------------
// |  GetId
// +----------------------------------------------------------------------------
// |  Returns the camera link board id
// +----------------------------------------------------------------------------
int CArcEdtCL::GetId()
{
	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "GetId",
								   "Not connected to any device." );
	}

	return edt_device_id( m_hDevice );
}

// +----------------------------------------------------------------------------
// |  GetStatus
// +----------------------------------------------------------------------------
// |  Not used for camera link
// +----------------------------------------------------------------------------
int CArcEdtCL::GetStatus()
{
	CArcTools::ThrowException( "CArcEdtCL",
							   "GetStatus",
							   "Method not available for camera link device!" );

	return 0;
}

// +----------------------------------------------------------------------------
// |  ClearStatus
// +----------------------------------------------------------------------------
// |  Not used for camera link
// +----------------------------------------------------------------------------
void CArcEdtCL::ClearStatus()
{
	CArcTools::ThrowException( "CArcEdtCL",
							   "ClearStatus",
							   "Method not available for camera link device!" );
}

// +----------------------------------------------------------------------------
// |  Set2xFOTransmitter
// +----------------------------------------------------------------------------
// |  Not used for camera link
// |
// |  Throws std::runtime_error on error
// |
// |  <IN> -> bOnOff - True to enable dual transmitters; false otherwise.
// +----------------------------------------------------------------------------
void CArcEdtCL::Set2xFOTransmitter( bool bOnOff )
{
	CArcTools::ThrowException( "CArcEdtCL",
							   "Set2xFOTransmitter",
							   "Method not available for camera link device!" );
}

// +----------------------------------------------------------------------------
// |  LoadDeviceFile
// +----------------------------------------------------------------------------
// |  Not used by camera link - BUT could be used to load configuration file!
// +----------------------------------------------------------------------------
void CArcEdtCL::LoadDeviceFile( const char* pszFile )
{
	CArcTools::ThrowException( "CArcEdtCL",
							   "LoadDeviceFile",
							   "Method not available for camera link device!" );
}

// +----------------------------------------------------------------------------
// |  GetProperties
// +----------------------------------------------------------------------------
// |  Used to return any device specific properties. All properties are returned
// |  as strings regardless of the actual property data type. The returned array
// |  is of the following form:
// |
// |  array[ N ][ 0 ] = Property Name
// |  array[ N ][ 1 ] = Property Value
// |
// |  Where N is the total number of properties returned by the PropertyCount()
// |  method.
// |
// |  Returns NULL if no properties are available.
// +----------------------------------------------------------------------------
const char*** CArcEdtCL::GetProperties()
{
	if ( m_mPropList.empty() )
	{
		ostringstream oss;

		oss << m_hDevice->dd_p->camera_class << ends;
		m_mPropList.insert(
					pair<string,string>( "Camera Class", oss.str() ) );
		oss.str( "" );

		oss << m_hDevice->dd_p->camera_model << ends;
		m_mPropList.insert(
					pair<string,string>( "Camera Model", oss.str() ) );
		oss.str( "" );

		oss << m_hDevice->dd_p->camera_info << ends;
		m_mPropList.insert(
					pair<string,string>( "Camera Info", oss.str() ) );
		oss.str( "" );

		oss << m_hDevice->dd_p->width << ends;
		m_mPropList.insert(
					pair<string,string>( "Buffer Width", oss.str() ) );
		oss.str( "" );

		oss << m_hDevice->dd_p->height << ends;
		m_mPropList.insert(
					pair<string,string>( "Buffer Height", oss.str() ) );
		oss.str( "" );

		oss << m_hDevice->dd_p->depth << ends;
		m_mPropList.insert(
					pair<string,string>( "Buffer Depth", oss.str() ) );
		oss.str( "" );

		oss << m_hDevice->dd_p->extdepth << ends;
		m_mPropList.insert(
					pair<string,string>( "External Depth", oss.str() ) );
		oss.str( "" );

		oss << "0x" << hex << m_hDevice->dd_p->cl_data_path << dec << ends;
		m_mPropList.insert(
					pair<string,string>( "Camera Link Data Path", oss.str() ) );
		oss.str( "" );

		oss << m_hDevice->dd_p->serial_baud << ends;
		m_mPropList.insert(
					pair<string,string>( "Serial Baud Rate", oss.str() ) );
		oss.str( "" );
	}

	map<string,string>::iterator it;

	int i = 0;

	m_pszPropList = new char**[ m_mPropList.size() ];

	for ( it=m_mPropList.begin(), i=0; it != m_mPropList.end(); it++, i++ )
	{
		m_pszPropList[ i ] = new char*[ 2 ];

		m_pszPropList[ i ][ 0 ] = new char[ ( *it ).first.length() + 1 ];

		Arc_StrCopy( m_pszPropList[ i ][ 0 ],
					 ( ( *it ).first.length() + 1 ),
					 ( *it ).first.c_str() );

		m_pszPropList[ i ][ 1 ] = new char[ ( *it ).second.length() + 1 ];

		Arc_StrCopy( m_pszPropList[ i ][ 1 ],
				 	 ( ( *it ).second.length() + 1 ),
					 ( *it ).second.c_str() );
	}

	return ( const char *** )m_pszPropList;
}

// +----------------------------------------------------------------------------
// |  FreeProperties
// +----------------------------------------------------------------------------
// |  Frees any property arrays returned to the caller of GetProperties().
// +----------------------------------------------------------------------------
void CArcEdtCL::FreeProperties()
{
	if ( m_pszPropList != NULL )
	{
		for ( int i=0; i<PropertyCount(); i++ )
		{
			delete [] m_pszPropList[ i ][ 0 ];
			delete [] m_pszPropList[ i ][ 1 ];
			delete [] m_pszPropList[ i ];
		}

		delete [] m_pszPropList;

		m_pszPropList = NULL;
	}
}

// +----------------------------------------------------------------------------
// |  PropertyCount
// +----------------------------------------------------------------------------
// |  Returns the number of properties.
// +----------------------------------------------------------------------------
int CArcEdtCL::PropertyCount()
{
	return int( m_mPropList.size() );
}

// +----------------------------------------------------------------------------
// |  Command
// +----------------------------------------------------------------------------
// |  Send a command to the controller timing or utility board. Returns the
// |  controller reply, typically 'DON'.
// |
// |  Throws std::runtime_error on error
// |
// |  <IN>  -> dBoardId       - Command board id ( TIM or UTIL )
// |  <IN>  -> dCommand       - Board command
// |  <IN>  -> dArg1 to dArg4 - Command arguments ( optional )
// +----------------------------------------------------------------------------
int CArcEdtCL::Command( int dBoardId, int dCommand, int dArg0, int dArg1, int dArg2, int dArg3 )
{
	int dNumOfArgs = 2;
	int dHeader    = 0;
	int dReply     = 0;

	enum { HDR = 0, CMD, ARG0, ARG1, ARG2, ARG3 };
	int dCmdBuf[]  = { 0, 0, -1, -1, -1, -1 };

	if ( dBoardId != TIM_ID )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "Command",
								   "Only TIMING board commands are valid!" );
	}

	//
	//  Calculate the number of arguments
	// +-------------------------------------------------+
	if ( dArg0 >= 0 ) { dNumOfArgs++; }
	if ( dArg1 >= 0 ) { dNumOfArgs++; }
	if ( dArg2 >= 0 ) { dNumOfArgs++; }
	if ( dArg3 >= 0 ) { dNumOfArgs++; }

	try
	{
		//
		//  Send Header
		// +-------------------------------------------------+
		dHeader = ( ( dBoardId << 8 ) | dNumOfArgs );

		dCmdBuf[ HDR ] = SWAP( ( 0xAC000000 | dHeader ) );

		//
		//  Send Command
		// +-------------------------------------------------+
		dCmdBuf[ CMD ] = SWAP( ( 0xAC000000 | dCommand ) );

		//
		//  Send Arguments
		// +-------------------------------------------------+
		if ( dArg0 >= 0 )
		{
			dCmdBuf[ ARG0 ] = SWAP( ( 0xAC000000 | dArg0 ) );
		}

		if ( dArg1 >= 0 )
		{
			dCmdBuf[ ARG1 ] = SWAP( ( 0xAC000000 | dArg1 ) );
		}

		if ( dArg2 >= 0 )
		{
			dCmdBuf[ ARG2 ] = SWAP( ( 0xAC000000 | dArg2 ) );
		}

		if ( dArg3 >= 0 )
		{
			dCmdBuf[ ARG3 ] = SWAP( ( 0xAC000000 | dArg3 ) );
		}

		WriteSerialBytes( &dCmdBuf[ 0 ], ( dNumOfArgs * sizeof( int ) ) );

		////
		////  Send Header
		//// +-------------------------------------------------+
		//dHeader = ( ( dBoardId << 8 ) | dNumOfArgs );

		//WriteSerialBytes( ( 0xAC000000 | dHeader ) );

		////
		////  Send Command
		//// +-------------------------------------------------+
		//WriteSerialBytes( ( 0xAC000000 | dCommand ) );

		////
		////  Send Arguments
		//// +-------------------------------------------------+
		//if ( dArg0 >= 0 )
		//{
		//	WriteSerialBytes( ( 0xAC000000 | dArg0 ) );
		//}

		//if ( dArg1 >= 0 )
		//{
		//	WriteSerialBytes( ( 0xAC000000 | dArg1 ) );
		//}

		//if ( dArg2 >= 0 )
		//{
		//	WriteSerialBytes( ( 0xAC000000 | dArg2 ) );
		//}

		//if ( dArg3 >= 0 )
		//{
		//	WriteSerialBytes( ( 0xAC000000 | dArg3 ) );
		//}
	}
	catch ( ... )
	{
		if ( m_bStoreCmds )
		{
			m_cApiLog.Put(
					CArcTools::CmdToString( dReply,
											dBoardId,
				   							dCommand,
											dArg0,
											dArg1,
											dArg2,
											dArg3 ).c_str() );
		}

		throw;
	}

	//
	//  Return the reply
	// +-------------------------------------------------+
	try
	{
		dReply = ReadReply();
	}
	catch ( exception& e )
	{
		if ( m_bStoreCmds )
		{
			m_cApiLog.Put(
					CArcTools::CmdToString( dReply,
											dBoardId,
				   							dCommand,
											dArg0,
											dArg1,
											dArg2,
											dArg3 ).c_str() );
		}

		ostringstream oss;

		oss << e.what() << endl
			<< "Exception Details: 0x"
			<< hex << dHeader << dec << " "
			<< CArcTools::CmdToString( dCommand ) << " "
			<< "0x" << hex << dArg0 << dec << " "
			<< "0x" << hex << dArg1 << dec << " "
			<< "0x" << hex << dArg2 << dec << " "
			<< "0x" << hex << dArg3 << dec << endl;

		CArcTools::ThrowException( "CArcEdtCL", "Command", oss.str() );
	}

	//
	// Set the debug message queue.
	//
	if ( m_bStoreCmds )
	{
		m_cApiLog.Put(
				CArcTools::CmdToString( dReply,
										dBoardId,
				   						dCommand,
										dArg0,
										dArg1,
										dArg2,
										dArg3 ).c_str() );
	}

	if ( dReply == CNR )
	{
		CArcTools::ThrowException(
						"CArcEdtCL",
						"Command",
						"Controller not ready! Verify controller has been setup! Reply: 0x%X",
						 dReply );
	}

    return dReply;
}

// +----------------------------------------------------------------------------
// |  GetControllerId
// +----------------------------------------------------------------------------
// |  Returns the controller ID or 'ERR' if none.
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
int CArcEdtCL::GetControllerId()
{
	return ERR;
}

// +----------------------------------------------------------------------------
// |  ResetController
// +----------------------------------------------------------------------------
// |  Resets the controller.
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
void CArcEdtCL::ResetController()
{
	CArcTools::ThrowException( "CArcEdtCL",
							   "ResetController",
					           "This method not yet implemented!" );
}

// +----------------------------------------------------------------------------
// | IsControllerConnected
// +----------------------------------------------------------------------------
// |  Returns 'true' if a controller is connected to the camera link board.
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
bool CArcEdtCL::IsControllerConnected()
{
	return true;
}

// +----------------------------------------------------------------------------
// |  LoadControllerFile
// +----------------------------------------------------------------------------
// |  Loads a FastCam timing or utility file (.lod) into the board DSP.
// |
// |  Throws std::runtime_error on error
// |
// |  <IN> -> pszFilename - The SMALLCAM or GENI/II/III TIM or UTIL lod file to load.
// |  <IN> -> bValidate   - Set to 'true' if the download should be read back and
// |                        checked after every write.
// |  <IN> -> bAbort      - 'true' to stop; 'false' otherwise. Default: false
// +----------------------------------------------------------------------------
void CArcEdtCL::LoadControllerFile( const char *pszFilename, bool bValidate, const bool& bAbort )
{
	int dAddr	= 0;
	int dData	= 0;

	std::string sFilename( pszFilename );
	std::string sLine;
	std::string sToken;
	CArcTools::CTokenizer cTokenizer;

	std::queue<int> qData;

	//
	// Verify device connection
	// -------------------------------------------------------------------
	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcDevice",
								   "LoadControllerFile",
								   "Not connected to any device." );
	}

	if ( bAbort ) { return; }

	//
	// Open the file for reading.
	// -------------------------------------------------------------------
	ifstream inFile( sFilename.c_str() );

	if ( !inFile.is_open() )
	{
		CArcTools::ThrowException( "CArcDevice",
								   "LoadControllerFile",
								   "Cannot open file: %s",
								    sFilename.c_str() );
	}

	if ( bAbort ) { inFile.close(); return; }

	//
	// Read in the file one line at a time
	// --------------------------------------
	while ( !inFile.eof() )
	{
		if ( bAbort ) { inFile.close(); return; }

		getline( inFile, sLine );

		//
		// Only "_DATA" blocks are valid for download
		// ---------------------------------------------
		if ( sLine.find( "_DATA " ) != std::string::npos )
		{
			cTokenizer.Victim( sLine );
			cTokenizer.Next();	// Dump _DATA string

			//
			// Get the memory type and start address
			// ---------------------------------------------
			cTokenizer.Next();	// Don't need to know memory type
			dAddr = CArcTools::StringToHex( cTokenizer.Next() );

			//
			// The start address must be less than MAX_DSP_START_LOAD_ADDR
			// -------------------------------------------------------------
			if ( dAddr < MAX_DSP_START_LOAD_ADDR )
			{
				//
				// Read the data block and store it in a Q
				// ----------------------------------------
				while ( !inFile.eof() && inFile.peek() != '_' )
				{
					if ( bAbort ) { inFile.close(); return; }

					getline( inFile, sLine );
					cTokenizer.Victim( sLine );

					while ( !( ( sToken = cTokenizer.Next() ).empty() ) )
					{
						if ( bAbort ) { inFile.close(); return; }

						dData = CArcTools::StringToHex( sToken );

						qData.push( dData );

						dAddr++;

					} // while tokenizer next
				} // while not EOF or '_'
			}	// if address < 0x4000
		}	// if not '_DATA'
	}	// while not EOF

	if ( qData.size() > MAX_DSP_FILE_SIZE )
	{
		int dQSize = int( qData.size() );

		while ( !qData.empty() )
		{
			qData.pop();
		}

		CArcTools::ThrowException(
						"CArcEdtCL",
						"LoadControllerFile",
						"Timing file too large [ %d bytes ] to download. Must be less than %d bytes",
						dQSize,
						MAX_DSP_FILE_SIZE );
	}

	int dBytes = qData.size() * 3;	// Data is 24-bits

	arc::ubyte* pU8Data = new arc::ubyte[ dBytes ];
	arc::ubyte* pU8Next = pU8Data;

	while ( !qData.empty() )
	{
		*pU8Next = ( ( qData.front() & 0xFF0000 ) >> 16 ); pU8Next++;
		*pU8Next = ( ( qData.front() & 0x00FF00 ) >>  8 ); pU8Next++;
		*pU8Next = ( ( qData.front() & 0x0000FF ) >>  0 ); pU8Next++;

		qData.pop();
	}

//	pdv_set_serial_block_size( 2048 );

//	clock_t begin = clock();

	//
	// Tell the controller FPGA how much data to expect
	//
	try
	{
		WriteSerialByte( DSP_HEADER_BYTE );
		WriteSerialByte( ( ( dBytes & 0xFF00 ) >> 8 ) );
		WriteSerialByte( ( dBytes & 0x00FF ) );

		WriteSerialBytes( pU8Data, dBytes );
	}
	catch ( ... )
	{
		if ( pU8Data != NULL )
		{
			delete[] pU8Data;
		}

		throw;
	}

	//clock_t end = clock();
	//double diffticks = end - begin;
	//double diffms = ( diffticks * 1000 ) / CLOCKS_PER_SEC;
	//double diffs  = diffticks / CLOCKS_PER_SEC;
	//cout << "Time elapsed: " << double( diffms ) << " ms " << diffs << " sec" << endl;

//	int dCount = 0;
//
//	//
//	// Write the data from the Q
//	// ----------------------------------------
//	while ( !qData.empty() )
//	{
//		// Write the data to the controller
//		// --------------------------------------------------------------
////		WriteSerialBytes( qData.front() );
//
//		WriteSerialByte( ( ( qData.front() & 0xFF0000 ) >> 16 ) );
//		WriteSerialByte( ( ( qData.front() & 0x00FF00 ) >>  8 ) );
//		WriteSerialByte( ( ( qData.front() & 0x0000FF ) >>  0 ) );
//
//		cout << "WRITE: " << uppercase << hex << qData.front() << dec << "\t" << dCount << " / " <<	qData.size() << endl;
//
//		qData.pop();
//
//		dCount++;
//	}

	//
	// Wait 10ms. The controller DSP takes 5ms to start processing
	// commands after the download is complete.  This time was
	// measured using the logic analyzer on a PCIe <-> SmallCam
	// system on Nov 16, 2011 at 12:50pm by Bob and Yoating.
	//
	// Without this delay the RCC command that follows will cause
	// problems that result in the DSP not responding to any further
	// commands.

	if ( bAbort ) { return; }

	//
	// Auto-get CC Params after setup
	//
	// Loop N number of times while waiting for the GetCCParams
	// method to return a value other than CNR.  Any value other
	// than CNR will cause the code to continue on.  This is
	// primarily for use on a PCIe <-> SmallCam system.
	//int dCCParams = CNR;
	//int dTryCount = 0;

	//while ( dCCParams == CNR && dTryCount < 500 )
	//{
	//	if ( bAbort ) { break; }

	//	try
	//	{
	//		dCCParams = GetCCParams();
	//	}
	//	catch ( ... )
	//	{
	//	}

	//	dTryCount++;

	//	Arc_Sleep( 100 );
	//}
}

// +----------------------------------------------------------------------------
// |  StopExposure
// +----------------------------------------------------------------------------
// |  Stops the current exposure.
// |
// |  NOTE: The command is sent manually and NOT using the Command() method
// |        because the Command() method checks for readout and fails.
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
void CArcEdtCL::StopExposure()
{
	//if ( !IsOpen() )
	//{
	//	CArcTools::ThrowException( "CArcEdtCL",
	//							   "StopExposure",
	//							   "Not connected to any device." );
	//}

	//
	//  Send Header
	// +-------------------------------------------------+
	int dHeader = ( ( TIM_ID << 8 ) | 2 );

	WriteSerialBytes( ( 0xAC000000 | dHeader ) );

	//
	//  Send Command
	// +-------------------------------------------------+
	WriteSerialBytes( ( 0xAC000000 | ABR ) );

	//
	//  Read the reply
	// +-------------------------------------------------+
	int dReply = ReadReply();

	if ( dReply != DON )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "StopExposure",
								   "Failed to stop exposure/readout, reply: 0x%X",
									dReply );
	}

	if ( edt_abort_dma( m_hDevice ) < 0 )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "StopExposure",
								   "Failed to stop camera link read!" );
	}
}

// +----------------------------------------------------------------------------
// |  IsReadout
// +----------------------------------------------------------------------------
// |  Returns 'true' if the controller is in readout.
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
bool CArcEdtCL::IsReadout()
{
	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "IsReadout",
								   "Not connected to any device." );
	}

	unsigned char* pBuf = edt_check_for_buffers( m_hDevice, 1 );

	return ( pBuf == NULL ? true : false );
}

// +----------------------------------------------------------------------------
// |  GetPixelCount
// +----------------------------------------------------------------------------
// |  Returns the current pixel count.
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
int CArcEdtCL::GetPixelCount()
{
	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "GetPixelCount",
								   "Not connected to any device." );
	}

	int dPixCnt =
			int( edt_get_bytecount( m_hDevice ) / sizeof( unsigned short ) );

	if ( m_bStoreCmds )
	{
		m_cApiLog.Put(
			CArcTools::FormatString( "[ PIXEL COUNT -> %d ]", dPixCnt ).c_str() );
	}

	return dPixCnt;
}

// +----------------------------------------------------------------------------
// |  GetCRPixelCount
// +----------------------------------------------------------------------------
// |  Returns the cumulative pixel count as when doing continuous readout.
// |  This method is used by user applications when reading super dArge images
// | ( greater than 4K x 4K ). This value increases across all frames being
// |  readout. This code needs to execute fast, so not pre-checking, such as 
// |  IsControllerConnected() is done.
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
int CArcEdtCL::GetCRPixelCount()
{
	CArcTools::ThrowException( "CArcEdtCL",
							   "GetCRPixelCount",
							   "Method not supported by camera link!" );
	return 0;
}

// +----------------------------------------------------------------------------
// |  GetFrameCount
// +----------------------------------------------------------------------------
// |  Returns the current frame count. The camera MUST be set for continuous
// |  readout for this to work. This code needs to execute fast, so no 
// |  pre-checking, such as IsControllerConnected() is done.
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
int CArcEdtCL::GetFrameCount()
{
	CArcTools::ThrowException( "CArcEdtCL",
							   "GetFrameCount",
							   "Method not supported by camera link!" );
	return 0;
}

// +----------------------------------------------------------------------------
// |  GetBaudRate
// +----------------------------------------------------------------------------
// |  Returns the current serial baud rate or 0 on error.  The baud rate is not
// |  a user settable property.  This method only exists for info/debug
// |  purposes.
// |
// |  Throws std::runtime_error on error
// +----------------------------------------------------------------------------
int CArcEdtCL::GetBaudRate()
{
	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "GetBaudRate",
								   "Not connected to any device." );
	}

	return pdv_get_baud( m_hDevice );
}

// +----------------------------------------------------------------------------
// |  WriteSerialByte
// +----------------------------------------------------------------------------
// |  Send a single byte to the controller.  Uses the EDT library function 
// |  'pdv_serial_write()'.
// |
// |  Throws std::runtime_error on error
// |
// |  <IN> -> zByte - The data byte to send to the controller
// +----------------------------------------------------------------------------
void CArcEdtCL::WriteSerialByte( unsigned char zByte )
{
	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "WriteSerialByte",
								   "Not connected to any device." );
	}

	edt_send_msg( m_hDevice, 0, ( const char * )&zByte, sizeof( unsigned char ) );

	//FlushSerial();

	//int dSuccess = pdv_serial_write( m_hDevice,
	//								( const char * )&zByte,
	//								 sizeof( unsigned char ) );

	//if ( dSuccess < 0 )
	//{
	//	CArcTools::ThrowException(
	//					"CArcEdtCL",
	//					"WriteSerialByte",
	//					"Failed to send: 0x%X : %s",
	//					zByte,
	//					CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	//}
}

// +----------------------------------------------------------------------------
// |  WriteSerialBytes
// +----------------------------------------------------------------------------
// |  Send a 4-byte value ( integer ) to the controller one byte at a time.
// |  Uses the EDT library function 'pdv_serial_write()'.
// |
// |  Throws std::runtime_error on error
// |
// |  <IN>  -> dWord - The data word to send to the controller
// +----------------------------------------------------------------------------
void CArcEdtCL::WriteSerialBytes( int dWord )
{
	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "WriteSerialBytes",
								   "Not connected to any device." );
	}

	int dSwapped = SWAP( dWord );

	ubyte* pU8Buf	= ( ubyte * )&dSwapped;
	//int    dCount	= sizeof( int );
	//int    dTimeOut	= 1000;

	//for ( ;; )
	//{
	//	int dStatus = edt_reg_read( m_hDevice, PDV_SERIAL_DATA_STAT );

	//	if ( ( dStatus & 0x2 ) )	// TRANSMIT_READY
	//	{
	//		edt_reg_write( m_hDevice, PDV_SERIAL_DATA, *pU8Buf );

	//		pU8Buf++;
	//		dCount--;
	//	}

	//	if ( dCount <= 0 ) { break; }

	//	if ( dTimeOut <= 0 ) { break; } dTimeOut--;
	//}

	//if ( dTimeOut <= 0 )
	//{
	//	CArcTools::ThrowException(
	//						"CArcEdtCL",
	//						"WriteSerialBytes",
	//						"Failed to send: 0x%X : TIMEOUT!",
	//						 dWord );
	//}

	edt_send_msg( m_hDevice, 0, ( const char * )pU8Buf, sizeof( int ) );

	//if ( !IsOpen() )
	//{
	//	CArcTools::ThrowException( "CArcEdtCL",
	//							   "WriteSerialBytes",
	//							   "Not connected to any device." );
	//}

	//FlushSerial();

	//int dSwappedWord = SWAP( dWord );

	//int dSuccess = pdv_serial_write( m_hDevice,
	//								( const char * )&dSwappedWord,
	//								 sizeof( int ) );

	//if ( dSuccess < 0 )
	//{
	//	CArcTools::ThrowException(
	//					"CArcEdtCL",
	//					"WriteSerialBytes",
	//					"Failed to send: 0x%X : %s",
	//					dWord,
	//					CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	//}
}

// +----------------------------------------------------------------------------
// |  WriteSerialBytes
// +----------------------------------------------------------------------------
// |  Sends a block of byte data to the controller one byte at a time. Uses the
// |  EDT library function 'pdv_serial_write()'.  Generally used to send a
// |  command ( header, cmd, arg0, ... ) to the controller.
// |
// |  Throws std::runtime_error on error
// |
// |  <IN> -> pBuffer - The data byte buffer to write.
// |  <IN> -> dSize   - The number of bytes to write.
// +----------------------------------------------------------------------------
void CArcEdtCL::WriteSerialBytes( void* pBuffer, int dSize )
{
	if ( !IsOpen() )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "WriteSerialBytes",
								   "Not connected to any device." );
	}

	if ( pBuffer == NULL || dSize <= 0 )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "WriteSerialBytes",
								   "NULL buffer or invalid data size!" );
	}

	ubyte* pU8Buf	= ( ubyte * )pBuffer;
	int    dCount	= dSize;
	int    dTimeOut	= 100000;
	int    dStatus  = 0;

	FlushSerial();

	for ( ;; )
	{
		dStatus = edt_reg_read( m_hDevice, PDV_SERIAL_DATA_STAT );

		if ( ( dStatus & 0x2 ) )	// TRANSMIT_READY
		{
			edt_reg_write( m_hDevice, PDV_SERIAL_DATA, *pU8Buf );

			pU8Buf++;
			dCount--;
		}

		if ( dCount <= 0 ) { break; }

		if ( dTimeOut <= 0 ) { break; } dTimeOut--;
	}

	if ( dTimeOut <= 0 )
	{
		CArcTools::ThrowException(
							"CArcEdtCL",
							"WriteSerialBytes",
							"Failed to send data block : TIMEOUT! Status: 0x%X",
							 dStatus );
	}

	//FlushSerial();

	//int dSuccess = pdv_serial_write( m_hDevice,
	//								( const char * )pBuffer,
	//								 dSize );

	//if ( dSuccess != 0 )
	//{
	//	CArcTools::ThrowException(
	//					"CArcEdtCL",
	//					"WriteSerialBytes",
	//					"Failed to send data block : %s",
	//					CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	//}
}

// +----------------------------------------------------------------------------
// |  ReadReply
// +----------------------------------------------------------------------------
// |  Reads the reply register value. This method will time-out if the
// |  specified number of seconds passes before the reply is received.
// |
// |  Throws std::runtime_error on error
// |
// |  <IN> -> gTimeOutSecs - Seconds to wait before time-out occurs.
// +----------------------------------------------------------------------------
int CArcEdtCL::ReadReply( double gTimeOutSecs )
{
	enum { HDR = 0, REPLY, REPLY_SIZE };
	int dReply[ REPLY_SIZE ] = { 0 };
	int dBytesRcvd = 0;
	//int dTimeout = 0;

	const int REPLY_BYTE_COUNT = ( REPLY_SIZE * sizeof( int ) );

	for ( int i=0; i<5; i++ )
	{
		dBytesRcvd = pdv_serial_wait( m_hDevice,
									  int( gTimeOutSecs * 1000 ),
									  ( REPLY_SIZE * sizeof( int ) ) );

		if ( dBytesRcvd >= REPLY_BYTE_COUNT )
		{
			break;
		}
	}

	if ( dBytesRcvd < REPLY_BYTE_COUNT )
	{
		CArcTools::ThrowException(
						"CArcEdtCL",
						"ReadReply",
						"Timeout occurred while waiting for reply! Read: %d Expected: %d",
						 dBytesRcvd,
						 REPLY_BYTE_COUNT );
	}

	dBytesRcvd = pdv_serial_read( m_hDevice,
								  ( char * )&dReply,
								  REPLY_BYTE_COUNT );

	//cout << "BYTES RCVD -> " << dBytesRcvd << endl;

	//cout << "ReadReply Reply Header -> 0x" << hex << dReply[ HDR ]
	//     << dec << " Reply: 0x" << hex << SWAP_REPLY( dReply[ REPLY ] ) << dec << endl;

	if ( dBytesRcvd <= 0 )
	{
		CArcTools::ThrowException(
						"CArcEdtCL",
						"ReadReply",
						"Failed to read reply : %s",
						CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	}

	return SWAP_REPLY( dReply[ REPLY ] );
}


// +----------------------------------------------------------------------------
// |  DefaultDependent
// +----------------------------------------------------------------------------
// |  CRITICAL - This method is required for the EDT board to be setup properly.
// |  This code is copied directly from "readcfg.c" and must be called. If this
// |  is not called, then a config file must be used in conjunction with the
// |  readcfg() and initcam() functions.
// |
// |  Throws std::runtime_error on error
// |
// |  <IN> -> dd_p - Dependent structure to be filled with default values.
// +----------------------------------------------------------------------------
void CArcEdtCL::DefaultDependent( PdvDependent* dd_p )
{
	//  Clear dependent struct and set defaults to nonsense values
	// +-----------------------------------------------------------+
    Arc_ZeroMemory( dd_p, sizeof( Dependent ) );

    dd_p->rbtfile[ 0 ]			= '\0';
    dd_p->cameratype[ 0 ]		= '\0';
    dd_p->shutter_speed			= NOT_SET;
    dd_p->default_shutter_speed	= NOT_SET;
    dd_p->default_gain			= NOT_SET;
    dd_p->default_offset		= NOT_SET;
    dd_p->default_aperture		= NOT_SET;
    dd_p->binx					= 1;
    dd_p->biny					= 1;
    dd_p->byteswap				= NOT_SET;
    dd_p->serial_timeout		= 1000;
    dd_p->serial_response[ 0 ]	= '\r';
    dd_p->xilinx_rev			= NOT_SET;
    dd_p->timeout				= NOT_SET;
    dd_p->user_timeout			= NOT_SET;
    dd_p->mode_cntl_norm		= NOT_SET;
    dd_p->mc4					= 0;
    dd_p->pulnix				= 0;
    dd_p->dbl_trig				= 0;
    dd_p->shift					= NOT_SET;
    dd_p->mask					= 0xffff;
    dd_p->mode16				= NOT_SET;
	dd_p->serial_baud			= NOT_SET;
    dd_p->serial_waitc			= NOT_SET ;
    dd_p->serial_format			= SERIAL_ASCII;

	Arc_StrCopy( dd_p->serial_term,
				 sizeof( dd_p->serial_term ),
				 "\r" );	// Term for most ASCII exc. ES4.0

	dd_p->kbs_red_row_first		= 1;
    dd_p->kbs_green_pixel_first	= 0;


    dd_p->htaps					= NOT_SET;
    dd_p->vtaps					= NOT_SET;

    dd_p->cameralink			= 0;

    dd_p->start_delay			= 0;
    dd_p->frame_period			= NOT_SET;
    dd_p->frame_timing			= NOT_SET;

    dd_p->strobe_enabled		= NOT_SET;
    dd_p->register_wrap			= 0;
    dd_p->serial_init_delay		= NOT_SET;

    dd_p->irig_offset			= 2;

	//  xregwrite registers can be 0-ff. We need a way to flag the
	//  end of the array, so just waste ff and call that "not set"
	// +-----------------------------------------------------------+
    for ( int i=0; i<32; i++ )
	{
		dd_p->xilinx_flag[ i ] = 0xff;
	}
}


// +----------------------------------------------------------------------------
// |  FlushSerial
// +----------------------------------------------------------------------------
// |  Flushes any junk that may be in the serial lines.
// +----------------------------------------------------------------------------
void CArcEdtCL::FlushSerial()
{
	const int SERBUFSIZE = 512;

	if ( IsOpen() )
	{
		char pszBuf[ SERBUFSIZE + 1 ];

	    ( void )pdv_serial_read( m_hDevice, pszBuf, SERBUFSIZE );
	}
}














void CArcEdtCL::Expose( float fExpTime, int dRows, int dCols, const bool& bAbort,
						 CExpIFace* pExpIFace, bool bOpenShutter )
{
	float fElapsedTime		= fExpTime;
	bool  bInReadout		= false;
	int   dTimeoutCounter	= 0;
	int   dLastPixelCount	= 0;
	int   dPixelCount		= 0;
	int   dExposeCounter	= 0;

	//
	// Check for adequate buffer size
	//
	if ( int( dRows * dCols * sizeof( unsigned short ) ) > CommonBufferSize() )
	{
		CArcTools::ThrowException(
							"CArcEdtCL",
							"Expose",
							"Image dimensions [ %d x %d ] exceed buffer size: %d. %s.",
							dCols,
							dRows,
							CommonBufferSize(),
							"Try calling ReMapCommonBuffer()" );
	}

	//
	// Inform EDT board of new dimensions if different from current
	//
	if ( m_hDevice->dd_p->height != dRows || m_hDevice->dd_p->width != dCols )
	{
		pdv_setsize( m_hDevice, dCols, dRows );
	}

	//  Cleans up after a timeout
	// +--------------------------------------------+
	pdv_timeout_cleanup( m_hDevice );

	if ( pdv_set_timeout( m_hDevice, int( fExpTime * 1000 ) + 1000 ) < 0 )
	{
		CArcTools::ThrowException( "CArcEdtCL",
								   "Expose",
								    CArcTools::GetSystemMessage( int( edt_errno() ) ).c_str() );
	}

//	cout << "TIMEOUT SET TO -> " << pdv_get_timeout( m_hDevice ) << endl;

	//  Resets the frame valid counter to zero
	// +--------------------------------------------+
	pdv_cl_reset_fv_counter( m_hDevice );

	//  Flush fifos
	// +--------------------------------------------+
	pdv_flush_fifo( m_hDevice );

#ifdef WIN2K
	edt_get_kernel_event( m_hDevice, EDT_EVENT_BUF );
#endif

	//  Start DMA to the specified number of buffers
	// +--------------------------------------------+
//	pdv_start_image( m_hDevice );
	edt_start_buffers( m_hDevice, 1 );

	//
	//  Start the exposure
	// +--------------------------------------------+
	int dRetVal = Command( TIM_ID, 0x53494D, dRows, dCols );

	//if ( dRetVal != DON )
	//{
	//	CArcTools::ThrowException( "CArcEdtCL",
	//							   "Expose",
	//							   "Start exposure command failed. Reply: 0x%X",
	//								dRetVal );
	//}

	while ( dPixelCount < ( dRows * dCols ) )
	{
		if ( IsReadout() )
		{
			bInReadout = true;
		}

		//if ( pExpIFace != NULL )
		//{
		//	pExpIFace->ExposeCallback( static_cast<int>( bInReadout ) );
		//}

		// ----------------------------
		// READOUT PIXEL COUNT
		// ----------------------------
		// Save the last pixel count for use by the timeout counter.
		dLastPixelCount = dPixelCount;
		dPixelCount = GetPixelCount();

		if ( ContainsError( dPixelCount ) )
		{
			CArcTools::ThrowException( "CArcEdtCL",
									   "Expose",
									   "Failed to read pixel count!" );
		}

		if ( bInReadout && pExpIFace != NULL )
		{
			pExpIFace->ReadCallback( dPixelCount );
		}

		// If the controller's in READOUT, then increment the timeout
		// counter. Checking for readout prevents timeouts when clearing
		// large and/or slow arrays.
		if ( bInReadout && dPixelCount == dLastPixelCount )
		{
			dTimeoutCounter++;
		}
		else
		{
			dTimeoutCounter = 0;
		}

		if ( edt_timeouts( m_hDevice ) > 0 )
		{
			CArcTools::ThrowException( "CArcEdtCL",
									   "Expose",
									   "Read Timeout!" );
		}
	}
}
