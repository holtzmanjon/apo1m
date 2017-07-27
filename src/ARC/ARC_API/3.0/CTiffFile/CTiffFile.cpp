// CTiff.cpp : Defines the exported functions for the DLL application.
//
//
// Notes:
//
// Memory Management
// -------------------
// libtiff uses a machine-specific set of routines for managing dynamically
// allocated memory. _TIFFmalloc, _TIFFrealloc, and _TIFFfree mimic the normal
// ANSI C routines. Any dynamically allocated memory that is to be passed into
// the library should be allocated using these interfaces in order to insure
// pointer compatibility on machines with a segmented architecture. (On 32-bit
// UNIX systems these routines just call the normal malloc, realloc, and free
// routines in the C library.)
//
// To deal with segmented pointer issues libtiff also provides _TIFFmemcpy,
// _TIFFmemset, and _TIFFmemmove routines that mimic the equivalent ANSI C
// routines, but that are intended for use with memory allocated through
// _TIFFmalloc and _TIFFrealloc. 
//
#include "CTiffFile.h"
#include <sstream>

using namespace std;
using namespace arc;


// +---------------------------------------------------------------------------+
// |  Constructor for READING/WRITING a TIFF file.                             |
// +---------------------------------------------------------------------------+
// |  Opens an existing or creates a new TIFF file depending on the mode       |
// |  parameter. The default mode is to create ( WRITEMODE ).                      |
// |                                                                           |
// |  Throws std::runtime_error on parameter or libtiff library error.         |
// |                                                                           |
// |  <IN> -> pszFilename - The file to open, including path.                  |
// |  <IN> -> dMode       - Read/Write mode, default = CTiffFile::WRITEMODE.       |
// +---------------------------------------------------------------------------+
CTiffFile::CTiffFile( const char* pszFilename, int dMode )
{
	string sFilename( pszFilename );
	string sMode( "w" );

	// Verify filename and make sure the kernel image
	// buffer has been initialized.
	// --------------------------------------------------
	if ( sFilename.empty() )
	{
		ThrowException( "CTiffFile",
						"Invalid file name : " + sFilename );
	}

	m_pTiff = NULL;

	if ( dMode == CTiffFile::READMODE )
	{
		sMode = "r";
	}

	// Open the TIFF file
	// --------------------------------------------------
	m_pTiff = TIFFOpen( sFilename.c_str(), sMode.c_str() );

	if ( m_pTiff == NULL )
	{
		ThrowException( "CTiffFile",
						"Failed to create " + sFilename );
	}

	// Get image dimensions if READMODE
	// --------------------------------------------------
	if ( dMode == CTiffFile::READMODE )
	{

		TIFFGetField( m_pTiff, TIFFTAG_IMAGEWIDTH,    &m_dRows );
		TIFFGetField( m_pTiff, TIFFTAG_IMAGELENGTH,   &m_dCols );

		//  For some maddening reason, using &m_dBpp produces,
		//  garbage, while the following works!!!!
		// -----------------------------------------------------------
		uint32 dBitsPerSample = 0;
		TIFFGetField( m_pTiff, TIFFTAG_BITSPERSAMPLE, &dBitsPerSample );
		m_dBpp = dBitsPerSample;
	}

	m_pDataBuffer = NULL;
}

// +---------------------------------------------------------------------------+
// |  Class destructor                                                         |
// +---------------------------------------------------------------------------+
// |  Destroys the class. Deallocates any data buffers. Closes any open TIFF   |
// |  pointers.                                                                |
// +---------------------------------------------------------------------------+
CTiffFile::~CTiffFile()
{
	if ( m_pDataBuffer != NULL )
	{
		delete [] m_pDataBuffer;
	}

	if ( m_pTiff != NULL )
	{
		TIFFClose( m_pTiff );
	}

	m_pTiff = NULL;
	m_dRows = 0;
	m_dCols = 0;
	m_dBpp  = 0;
}

// +---------------------------------------------------------------------------+
// |  GetRows                                                                  |
// +---------------------------------------------------------------------------+
// |  Returns the row dimension of the TIFF file currently open.               |
// |                                                                           |
// |  <OUT> -> Number of rows in TIFF image.                                   |
// +---------------------------------------------------------------------------+
int CTiffFile::GetRows()
{
	return int( m_dRows );
}

