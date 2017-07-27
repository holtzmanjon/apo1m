#ifndef _CTIFF_FILE_H_
#define _CTIFF_FILE_H_

#include <stdexcept>
#include <string>
#include "DllMain.h"
#include "tiffio.h"



namespace arc
{
	class DLLTIFFFILE_API CTiffFile
	{
		public:
			// Read/Write existing file constructor
			// ------------------------------------------------------------------------
			 CTiffFile( const char* pszFilename, int dMode = WRITEMODE );
			~CTiffFile();

			// Header methods
			// ------------------------------------------------------------------------
			int GetRows();
			int GetCols();
			int GetBpp();

			// Read/Write methods
			// ------------------------------------------------------------------------
			void Write( void* pData, int dRows, int dCols, int dBpp = BPP16 );
			void *Read();

			// Constants
			// ------------------------------------------------------------------------
			const static int READMODE  = 0;
			const static int WRITEMODE = 1;

			const static int BPP8  = 8;
			const static int BPP16 = 16;

			const static int RGB_SAMPLES_PER_PIXEL  = 3;
			const static int RGBA_SAMPLES_PER_PIXEL = 4;

		private:
			void WriteU16( void* pData, int dRows, int dCols );
			void WriteU8( void* pData, int dRows, int dCols );

			void ReadU16( void* pData );
			void ReadU8( void* pData );

			void ThrowException( std::string sMethodName, std::string sMsg );

			TIFF*   m_pTiff;
			uint16* m_pDataBuffer;
			uint32  m_dBpp;
			uint32  m_dRows;
			uint32  m_dCols;
	};

}	// end namespace

#endif
