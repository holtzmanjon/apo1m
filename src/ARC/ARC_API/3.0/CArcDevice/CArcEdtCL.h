#ifndef _CARC_EDTCL_H_
#define _CARC_EDTCL_H_

#include <string>
#include <vector>
#include <map>
#include "CArcPCIBase.h"
#include "DllMain.h"
#include "edtinc.h"


namespace arc
{

	//  CArcEdtCL ( EDT Camera Link ) Class
	// +-------------------------------------------------+
	class CARCDEVICE_API CArcEdtCL : public CArcPCIBase
	{
		public:
			//  Constructor/Destructor
			// +-------------------------------------------------+
			CArcEdtCL();
			virtual ~CArcEdtCL();

			const char* ToString();

			//  PCI(e) configuration space access
			// +-------------------------------------------------+
			int  GetCfgSpByte( int dOffset );
			int  GetCfgSpWord( int dOffset );
			int  GetCfgSpDWord( int dOffset );

			void SetCfgSpByte( int dOffset, int dValue );
			void SetCfgSpWord( int dOffset, int dValue );
			void SetCfgSpDWord( int dOffset, int dValue );

			//  Device access
			// +-------------------------------------------------+
			static void FindDevices();
			static void UseDevices( const char** pszDeviceList, int dListCount );
			static int  DeviceCount();
			static const char** GetDeviceStringList();
			static void FreeDeviceStringList();

			bool IsOpen();
			void Open( int dDeviceNumber = 0 );
			void Open( int dDeviceNumber, int dBytes );
			void Close();
			void Reset();

			void  MapCommonBuffer( int dBytes = 0 );
			void  UnMapCommonBuffer();
			bool  GetCommonBufferProperties();

			int  GetId();
			int  GetStatus();
			void ClearStatus();

			void Set2xFOTransmitter( bool bOnOff );
			void LoadDeviceFile( const char* pszFile );

			const char*** GetProperties();
			void FreeProperties();
			int  PropertyCount();

			//  Setup & General commands
			// +-------------------------------------------------+
			int  Command( int dBoardId, int dCommand, int dArg0 = -1, int dArg1 = -1, int dArg2 = -1, int dArg3 = -1 );

			int  GetControllerId();
			void ResetController();
			bool IsControllerConnected();

			void LoadControllerFile( const char* pszFilename, bool bValidate = true, const bool& bAbort = false );

			//  Expose commands
			// +-------------------------------------------------+
			void Expose( float fExpTime, int dRows, int dCols, const bool& bAbort = false,
						 CExpIFace* pExpIFace = NULL, bool bOpenShutter = true );

			void StopExposure();
			bool IsReadout();
			int  GetPixelCount();
			int  GetCRPixelCount();
			int  GetFrameCount();

			//  EDT only commands
			// +-------------------------------------------------+
			int GetBaudRate();

		protected:
			int  SmallCamDLoad( int dBoardId, std::vector<int>& vData ) { return 0; };
			void LoadGen23ControllerFile( const char *pszFilename, bool bValidate, const bool& bAbort = false ) {};
			void SetByteSwapping() {};

			void WriteSerialByte( unsigned char zByte );
			void WriteSerialBytes( int dWord );
			void WriteSerialBytes( void* pBuffer, int dSize );
			int  ReadReply( double gTimeOutSecs = 1.5 );
			void DefaultDependent( PdvDependent* dd_p );
			void FlushSerial();

			PdvDev*		m_hDevice;
			//EdtDev*	m_hDevice;
			Dependent*	m_pDependent;

			std::map<std::string,std::string> m_mPropList;
			char*** m_pszPropList;

			static std::vector<ArcDev_t>	m_vDevList;
			static char**					m_pszDevList;
	};


	//  Define EDT dependent default values
	// +-------------------------------------------------+
	#define ARC_EDT_CAMERA_CLASS		"ARC, Inc"
	#define ARC_EDT_CAMERA_MODEL		"FastCam"
	#define ARC_EDT_CAMERA_INFO			"Camera Link 16-bit"

	#define ARC_EDT_BUF_WIDTH			4200		// Default buffer cols
	#define ARC_EDT_BUF_HEIGHT			4200		// Default buffer rows
	#define ARC_EDT_IMG_DEPTH			16			// Bits-per-pixel coming from camera
	#define ARC_EDT_DATA_PATH			0x3f		// Camera link data rate - 3 = 4 pixels/clock
	#define ARC_EDT_BAUD_RATE			460800		// Serial baud rate - fixed by controller FPGA


	//  Define FastCam DSP download header byte
	// +-------------------------------------------------+
	#define DSP_HEADER_BYTE				0xDD


	//  Define max DSP file size that FastCam can handle
	// +-------------------------------------------------+
	#define MAX_DSP_FILE_SIZE			65536


			////  Define EDT properties
			//// +--------------------------------------------------+
			//enum {
			//	ARC_EDT_PROP_CAM_CLASS = 0,			// Camera class <string>
			//	ARC_EDT_PROP_CAM_MODEL,				// Camera model <string>
			//	ARC_EDT_PROP_CAM_INFO,				// Camera info  <string>
			//	ARC_EDT_PROP_BUF_WIDTH,				// Buffer width <integer>
			//	ARC_EDT_PROP_BUF_HEIGHT,			// Buffer height <integer>
			//	ARC_EDT_PROP_BUF_DEPTH,				// Buffer bits-per-pixel <integer>
			//	ARC_EDT_PROP_EXT_DEPTH,				// Camera bits-per-pixel <integer>
			//	ARC_EDT_PROP_DATA_PATH,				// Camera link data path <hex-integer>
			//	ARC_EDT_PROP_SER_BAUD,				// Serial link baud rate <integer>
			//	ARC_EDT_PROP_COUNT
			//};

}	// end namespace

#endif