// +---------------------------------------------------------------------------+
// |  GetCols                                                                  |
// +---------------------------------------------------------------------------+
// |  Returns the column dimension of the TIFF file currently open.            |
// |                                                                           |
// |  <OUT> -> Number of columns in TIFF image.                                |
// +---------------------------------------------------------------------------+
int CTiffFile::GetCols()
{
	return int( m_dCols );
}

// +---------------------------------------------------------------------------+
// |  GetBpp                                                                   |
// +---------------------------------------------------------------------------+
// |  Returns the bits-per-pixel ( bits-per-sample ) of the TIFF file          |
// |  currently open.                                                          |
// |                                                                           |
// |  <OUT> -> bits-per-pixel ( bits-per-sample ) in TIFF image.               |
// +---------------------------------------------------------------------------+
int CTiffFile::GetBpp()
{
	return int( m_dBpp );
}

// +---------------------------------------------------------------------------+
// |  Write                                                                    |
// +---------------------------------------------------------------------------+
// |  Writes 16-bit grayscale image to a 16-bit TIFF file.                     |
// |                                                                           |
// |  Throws std::runtime_error on parameter error or libtiff library error.   |
// |                                                                           |
// |  <IN> -> pData  - Pointer to 16-bit image data.                           |
// |  <IN> -> dRows  - Number of rows in image data.                           |
// |  <IN> -> dCols  - Number of cols in image data.                           |
// |  <IN> -> dBpp   - The bits-per-pixel(/sample), default = 16.              |
// +---------------------------------------------------------------------------+
void CTiffFile::Write( void* pData, int dRows, int dCols, int dBpp )
{
	if ( dBpp == CTiffFile::BPP16 )
	{
		WriteU16( pData, dRows, dCols );
	}

	else if ( dBpp == CTiffFile::BPP8 )
	{
		WriteU8( pData, dRows, dCols );
	}

	else
	{
		ThrowException( "Write",
						"Invalid bits-per-pixel, must be 8 or 16!" );
	}
}

// +---------------------------------------------------------------------------+
// |  Read                                                                     |
// +---------------------------------------------------------------------------+
// |  Reads from a TIFF file.                                                  |
// |                                                                           |
// |  Throws std::runtime_error on parameter or libtiff library error.         |
// |                                                                           |
// |  <OUT> -> Pointer to the image data.                                      |
// +---------------------------------------------------------------------------+
void *CTiffFile::Read()
{
	//  Verify TIFF file is open
	// --------------------------------------------------------------------
	if ( m_pTiff == NULL )
	{
		ThrowException( "Read",
						"Invalid TIFF handle, no file open!" );
	}

	//  Verify the image dimensions
	// --------------------------------------------------------------------
	if ( m_dRows <= 0 || m_dCols <= 0 )
	{
		ostringstream oss;

		oss << "Invalid TIFF image dimensions, rows: "
			<< m_dRows << " cols: " << m_dCols;

		ThrowException( "Read", oss.str() );
	}

	//  Verify the bits-per-pixel ( bits-per-sample ), must be 8 0r 16
	// --------------------------------------------------------------------
	if ( int( m_dBpp ) != CTiffFile::BPP16 && int( m_dBpp ) != CTiffFile::BPP8 )
	{
		ostringstream oss;

		oss << "Invalid TIFF bits-per-pixel: " << m_dBpp
			<< "! Must be 8 or 16!" << ends;

		ThrowException( "Read", oss.str() );
	}

	//  Verify the samples-per-pixel is three, only RGB is supported
	// --------------------------------------------------------------------
	uint32 dSamplesPerPixel = 0;
	TIFFGetField( m_pTiff, TIFFTAG_SAMPLESPERPIXEL, &dSamplesPerPixel );

	if ( int( dSamplesPerPixel ) != CTiffFile::RGB_SAMPLES_PER_PIXEL )
	{
		ostringstream oss;
		oss << "Invalid TIFF samples-per-pixel: " << dSamplesPerPixel
			<< "! Only RGB samples-per-pixel of three is supported!"
			<< ends;

		ThrowException( "Read", oss.str() );
	}

	//  Allocate 16-bit buffer
	// --------------------------------------------------------------------
	m_pDataBuffer = new uint16[ m_dRows * m_dCols ];

	if ( m_pDataBuffer == NULL )
	{
		ThrowException( "Read",
						"Failed to allocate TIFF data buffer!" );
	}

	if ( int( m_dBpp ) == CTiffFile::BPP16 )
	{
		ReadU16( m_pDataBuffer );
	}
	else
	{
		ReadU8( m_pDataBuffer );
	}

	return ( void * )m_pDataBuffer;
}

