#ifndef CARCAPISERVER_H_
#define CARCAPISERVER_H_

#ifdef WIN32
#include <Winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#endif

#include "CArcDevice.h"
#include "CDeinterlace.h"
#include "CFitsFile.h"
#include "CTiffFile.h"
#include "CImage.h"


#ifdef WIN32
	typedef int						socklen_t;
#else
	#define INVALID_SOCKET			-1
	#define SOCKET_ERROR			-1
	#define MAX_PATH				FILENAME_MAX
	typedef int						SOCKET;
	typedef struct sockaddr_in		SOCKADDR_IN;
	typedef struct sockaddr			SOCKADDR;
	typedef const struct sockaddr*	LPSOCKADDR;
	typedef unsigned long			ULONG_PTR;
	typedef unsigned long			DWORD;
	typedef unsigned int			UINT;
#endif



namespace arc
{

	class CArcAPIServer
	{
		public:
			CArcAPIServer();
			~CArcAPIServer();

			void Start( bool bLog = false );
			void Bind( unsigned short usPort );
			void CloseConnection( SOCKET& tSocket );
			void CloseAllConnections();

		protected:
			std::string GetSystemMessage( int dCode );
			void   ThrowException( std::string sMethodName, std::string sMsg );
			void   ZeroMem( void* pBuf, int dSize );
			int    GetError();

			void HandleArcAPIServer( std::string sInData );
			void HandleCArcDevice( std::string sInData );
			void HandleCDeinterlace( std::string sInData );
			void HandleCFitsFile( std::string sInData );
			void HandleCTiffFile( std::string sInData );
			void HandleCImage( std::string sInData );

			const std::string GetTempFileName();
			void FixMultiStringData( std::string& sData );
			const std::string StripCmd( const std::string sInData );

			std::string RecvDspFile( const std::string sBoard );
			void RecvFile( std::string sFilename, int dLength );
			int Recv( void* pBuffer, int dSize );
			std::string Recv();
			std::string Peek();

			void SendOK();
			void Send( int dVal );
			void Send( long lVal );
			void Send( double gVal );
			void Send( ULONG_PTR ulptr );
			void Send( const char *fmt, ... );
			void Send( void* pBuffer, int dSize );
			void SendException( const char* szMsg );

			#ifdef WIN32
			static unsigned __stdcall threadProc( void * );
			#else
			static void* threadProc( void * );
			#endif

			int Run();

			SOCKET m_tBindSocket;
			SOCKET m_tAcceptSocket;		// Probably need more than one to access multiple controllers from multiple clients.
										// Otherwise, why even use a thread!

			#ifndef WIN32
			pthread_t m_tThread;
			#endif

			CArcDevice*                m_pCArcDev;
			CDeinterlace*              m_pCDeinterlace;
			CFitsFile*                 m_pCFitsFile;
			CTiffFile*                 m_pCTiffFile;
			CImage*                    m_pCImage;

			bool m_bLog;
	};

}	// end namespace

#endif
