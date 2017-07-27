#ifndef _ARC_CDISPLAY_H_
#define _ARC_CDISPLAY_H_

#include <stdexcept>
#include <string>
#include "DllMain.h"

#ifdef WIN32
	#include <windows.h>

#elif defined( linux ) || defined( __linux ) || defined( __APPLE__ )
	#include <xpa.h>

#endif




namespace arc
{
	// This class is exported from the DllTest.dll
	class DLLDISPLAY_API CDisplay
	{
		public:
			// Default constructor/destructor
			// ------------------------------------------------------------------------
			 CDisplay();
			~CDisplay();

			// Display methods
			// ------------------------------------------------------------------------
			void Launch( int dMSDelay = 1000 );
			void Show( void *pBuffer, int dRows, int dCols );
			void Show( std::string sFitsFile );
			void Clear( int dFrame = -1 );
			void Terminate();

			const static int BUFSIZE	= 512;

		private:
			void Set( std::string sDS9Cmd, void *pBuffer = NULL, int dRows = 0, int dCols = 0 );
			void RunProcess( std::string sBuf );
			void RunAndSaveProcess( std::string sBuf );
			void ThrowException( std::string sMethodName, std::string sMsg );

		#ifdef WIN32
			bool GetModuleDirectory( TCHAR* pszBuf, size_t tBufSize );
		#else
			bool GetModuleDirectory( char* pszBuf, size_t tBufSize );
		#endif

		#ifdef WIN32
			PROCESS_INFORMATION m_processInfo;
		#elif defined( linux ) || defined( __APPLE__ )
			pid_t	m_pid;
			XPA		m_xpa;
		#endif
	};

}	// end namespace

#endif