// +---------------------------------------------------------------------------+
// |  WriteU16                                                                 |
// +---------------------------------------------------------------------------+
// |  Writes 16-bit grayscale image to a 16-bit TIFF file.                     |
// |                                                                           |
// |  Throws std::runtime_error on parameter error or libtiff library error.   |
// |                                                                           |
// |  <IN> -> pData - Pointer to 16-bit image data.                            |
// |  <IN> -> dRows - Number of rows in image data.                            |
// |  <IN> -> dCols - Number of cols in image data.                            |
// +---------------------------------------------------------------------------+
void CTiffFile::WriteU16( void* pData, int dRows, int dCols )
{
	if ( m_pTiff == NULL )
	{
		ThrowException( "Write",
						"Invalid TIFF handle, no file open!" );
	}

	if ( pData == NULL )
	{
		ThrowException( "Write",
						"Invalid pixel data, NULL pointer!" );
	}

	tsize_t tTotalDataSize =
			dCols * dRows * CTiffFile::RGB_SAMPLES_PER_PIXEL * sizeof( uint16 );


	//  Write TIFF header tags
	// --------------------------------------------------
	TIFFSetField( m_pTiff, TIFFTAG_IMAGEWIDTH, dCols );
	TIFFSetField( m_pTiff, TIFFTAG_IMAGELENGTH, dRows );
	TIFFSetField( m_pTiff, TIFFTAG_BITSPERSAMPLE, CTiffFile::BPP16 );
	TIFFSetField( m_pTiff, TIFFTAG_SAMPLESPERPIXEL, CTiffFile::RGB_SAMPLES_PER_PIXEL );
	TIFFSetField( m_pTiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
	TIFFSetField( m_pTiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
	TIFFSetField( m_pTiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );    // set the origin of the image

	//  We set the strip size of the file to be size of one row of pixels
	// ------------------------------------------------------------------------------------
	TIFFSetField( m_pTiff,
				  TIFFTAG_ROWSPERSTRIP,
				  TIFFDefaultStripSize( m_pTiff, dCols * CTiffFile::RGB_SAMPLES_PER_PIXEL ) );

	//  Copy the image data to libtiff buffer. Must be this way or it won't work!
	// ---------------------------------------------------------------------------
	uint16* pU16TiffBuf = ( uint16 * )_TIFFmalloc( tTotalDataSize );

	for ( int i=0, pos=0; i<(dRows * dCols); i++, pos+=CTiffFile::RGB_SAMPLES_PER_PIXEL )
	{
		pU16TiffBuf[ pos + 0 ] = ( ( uint16 * )pData )[ i ];
		pU16TiffBuf[ pos + 1 ] = ( ( uint16 * )pData )[ i ];
		pU16TiffBuf[ pos + 2 ] = ( ( uint16 * )pData )[ i ];
	}

	// Write the image to the file
	// --------------------------------------------------
	TIFFWriteEncodedStrip( m_pTiff, 0, ( uint8 * )pU16TiffBuf, tTotalDataSize );

	//  The following would work too ....
	// --------------------------------------------------
	//for ( uint32 row = 0; row<uint32( dRows ); row++ )
	//{
	//	if ( TIFFWriteScanline( m_pTiff, &pU16TiffBuf[ row * dCols ], row, 0 ) < 0 )
	//	{
			//ostringstream oss; 
			//oss << "Failed to write row #" << y;

			//ThrowException( "Write", oss.str() );
	//	}
	//}

	_TIFFfree( pU16TiffBuf );
}

// +---------------------------------------------------------------------------+
// |  WriteU8                                                                  |
// +---------------------------------------------------------------------------+
// |  Writes 16-bit grayscale image to an 8-bit TIFF file.                     |
// |                                                                           |
// |  Throws std::runtime_error on parameter error or libtiff library error.   |
// |                                                                           |
// |  <IN> -> pData - Pointer to 16-bit image data.                            |
// |  <IN> -> dRows - Number of rows in image data.                            |
// |  <IN> -> dCols - Number of cols in image data.                            |
// +---------------------------------------------------------------------------+
void CTiffFile::WriteU8( void* pData, int dRows, int dCols )
{
	if ( m_pTiff == NULL )
	{
		ThrowException( "Write",
						"Invalid TIFF handle, no file open!" );
	}

	if ( pData == NULL )
	{
		ThrowException( "Write",
						"Invalid pixel data, NULL pointer!" );
	}

	tsize_t tTotalDataSize = dCols * dRows * CTiffFile::RGB_SAMPLES_PER_PIXEL;


	//  Write TIFF header tags
	// --------------------------------------------------
	TIFFSetField( m_pTiff, TIFFTAG_IMAGEWIDTH, dCols );
	TIFFSetField( m_pTiff, TIFFTAG_IMAGELENGTH, dRows );
	TIFFSetField( m_pTiff, TIFFTAG_BITSPERSAMPLE, CTiffFile::BPP8 );
	TIFFSetField( m_pTiff, TIFFTAG_SAMPLESPERPIXEL, CTiffFile::RGB_SAMPLES_PER_PIXEL );
	TIFFSetField( m_pTiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
	TIFFSetField( m_pTiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
	TIFFSetField( m_pTiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );    // set the origin of the image

	//  We set the strip size of the file to be size of one row of pixels
	// ------------------------------------------------------------------------------------
	TIFFSetField( m_pTiff,
				  TIFFTAG_ROWSPERSTRIP,
				  TIFFDefaultStripSize( m_pTiff, dCols * CTiffFile::RGB_SAMPLES_PER_PIXEL ) );

	//  Copy the image data to libtiff buffer. Must be this way or it won't work!
	// ---------------------------------------------------------------------------
	uint8* pU8TiffBuf = ( uint8 * )_TIFFmalloc( tTotalDataSize );
	uint8  u8TiffData = 0;

	for ( int i=0, pos=0; i<(dRows * dCols); i++, pos+=CTiffFile::RGB_SAMPLES_PER_PIXEL )
	{
		u8TiffData = ( ( ( uint16 * )pData )[ i ] / 256 );

		pU8TiffBuf[ pos + 0 ] = u8TiffData;		// red
		pU8TiffBuf[ pos + 1 ] = u8TiffData;		// green
		pU8TiffBuf[ pos + 2 ] = u8TiffData;		// blue
	}

	// Write the image to the file
	// --------------------------------------------------
	TIFFWriteEncodedStrip( m_pTiff, 0, ( uint8 * )pU8TiffBuf, tTotalDataSize );


	//  The following would work too ....
	// --------------------------------------------------
	//for ( uint32 row = 0; row<uint32( dRows ); row++ )
	//{
	//	if ( TIFFWriteScanline( m_pTiff, &pU8TiffBuf[ row * dCols ], row, 0 ) < 0 )
	//	{
	//		ostringstream oss; 
	//		oss << "Failed to write row #" << row << ends;

	//		ThrowException( "Write", oss.str() );
	//	}
	//}

	_TIFFfree( pU8TiffBuf );
}

// +---------------------------------------------------------------------------+
// |  ReadU16                                                                  |
// +---------------------------------------------------------------------------+
// |  Reads 16-bit grayscale image from a 16-bit TIFF file.                    |
// |                                                                           |
// |  Throws std::runtime_error on parameter error or libtiff library error.   |
// |                                                                           |
// |  <IN> -> pDataBuffer - Pointer to 16-bit image data buffer.               |
// +---------------------------------------------------------------------------+
void CTiffFile::ReadU16( void* pDataBuffer )
{
	//  Verify 16-bit output buffer
	// --------------------------------------------------------------------
	if ( pDataBuffer == NULL )
	{
		ThrowException( "ReadU16",
						"Failed to allocate TIFF data buffer!" );
	}

	//  Calculate size of 16-bit RGB TIFF buffer
	// --------------------------------------------------------------------
	tsize_t tTiffDataSize = ( m_dRows * m_dCols * m_dBpp * sizeof( uint16 ) );

	//  Allocate 16-bit TIFF buffer
	// --------------------------------------------------------------------
	uint16* pU16TiffData = ( uint16 * )_TIFFmalloc( tTiffDataSize );

	//  Read the image data from the TIFF
	// --------------------------------------------------------------------
	TIFFReadEncodedStrip( m_pTiff, 0, ( uint8 * )pU16TiffData, tTiffDataSize );

	//  Convert the 16-bit RGB TIFF data to single 16-bit buffer
	// --------------------------------------------------------------------
	for ( int i=0, j=0; i<int( m_dRows * m_dCols ); i++, j+=CTiffFile::RGB_SAMPLES_PER_PIXEL )
	{
		( ( uint16 * )pDataBuffer )[ i ] = pU16TiffData[ j ];
	}

	//  Free the 16-bit RGB TIFF buffer
	// --------------------------------------------------------------------
	_TIFFfree( pU16TiffData );
}

// +---------------------------------------------------------------------------+
// |  ReadU8                                                                   |
// +---------------------------------------------------------------------------+
// |  Reads 16-bit grayscale image from a 8-bit TIFF file. Each pixel from     |
// |  TIFF file is multiplied by 256 to convert to 16-bit image data.          |
// |                                                                           |
// |  Throws std::runtime_error on parameter error or libtiff library error.   |
// |                                                                           |
// |  <IN> -> pDataBuffer - Pointer to 16-bit image data buffer.               |
// +---------------------------------------------------------------------------+
void CTiffFile::ReadU8( void* pDataBuffer )
{
	//  Verify 16-bit output buffer
	// --------------------------------------------------------------------
	if ( pDataBuffer == NULL )
	{
		ThrowException( "ReadU8",
						"Failed to allocate TIFF data buffer!" );
	}

	//  Calculate size of 8-bit RGB TIFF buffer
	// --------------------------------------------------------------------
	tsize_t tTiffDataSize = ( m_dRows * m_dCols * m_dBpp );

	//  Allocate 8-bit TIFF buffer
	// --------------------------------------------------------------------
	uint8* pU8TiffData = ( uint8 * )_TIFFmalloc( tTiffDataSize );

	//  Read the image data from the TIFF
	// --------------------------------------------------------------------
	TIFFReadEncodedStrip( m_pTiff, 0, ( uint8 * )pU8TiffData, tTiffDataSize );

	//  Convert the 8-bit RGB TIFF data to single 16-bit buffer
	// --------------------------------------------------------------------
	for ( int i=0, j=0; i<int( m_dRows * m_dCols ); i++, j+=CTiffFile::RGB_SAMPLES_PER_PIXEL )
	{
		( ( uint16 * )pDataBuffer )[ i ] = pU8TiffData[ j ] * 256;
	}

	//  Free the 16-bit RGB TIFF buffer
	// --------------------------------------------------------------------
	_TIFFfree( pU8TiffData );
}

// +----------------------------------------------------------------------------
// |  ThrowException
// +----------------------------------------------------------------------------
// |  Method to throw a "const char *" exception.
// |
// |  <IN> -> sMethodName : The method where the exception occurred.
// |  <IN> -> sMsg        : A std::string to use for the exception message.
// +----------------------------------------------------------------------------
void CTiffFile::ThrowException( std::string sMethodName, std::string sMsg )
{
	ostringstream oss;

	oss << "( CTiffFile::"
		<< ( sMethodName.empty() ? "???" : sMethodName )
		<< "() ): "
		<< sMsg
		<< ends;

	throw std::runtime_error( ( const std::string )oss.str() );
}

