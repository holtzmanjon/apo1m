//
//  CameraAPI.cpp
//
//  Created by Scott Streit on 2/9/11.
//  Copyright 2011 Astronomical Research Cameras, Inc. All rights reserved.

//
// Note: This may need to be changed in the generated
// owl_cameraAPI_CameraAPI.h header file as well.
//
#ifdef __APPLE__
#include <JavaVM/jni.h>
#else
#include <jni.h>
#endif

#include "CArcAPIClient.h"	// This MUST be first as it includes winsock2.h
#include "owl_cameraAPI_CameraAPI.h"
#include "CArcDevice.h"
#include "CArcPCIe.h"
#include "CArcPCI.h"
#include "CFitsFile.h"
#include "CTiffFile.h"
#include "CDeinterlace.h"
#include "CDisplay.h"
#include "CImage.h"
#include "CArcTools.h"
#include "ArcOSDefs.h"
#include "ArcDefs.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <sstream>
#include <cstring>
#include <memory>
#include <list>
#include <ios>

//#include <fstream>
//std::ofstream dbgStream( "CameraAPI_Debug.txt" );

using namespace std;
using namespace arc;


void ThrowJNIException( JNIEnv* env, string msg );
void ThrowUnknownJNIException( JNIEnv* env );
bool NotZero( jfloat f ) { return ( ( f != 0 ) ); }


// NOTE: Maybe this should be a pointer that is instantiated and then later deleted!
static auto_ptr<CArcDevice>        		pCArcDev( new CArcPCIe() );
static auto_ptr<CArcAPIClient>     		pCArcClient( 0 );
static auto_ptr<CDisplay>          		pCDisplay( 0 );
static auto_ptr<CDeinterlace>			pCDeinterlace( 0 );
static bool                        		bRemoteAPI  = false;
static bool                        		bAbort      = false;

#ifndef WIN32
typedef uintptr_t INT_PTR;
#endif


#ifdef WIN32
	#define ASSERT_CLIENT_CLASS_PTR( env, methStr )	\
					if ( !pCArcClient.get() ) {		\
						throw runtime_error(		\
						"( "## methStr ##" ): Remote API not in use or client class object is NULL!" ); }
#else
	#define ASSERT_CLIENT_CLASS_PTR( env, methStr )	\
					if ( !pCArcClient.get() ) {		\
						throw runtime_error(		\
						"( ## methStr ## ): Remote API not in use or client class object is NULL!" ); }
#endif


/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    UseRemoteAPI
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_UseRemoteAPI( JNIEnv *env, jclass clazz, jboolean jbOnOff )
{
//	bRemoteAPI = jbOnOff;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    ConnectToServer
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_ConnectToServer( JNIEnv *env, jclass clazz, jstring jIPAddr, jint jPort )
{
	//
	// Get the server IP address
	//
	const char* pszIPAddr = ( jIPAddr == NULL ? NULL : ( env )->GetStringUTFChars( jIPAddr, 0 ) );

	if ( pszIPAddr != NULL )
	{
		ThrowJNIException( env,
				"( ConnectToServer ): Failed to convert IP address parameter!" );
	}

	try
	{
		if ( pCArcClient.get() == 0 )
		{
			pCArcClient.reset( new CArcAPIClient() );
		}

		if ( pCArcClient.get() != 0 )
		{
			pCArcClient.get()->Connect( pszIPAddr, int( jPort ) );

			double gVersion = pCArcClient.get()->GetServerVersion();

			if ( gVersion < 3.0 )
			{
				ostringstream oss;

				oss << "Invalid server version: "
					<< gVersion
					<< " Expected version 3.0 or greater!";

				throw std::runtime_error( oss.str() ); 
			}

			bRemoteAPI = true;
		}
		else
		{
			throw std::runtime_error(
							"Failed to create client class object!" );
		}

		// Free the allocated memory for the device name
		( env )->ReleaseStringUTFChars( jIPAddr, pszIPAddr );
	}
	catch ( exception& e )
	{
		bRemoteAPI = false;

		// Free the allocated memory for the device name
		( env )->ReleaseStringUTFChars( jIPAddr, pszIPAddr );

		ThrowJNIException( env,
						   string( "Server Error: " ) + e.what() );
	}
	catch ( ... )
	{
		bRemoteAPI = false;

		// Free the allocated memory for the device name
		( env )->ReleaseStringUTFChars( jIPAddr, pszIPAddr );

		ThrowJNIException( env,
						   "Server Error: Failed to connect to server!" );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    DisconnectServer
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_DisconnectServer( JNIEnv *env, jclass clazz )
{
	try
	{
		ASSERT_CLIENT_CLASS_PTR( env, "DisconnectServer" )

		bRemoteAPI = false;

		pCArcClient.get()->CloseConnections();
	}
	catch ( exception& e )
	{
		ThrowJNIException( env, e.what() );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    LogMsgOnServer
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_LogMsgOnServer( JNIEnv *env, jclass clazz, jstring jMsg )
{
	if ( bRemoteAPI )
	{
		const char* pszMsg = ( jMsg == NULL ? NULL : env->GetStringUTFChars( jMsg, 0 ) );

		if ( pszMsg == NULL )
		{
			ThrowJNIException( env, "( LogMsgOnServer ): Failed to convert message!" );
		}

		try
		{
			ASSERT_CLIENT_CLASS_PTR( env, "LogMsgOnServer" )

			pCArcClient.get()->LogMsgOnServer( pszMsg );

			// Free the allocated memory for the target name
			env->ReleaseStringUTFChars( jMsg, pszMsg );
		}
		catch ( exception& e )
		{
			env->ReleaseStringUTFChars( jMsg, pszMsg );
			ThrowJNIException( env, e.what() );
		}
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    EnableServerLog
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_EnableServerLog( JNIEnv *env, jclass clazz, jboolean jEnable )
{
	if ( bRemoteAPI )
	{
		try
		{
			ASSERT_CLIENT_CLASS_PTR( env, "EnableServerLog" )

			pCArcClient.get()->EnableServerLog( ( jEnable != 0 ) );
		}
		catch ( exception& e )
		{
			ThrowJNIException( env, e.what() );
		}
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    IsServerLogging
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_owl_cameraAPI_CameraAPI_IsServerLogging( JNIEnv *env, jclass clazz )
{
	if ( !bRemoteAPI )
	{
		ThrowJNIException( env,
					"( IsServerLogging ): Remote API not in use!" );
	}

	try
	{
		ASSERT_CLIENT_CLASS_PTR( env, "IsServerLogging" )
	}
	catch ( exception& e )
	{
		ThrowJNIException( env, e.what() );
	}

	return pCArcClient.get()->IsServerLogging();
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetDirList
 * Signature: (Ljava/lang/String;)[Ljava/io/File;
 */
JNIEXPORT jobjectArray JNICALL
Java_owl_cameraAPI_CameraAPI_GetDirList( JNIEnv *env, jclass clazz, jstring jTargetDir )
{
	//
	// Not Used, Returns NULL!
	//
	return NULL;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetAPIConstants
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_GetAPIConstants( JNIEnv *env, jclass clazz )
{
	jfieldID fid;
	
	fid = env->GetStaticFieldID( clazz, "PCI_ID", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "PCI_ID field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, PCI_ID ); }
	
	fid = env->GetStaticFieldID( clazz, "TIM_ID", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "TIM_ID field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, TIM_ID ); }
	
	fid = env->GetStaticFieldID( clazz, "UTIL_ID", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "UTIL_ID field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, UTIL_ID ); }
	
	fid = env->GetStaticFieldID( clazz, "X_MEM", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "X_MEM field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, X_MEM ); }
	
	fid = env->GetStaticFieldID( clazz, "Y_MEM", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "Y_MEM field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, Y_MEM ); }

	fid = env->GetStaticFieldID( clazz, "P_MEM", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "P_MEM field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, P_MEM ); }
	
	fid = env->GetStaticFieldID( clazz, "R_MEM", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "R_MEM field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, R_MEM ); }

	fid = env->GetStaticFieldID( clazz, "DON", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "DON field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, DON ); }
	
	fid = env->GetStaticFieldID( clazz, "ERR", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "ERR field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ERR ); }
	
	fid = env->GetStaticFieldID( clazz, "SYR", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SYR field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SYR ); }
	
	fid = env->GetStaticFieldID( clazz, "TOUT", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "TOUT field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, TOUT ); }
	
	fid = env->GetStaticFieldID( clazz, "READOUT", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "READOUT field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ROUT ); }
	
	fid = env->GetStaticFieldID( clazz, "TDL", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "TDL field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, TDL ); }
	
	fid = env->GetStaticFieldID( clazz, "RDM", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "RDM field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, RDM ); }
	
	fid = env->GetStaticFieldID( clazz, "WRM", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "WRM field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, WRM ); }
	
	fid = env->GetStaticFieldID( clazz, "POF", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "POF field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, POF ); }
	
	fid = env->GetStaticFieldID( clazz, "PON", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "PON field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, PON ); }
	
	fid = env->GetStaticFieldID( clazz, "SET", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SET field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SET ); }
	
	fid = env->GetStaticFieldID( clazz, "RET", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "RET field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, RET ); }
	
	fid = env->GetStaticFieldID( clazz, "SEX", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SEX field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SEX ); }

	fid = env->GetStaticFieldID( clazz, "PEX", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "PEX field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, PEX ); }

	fid = env->GetStaticFieldID( clazz, "REX", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "REX field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, REX ); }

	fid = env->GetStaticFieldID( clazz, "CLR", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "CLR field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CLR ); }
	
	fid = env->GetStaticFieldID( clazz, "OSH", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "OSH field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, OSH ); }
	
	fid = env->GetStaticFieldID( clazz, "CSH", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "CSH field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CSH ); }
	
	fid = env->GetStaticFieldID( clazz, "IDL", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "IDL field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, IDL ); }
	
	fid = env->GetStaticFieldID( clazz, "STP", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "STP field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, STP ); }
	
	fid = env->GetStaticFieldID( clazz, "SMX", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SMX field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SMX ); }
	
	fid = env->GetStaticFieldID( clazz, "CLK", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "CLK field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CLK ); }
	
	fid = env->GetStaticFieldID( clazz, "VID", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "VID field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, VID ); }
	
	fid = env->GetStaticFieldID( clazz, "SBN", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SBN field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SBN ); }

	fid = env->GetStaticFieldID( clazz, "SGN", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SGN field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SGN ); }

	fid = env->GetStaticFieldID( clazz, "SOS", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SOS field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SOS ); }
	
	fid = env->GetStaticFieldID( clazz, "SSS", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SSS field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SSS ); }
	
	fid = env->GetStaticFieldID( clazz, "SSP", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SSP field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SSP ); }
	
	fid = env->GetStaticFieldID( clazz, "RNC", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "RNC field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, RNC ); }
	
	fid = env->GetStaticFieldID( clazz, "SID", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SID field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SID ); }
	
	fid = env->GetStaticFieldID( clazz, "AMP_0", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "AMP_0 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, AMP_0 ); }
	
	fid = env->GetStaticFieldID( clazz, "AMP_1", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "AMP_1 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, AMP_1 ); }
	
	fid = env->GetStaticFieldID( clazz, "AMP_2", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "AMP_2 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, AMP_2 ); }
	
	fid = env->GetStaticFieldID( clazz, "AMP_3", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "AMP_3 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, AMP_3 ); }
	
	fid = env->GetStaticFieldID( clazz, "AMP_L", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "AMP_L field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, AMP_L ); }
	
	fid = env->GetStaticFieldID( clazz, "AMP_R", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "AMP_R field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, AMP_R ); }
	
	fid = env->GetStaticFieldID( clazz, "AMP_LR", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "AMP_LR field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, AMP_LR ); }
	
	fid = env->GetStaticFieldID( clazz, "AMP_ALL", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "AMP_ALL field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, AMP_ALL ); }
	
	fid = env->GetStaticFieldID( clazz, "SPLIT_SERIAL", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SPLIT_SERIAL field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SPLIT_SERIAL ); }
	
	fid = env->GetStaticFieldID( clazz, "SPLIT_PARALLEL", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SPLIT_PARALLEL field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SPLIT_PARALLEL ); }
	
	fid = env->GetStaticFieldID( clazz, "VIDGENI", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "VIDGENI field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, VIDGENI ); }
	
	fid = env->GetStaticFieldID( clazz, "TIMGENI", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "TIMGENI field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, TIMGENI ); }
	
	fid = env->GetStaticFieldID( clazz, "SHUTTER_CC", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SHUTTER_CC field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SHUTTER_CC ); }
	
	fid = env->GetStaticFieldID( clazz, "TEMP_SIDIODE", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "TEMP_SIDIODE field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, TEMP_SIDIODE ); }
	
	fid = env->GetStaticFieldID( clazz, "TEMP_LINEAR", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "TEMP_LINEAR field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, TEMP_LINEAR ); }
	
	fid = env->GetStaticFieldID( clazz, "SUBARRAY", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "SUBARRAY field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, SUBARRAY ); }
	
	fid = env->GetStaticFieldID( clazz, "BINNING", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "BINNING field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, BINNING ); }
	
	fid = env->GetStaticFieldID( clazz, "MPP_CC", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "MPP_CC field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, MPP_CC ); }
	
	fid = env->GetStaticFieldID( clazz, "CLKDRVGENI", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "CLKDRVGENI field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CLKDRVGENI ); }
	
	fid = env->GetStaticFieldID( clazz, "MLO", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "MLO field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, MLO ); }
	
	fid = env->GetStaticFieldID( clazz, "NGST", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "NGST field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, NGST ); }
	
	fid = env->GetStaticFieldID( clazz, "ARC41", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "ARC41 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ARC41 ); }
	
	fid = env->GetStaticFieldID( clazz, "ARC42", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "ARC42 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ARC42 ); }
	
	fid = env->GetStaticFieldID( clazz, "ARC44", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "ARC44 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ARC44 ); }
	
	fid = env->GetStaticFieldID( clazz, "ARC45", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "ARC45 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ARC45 ); }
	
	fid = env->GetStaticFieldID( clazz, "ARC46", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "ARC46 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ARC46 ); }
	
	fid = env->GetStaticFieldID( clazz, "ARC47", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "ARC47 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ARC47 ); }
	
	fid = env->GetStaticFieldID( clazz, "ARC48", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "ARC48 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ARC48 ); }
	
	fid = env->GetStaticFieldID( clazz, "ARC20", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "ARC20 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ARC20 ); }
	
	fid = env->GetStaticFieldID( clazz, "ARC22", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "ARC22 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ARC22 ); }
	
	fid = env->GetStaticFieldID( clazz, "ARC50", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "ARC50 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ARC50 ); }
	
	fid = env->GetStaticFieldID( clazz, "ARC32", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "ARC32 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, ARC32 ); }
	
	fid = env->GetStaticFieldID( clazz, "CONT_RD", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "CONT_RD field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CONT_RD ); }
	
	fid = env->GetStaticFieldID( clazz, "FO_2X_TRANSMITR", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "FO_2X_TRANSMITR field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, FO_2X_TRANSMITR ); }
	
	fid = env->GetStaticFieldID( clazz, "DEINTERLACE_NONE", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "DEINTERLACE_NONE field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CDeinterlace::DEINTERLACE_NONE ); }
	
	fid = env->GetStaticFieldID( clazz, "DEINTERLACE_PARALLEL", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "DEINTERLACE_PARALLEL field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CDeinterlace::DEINTERLACE_PARALLEL ); }
	
	fid = env->GetStaticFieldID( clazz, "DEINTERLACE_SERIAL", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "DEINTERLACE_SERIAL field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CDeinterlace::DEINTERLACE_SERIAL ); }
	
	fid = env->GetStaticFieldID( clazz, "DEINTERLACE_CCD_QUAD", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "DEINTERLACE_CCD_QUAD field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CDeinterlace::DEINTERLACE_CCD_QUAD ); }
	
	fid = env->GetStaticFieldID( clazz, "DEINTERLACE_IR_QUAD", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "DEINTERLACE_IR_QUAD field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CDeinterlace::DEINTERLACE_IR_QUAD ); }
	
	fid = env->GetStaticFieldID( clazz, "DEINTERLACE_CDS_IR_QUAD", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "DEINTERLACE_CDS_IR_QUAD field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CDeinterlace::DEINTERLACE_CDS_IR_QUAD ); }
	
	fid = env->GetStaticFieldID( clazz, "DEINTERLACE_HAWAII_RG", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "DEINTERLACE_HAWAII_RG field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CDeinterlace::DEINTERLACE_HAWAII_RG ); }
	
	fid = env->GetStaticFieldID( clazz, "DEINTERLACE_STA1600", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "DEINTERLACE_STA1600 field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CDeinterlace::DEINTERLACE_STA1600 ); }

	fid = env->GetStaticFieldID( clazz, "CR_WRITE", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "CR_WRITE field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CR_WRITE ); }
	
	fid = env->GetStaticFieldID( clazz, "CR_COADD", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "CR_COADD field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CR_COADD ); }

	fid = env->GetStaticFieldID( clazz, "CR_DEBUG", "I" );
	if ( fid == 0 ) { ThrowJNIException( env, "CR_DEBUG field does not exist!" ); }
	else { env->SetStaticIntField( clazz, fid, CR_DEBUG ); }
}


/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetDeviceList
 * Signature: ([Ljava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_owl_cameraAPI_CameraAPI_GetDeviceList
( JNIEnv *env, jclass clazz, jobjectArray jDevArr )
{
	jobjectArray jDriverList	= 0;
	int		     dDeviceCount	= 0;

	// +-------------------------------------------------+
	// |  Local API Interface                            |
	// +-------------------------------------------------+
	if ( !bRemoteAPI )
	{
		char**		pszPCIDevList	= NULL;
		char**		pszPCIeDevList	= NULL;

		CArcPCIe	cArcPCIe;
		CArcPCI		cArcPCI;

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
				ThrowJNIException( env,
						"( GetDeviceList ): Failed to retrieve device string list!" );
			}

			//
			// Get the available device count
			//
			dDeviceCount = CArcPCI::DeviceCount() + CArcPCIe::DeviceCount();
			
			if ( dDeviceCount > 0 )
			{
				jDriverList =
						( jobjectArray )env->NewObjectArray( dDeviceCount,
															 env->FindClass( "java/lang/String" ),
															 env->NewStringUTF( "" ) );
				
				if ( jDriverList != NULL )
				{
					ostringstream oss;
					
					//
					// Add the PCIe devices
					//
					for ( int i=0; i<CArcPCIe::DeviceCount(); i++ )
					{
						env->SetObjectArrayElement( jDriverList,
													i,
													env->NewStringUTF( pszPCIeDevList[ i ] ) );
						
						if ( env->ExceptionOccurred() != NULL )
						{
							cArcPCIe.FreeDeviceStringList();
							
							ThrowJNIException( env,
									"( GetDeviceList ): Failed to add PCIe device name to list!" );
						}
					}

					//
					// Add the PCI devices
					//
					for ( int i=0, j=CArcPCIe::DeviceCount(); i<CArcPCI::DeviceCount(); i++, j++ )
					{
						env->SetObjectArrayElement( jDriverList,
													j,
													env->NewStringUTF( pszPCIDevList[ i ] ) );
						
						if ( env->ExceptionOccurred() != NULL )
						{
							cArcPCI.FreeDeviceStringList();
							
							ThrowJNIException( env,
										"( GetDeviceList ): Failed to add PCI device name to list!" );
						}
					}
				}
				else
				{
					cArcPCIe.FreeDeviceStringList();
					cArcPCI.FreeDeviceStringList();
					
					ThrowJNIException( env,
									"( GetDeviceList ): Failed create array for device list!" );
				}
				
				cArcPCIe.FreeDeviceStringList();
				cArcPCI.FreeDeviceStringList();
			}
		}
		catch ( std::exception &e )
		{
			cArcPCIe.FreeDeviceStringList();
			cArcPCI.FreeDeviceStringList();
			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			cArcPCIe.FreeDeviceStringList();
			cArcPCI.FreeDeviceStringList();
			ThrowUnknownJNIException( env );
		}
	}

	// +-------------------------------------------------+
	// |  Remote API Interface                           |
	// +-------------------------------------------------+
	else
	{
		char**       pszDevList		= NULL;
		char*        pszDevArr[ 10 ];

		try
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetDeviceList" )
		}
		catch ( exception& e )
		{
			ThrowJNIException( env, e.what() );
		}

		//
		//  Check if an override array exists and handle it.
		//  The override array allows Owl to function using
		//  the older version 1.7 astropci (non-WDF) driver.
		// -------------------------------------------------

		// Get override array size
		jsize jDevSize = env->GetArrayLength( jDevArr );

		for ( int i=0; i<jDevSize; i++ )
		{
			// Get the current object from the object array
			jobject jObj = env->GetObjectArrayElement( jDevArr, i );

			// Convert the object just obtained into a const char*
			const char *cStr = env->GetStringUTFChars( jstring( jObj ), 0 );

			if ( cStr != NULL )
			{
				pszDevArr[ i ] = ( char * )cStr;

				// Free the object element
				env->ReleaseStringUTFChars( jstring( jObj ), cStr );
			}
		}

		//
		//  Get the device bindings
		// -------------------------------------------------
		try
		{
			//
			// Set the available devices if we need to
			//
			if ( jDevSize > 0 )
			{
				pCArcClient.get()->UseDevices( ( const char ** )&pszDevArr[ 0 ],
										 ( int )jDevSize );
			}

			//
			// Get the available devices
			//
			pszDevList = pCArcClient.get()->GetDeviceList( dDeviceCount );

			if ( dDeviceCount > 0 )
			{
				jDriverList =
						( jobjectArray )env->NewObjectArray( dDeviceCount,
						  								     env->FindClass( "java/lang/String" ),
															 env->NewStringUTF( "" ) );

				if ( jDriverList != NULL )
				{
					for ( int i = 0; i < dDeviceCount; i++ )
					{
						env->SetObjectArrayElement( jDriverList,
													i,
													env->NewStringUTF( pszDevList[ i ] ) );

						if ( env->ExceptionOccurred() != NULL )
						{
							pCArcClient.get()->FreeDeviceList();

							ThrowJNIException( env,
									"( GetDeviceList ): Failed to add driver name to list!" );
						}
					}
				}
				else
				{
					pCArcClient.get()->FreeDeviceList();

					ThrowJNIException( env,
									"( GetDeviceList ): Failed create array for driver list!" );
				}
			}

			pCArcClient.get()->FreeDeviceList();
		}
		catch ( std::exception &e )
		{
			pCArcClient.get()->FreeDeviceList();
			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			pCArcClient.get()->FreeDeviceList();
			ThrowUnknownJNIException( env );
		}
	}

	return jDriverList;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    OpenDeviceAPI
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_OpenDeviceAPI
( JNIEnv *env, jclass clazz, jstring driverName )
{
	//
	// Get the name of the device to open
	//
	const char* pszDeviceName =
		( driverName == NULL ? NULL : ( env )->GetStringUTFChars( driverName, 0 ) );

	if ( pszDeviceName != NULL )
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			try
			{
				CArcTools::CTokenizer cTokenizer;
				cTokenizer.Victim( pszDeviceName );

				string sDevice = cTokenizer.Next();

				if ( sDevice.compare( "PCI" ) == 0 )
				{
					CArcPCI::FindDevices();

					pCArcDev.reset( new CArcPCI() );
				}

				else if ( sDevice.compare( "PCIe" ) == 0 )
				{
					CArcPCIe::FindDevices();
						
					pCArcDev.reset( new CArcPCIe() );
				}

				else
				{
					throw std::runtime_error(
								"( OpenDeviceAPI ): Invalid device: " + sDevice );
				}

				cTokenizer.Next();		// Dump "Device" Text
					
				int dDeviceNumber = atoi( cTokenizer.Next().c_str() );
				pCArcDev.get()->Open( dDeviceNumber );
					
				// Free the allocated memory for the device name
				( env )->ReleaseStringUTFChars( driverName,
												pszDeviceName );
			}
			catch ( std::exception &e )
			{
				( env )->ReleaseStringUTFChars( driverName,
												pszDeviceName );
					
				ThrowJNIException( env, e.what() );
			}
			catch ( ... )
			{
				( env )->ReleaseStringUTFChars( driverName,
												pszDeviceName );
					
				ThrowUnknownJNIException( env );
			}
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			try
			{
				ASSERT_CLIENT_CLASS_PTR( env, "OpenDeviceAPI" )

				pCArcClient.get()->Open( pszDeviceName );

				// Free the allocated memory for the device name
				( env )->ReleaseStringUTFChars( driverName, pszDeviceName );
			}
			catch ( std::exception &e )
			{
				( env )->ReleaseStringUTFChars( driverName, pszDeviceName );

				ThrowJNIException( env, e.what() );
			}
			catch ( ... )
			{
				( env )->ReleaseStringUTFChars( driverName, pszDeviceName );

				ThrowUnknownJNIException( env );
			}
		}
	}
	else
	{
		ThrowJNIException( env,
				"( OpenDeviceAPI ): Failed to convert device name parameter!" );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    CloseDeviceAPI
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_CloseDeviceAPI( JNIEnv *env, jclass clazz )
{
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->Close();
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "CloseDeviceAPI" )

			pCArcClient.get()->Close();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    MapDeviceAPI
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_MapDeviceAPI
( JNIEnv *env, jclass clazz, jint bytes )
{
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->MapCommonBuffer( bytes );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "MapDeviceAPI" )

			pCArcClient.get()->MapCommonBuffer( bytes );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    RemapDeviceAPI
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_RemapDeviceAPI
( JNIEnv *env, jclass clazz, jint bytes )
{
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->ReMapCommonBuffer( bytes );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "RemapDeviceAPI" )

			pCArcClient.get()->ReMapCommonBuffer( bytes );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    UnmapDeviceAPI
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_UnmapDeviceAPI
( JNIEnv *env, jclass clazz )
{
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->UnMapCommonBuffer();
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "UnmapDeviceAPI" )

			pCArcClient.get()->UnMapCommonBuffer();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    IsDeviceOpen
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_owl_cameraAPI_CameraAPI_IsDeviceOpen( JNIEnv *env, jclass clazz )
{
	bool bDeviceOpen = false;
	
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			bDeviceOpen = pCArcDev.get()->IsOpen();
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "IsDeviceOpen" )

			bDeviceOpen = pCArcClient.get()->IsOpen();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return bDeviceOpen;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    IsCCD
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_owl_cameraAPI_CameraAPI_IsCCD( JNIEnv *env, jclass clazz )
{
	// +-------------------------------------------------+
	// |  Local API Interface                            |
	// +-------------------------------------------------+
	if ( !bRemoteAPI )
	{
		return pCArcDev.get()->IsCCD();
	}

	// +-------------------------------------------------+
	// |  Remote API Interface                           |
	// +-------------------------------------------------+
	else
	{
		try
		{
			ASSERT_CLIENT_CLASS_PTR( env, "IsCCD" )
		}
		catch ( exception& e )
		{
			ThrowJNIException( env, e.what() );
		}

		return pCArcClient.get()->IsCCD();
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetDeviceId
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetDeviceId( JNIEnv *env, jclass clazz )
{
	jint jId = 0;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			jId = jint( pCArcDev.get()->GetId() );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetDeviceId" )

			jId = jint( pCArcClient.get()->GetId() );
		}
	}
	catch ( exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jId;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetDeviceString
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_owl_cameraAPI_CameraAPI_GetDeviceString( JNIEnv *env, jclass clazz )
{
	// +-------------------------------------------------+
	// |  Local API Interface                            |
	// +-------------------------------------------------+
	if ( !bRemoteAPI )
	{
		return env->NewStringUTF( pCArcDev.get()->ToString() );
	}

	// +-------------------------------------------------+
	// |  Remote API Interface                           |
	// +-------------------------------------------------+
	else
	{
		try
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetDeviceString" )
		}
		catch ( exception& e )
		{
			ThrowJNIException( env, e.what() );
		}

		return env->NewStringUTF( pCArcClient.get()->ToString() );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetImageBufferVA
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL
Java_owl_cameraAPI_CameraAPI_GetImageBufferVA( JNIEnv *env, jclass clazz )
{
	// +-------------------------------------------------+
	// |  Local API Interface                            |
	// +-------------------------------------------------+
	if ( !bRemoteAPI )
	{
		return jlong( pCArcDev.get()->CommonBufferVA() );
	}

	// +-------------------------------------------------+
	// |  Remote API Interface                           |
	// +-------------------------------------------------+
	else
	{
		try
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetImageBufferVA" )
		}
		catch ( exception& e )
		{
			ThrowJNIException( env, e.what() );
		}

		return jlong( pCArcClient.get()->CommonBufferVA() );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetImageBufferPA
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL
Java_owl_cameraAPI_CameraAPI_GetImageBufferPA( JNIEnv *env, jclass clazz )
{
	// +-------------------------------------------------+
	// |  Local API Interface                            |
	// +-------------------------------------------------+
	if ( !bRemoteAPI )
	{
		return jlong( pCArcDev.get()->CommonBufferPA() );
	}

	// +-------------------------------------------------+
	// |  Remote API Interface                           |
	// +-------------------------------------------------+
	else
	{
		try
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetImageBufferPA" )
		}
		catch ( exception& e )
		{
			ThrowJNIException( env, e.what() );
		}

		return jlong( pCArcClient.get()->CommonBufferPA() );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetImageBufferSize
 * Signature: ()J
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetImageBufferSize( JNIEnv *env, jclass clazz )
{
	// +-------------------------------------------------+
	// |  Local API Interface                            |
	// +-------------------------------------------------+
	if ( !bRemoteAPI )
	{
		return pCArcDev.get()->CommonBufferSize();
	}

	// +-------------------------------------------------+
	// |  Remote API Interface                           |
	// +-------------------------------------------------+
	else
	{
		try
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetImageBufferSize" )
		}
		catch ( exception& e )
		{
			ThrowJNIException( env, e.what() );
		}

		return pCArcClient.get()->CommonBufferSize();
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    FillImageBuffer
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_FillImageBuffer( JNIEnv *env, jclass clazz, jint jFillValue )
{
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->FillCommonBuffer( ( unsigned short )jFillValue );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "FillImageBuffer" )

			pCArcClient.get()->FillCommonBuffer( ( unsigned short )jFillValue );
		}
	}
	catch ( exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    LoadFitsToImageBuffer
 * Signature: (Ljava/lang/String;)[I
 */
JNIEXPORT jintArray JNICALL
Java_owl_cameraAPI_CameraAPI_LoadFitsToImageBuffer( JNIEnv* pEnv, jclass jclazz, jstring jsFilename )
{
	jintArray jImgSizeArr = NULL;

	// Get the data filename
	const char* pszFilename = ( jsFilename == NULL ? NULL : pEnv->GetStringUTFChars( jsFilename, 0 ) );

	if ( pszFilename == NULL )
	{
		ThrowJNIException( pEnv,
				"( LoadFitsToImageBuffer ): Failed to convert filename parameter!" );
	}

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			auto_ptr<CFitsFile> pFitsFile( new CFitsFile( pszFilename ) );

			if ( pFitsFile.get() != NULL )
			{
				unsigned short* pFitsBuf =
								( unsigned short * )pFitsFile.get()->Read();

				unsigned short* pImgBuf =
						( unsigned short * )pCArcDev.get()->CommonBufferVA();

				if ( pFitsBuf != NULL && pImgBuf != NULL )
				{
					long lNaxes[ CFitsFile::NAXES_SIZE ] = { 0, 0, 0 };

					pFitsFile.get()->GetParameters( lNaxes );

					ptrdiff_t tPixCnt = ( lNaxes[ CFitsFile::NAXES_COL ] *
										  lNaxes[ CFitsFile::NAXES_ROW ] );

					uninitialized_copy( pFitsBuf,
										( pFitsBuf + tPixCnt ),
										pImgBuf );

					jImgSizeArr = pEnv->NewIntArray( 2 );

					if ( jImgSizeArr != NULL )
					{
						jint dImgSizeArr[ 2 ] = { int( lNaxes[ CFitsFile::NAXES_ROW ] ),
												  int( lNaxes[ CFitsFile::NAXES_COL ] ) };

						pEnv->SetIntArrayRegion( jImgSizeArr, 0, 2, dImgSizeArr );
					}
				}
			}
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		//else
		//{
		//	//ASSERT_CLIENT_CLASS_PTR( env, "FillImageBuffer" )

		//	//pCArcClient.get()->FillCommonBuffer( ( unsigned short )jFillValue );
		//}
	}
	catch ( exception &e )
	{
		ThrowJNIException( pEnv, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( pEnv );
	}

	return jImgSizeArr;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    SetAbort
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_SetAbort( JNIEnv *env, jclass clazz, jboolean jbOnOff )
{
	bAbort = ( jbOnOff != 0 );
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    LoadTmpCtrlFile
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_LoadTmpCtrlFile( JNIEnv *env, jclass clazz, jstring jFilename )
{
	// Get the data filename
	const char* pszFilename =
					( jFilename == NULL ? NULL : env->GetStringUTFChars( jFilename, 0 ) );

	if ( pszFilename == NULL )
	{
		ThrowJNIException( env,
					"( LoadTmpCtrlFile ): Failed to convert filename parameter!" );
	}
	
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->LoadTemperatureCtrlData( pszFilename );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "LoadTmpCtrlFile" )

			pCArcClient.get()->LoadTemperatureCtrlData( pszFilename );
		}

		// Free the allocated memory for the file name
		env->ReleaseStringUTFChars( jFilename, pszFilename );
	}
	catch ( exception &e )
	{
		env->ReleaseStringUTFChars( jFilename, pszFilename );
		
		ThrowJNIException( env,
						  string( "( LoadTmpCtrlFile ): " ) + e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    SaveTmpCtrlFile
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_SaveTmpCtrlFile( JNIEnv *env, jclass clazz, jstring jFilename )
{
	// Get the data filename
	const char* pszFilename =
					( jFilename == NULL ? NULL : env->GetStringUTFChars( jFilename, 0 ) );
	
	if ( pszFilename == NULL )
	{
		ThrowJNIException( env,
					"( SaveTmpCtrlFile ): Failed to convert filename parameter!" );
	}
	
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->SaveTemperatureCtrlData( pszFilename );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "SaveTmpCtrlFile" )

			pCArcClient.get()->SaveTemperatureCtrlData( pszFilename );
		}

		// Free the allocated memory for the file name
		env->ReleaseStringUTFChars( jFilename, pszFilename );
	}
	catch ( std::exception &e )
	{
		env->ReleaseStringUTFChars( jFilename, pszFilename );
		
		ThrowJNIException( env,
						  string( "( SaveTmpCtrlFile ): " ) + e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    LaunchDisplay
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_LaunchDisplay( JNIEnv *env, jclass clazz )
{
	try
	{
		if ( pCDisplay.get() == NULL )
		{
			pCDisplay.reset( new CDisplay() );
		}

		if ( pCDisplay.get() != NULL )
		{
			pCDisplay.get()->Launch();
		}
		else
		{
			ThrowJNIException( env,
				"( LaunchDisplay ): Failed to create CDisplay object!" );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    TerminateDisplay
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_TerminateDisplay( JNIEnv *env, jclass clazz )
{
	try
	{
		if ( pCDisplay.get() != NULL )
		{
			pCDisplay.get()->Terminate();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    DisplayImage
 * Signature: (II)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_DisplayImage( JNIEnv *env, jclass clazz, jint jdRows, jint jdCols )
{
	unsigned short* pBuffer = NULL;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			pBuffer =
				( unsigned short * )pCArcDev.get()->CommonBufferVA();
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "DisplayImage" )

			pBuffer = new unsigned short[ jdRows * jdCols ];

			int dBufSize =
					( jdRows * jdCols * sizeof( unsigned short ) );

			int dBytesRecvd =
					pCArcClient.get()->CommonBufferPixels( pBuffer,
														   dBufSize );

			if ( dBytesRecvd != int( dBufSize ) )
			{
				ostringstream oss;

				oss << "( DisplayImage ): Failed to receive enough pixels! Expected: "
					<< ( jdRows * jdCols * sizeof( unsigned short ) )
					<< " bytes - Received: " << dBytesRecvd << " bytes.";

				delete [] pBuffer;

				throw std::runtime_error( oss.str() );
			}
		}

		if ( pBuffer == NULL )
		{
			throw std::runtime_error( "( DisplayImage ): Image buffer is NULL!" );
		}

		// +-------------------------------------------------+
		// |  Display Image                                  |
		// +-------------------------------------------------+
		if ( pCDisplay.get() == NULL )
		{
			pCDisplay.reset( new CDisplay() );
		}
	
		if ( pCDisplay.get() != NULL )
		{
			pCDisplay.get()->Clear();
			pCDisplay.get()->Show( pBuffer, jdRows, jdCols );
		}
		else
		{
			if ( bRemoteAPI ) { delete [] pBuffer; }

			throw std::runtime_error(
						"( DisplayImage ): Failed to create CDisplay object!" );
		}
	}
	catch ( std::exception &e )
	{
		if ( bRemoteAPI ) { delete [] pBuffer; }

		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		if ( bRemoteAPI ) { delete [] pBuffer; }

		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    WriteBufferTextFile
 * Signature: (Ljava/lang/String;II)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_WriteBufferTextFile
( JNIEnv *env, jclass clazz, jstring jFilename, jint jdRows, jint jdCols )
{
	unsigned short *pBuffer = NULL;

	if ( jdRows <= 0 )
	{
		ThrowJNIException( env,
				"( WriteBufferTextFile ): Invalid row parameter! Must be > 0!" );
	}
	
	if ( jdCols <= 0 )
	{
		ThrowJNIException( env,
				"( WriteBufferTextFile ): Invalid column parameter! Must be > 0!" );
	}

	// +-------------------------------------------------+
	// |  Local API Interface                            |
	// +-------------------------------------------------+
	if ( !bRemoteAPI )
	{
		pBuffer = ( unsigned short * )pCArcDev.get()->CommonBufferVA();
	}

	// +-------------------------------------------------+
	// |  Remote API Interface                           |
	// +-------------------------------------------------+
	else
	{
		int dBytesRecvd = 0;

		try
		{
			ASSERT_CLIENT_CLASS_PTR( env, "WriteBufferTextFile" )

			pBuffer = new unsigned short[ jdRows * jdCols ];

			dBytesRecvd =
					pCArcClient.get()->CommonBufferPixels(
									 pBuffer,
									( jdRows * jdCols * sizeof( unsigned short ) ) );

			if ( dBytesRecvd != int( jdRows * jdCols * sizeof( unsigned short ) ) )
			{
				ostringstream oss;

				oss << "( WriteBufferTextFile ): Failed to read all pixels! "
					<< "Only read " << dBytesRecvd << " bytes ( "
					<< ( dBytesRecvd / sizeof( unsigned short ) )
					<< " pixels )!";

				delete [] pBuffer;

				throw std::runtime_error( oss.str() );
			}
		}
		catch ( std::bad_alloc &ba )
		{
			ThrowJNIException( env,
					string( "( WriteBufferTextFile ): Failed to allocate pixel buffer!\n" ) +
					ba.what() );
		}
		catch ( ... )
		{
			delete [] pBuffer;

			ThrowJNIException( env,
					"( WriteBufferTextFile ): Failed to read OR allocate pixel buffer!" );
		}
	}

	// +-------------------------------------------------+
	// |  Write The File                                 |
	// +-------------------------------------------------+
	if ( pBuffer != NULL )
	{
		//
		// Get the name of the device to open
		//
		const char* pszFilename =
				( jFilename == NULL ? NULL : ( env )->GetStringUTFChars( jFilename, 0 ) );
		
		if ( pszFilename == NULL )
		{
			if ( bRemoteAPI ) { delete [] pBuffer; }

			ThrowJNIException( env,
						"( WriteBufferTextFile ): Failed to access java filename!" );
		}

		ofstream ofs( pszFilename );

		if ( ofs.is_open() )
		{
			for ( int dPix=0; dPix<int( jdRows * jdCols ); dPix++ )
			{
				ofs << dec << dPix << "\t\t0x" << hex << uppercase
					<< pBuffer[ dPix ] << endl;
			}

			ofs.close();
		}

		if ( bRemoteAPI ) { delete [] pBuffer; }

		// Free the allocated memory for the device name
		( env )->ReleaseStringUTFChars( jFilename, pszFilename );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    LoadBufferTextFile
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_LoadBufferTextFile
( JNIEnv *env, jclass clazz, jstring jFilename )
{
	unsigned short* pData  = NULL;
	int             dTemp  = 0;
	int             dCount = 0;

	if ( bRemoteAPI )
	{
		try
		{
			ASSERT_CLIENT_CLASS_PTR( env, "LoadBufferTextFile" )
		}
		catch ( exception& e )
		{
			ThrowJNIException( env, e.what() );
		}
	}

	//
	// Get the data filename
	//
	const char* pszFilename =
			( jFilename == NULL ? NULL : env->GetStringUTFChars( jFilename, 0 ) );

	if ( pszFilename == NULL )
	{
		ThrowJNIException( env,
				"( LoadBufferTextFile ): Failed to convert filename ( parameter #1 )!" );
	}

	//
	// Open the data filestream
	//
	ifstream ifs( pszFilename );
	string   sLine;
	
	if ( !ifs )
	{
		string sMsg = "( LoadBufferTextFile ): Failed to open file: " +
						string( pszFilename );

		env->ReleaseStringUTFChars( jFilename, pszFilename );

		ThrowJNIException( env, sMsg );
	}

	// +-------------------------------------------------+
	// |  Local API Interface                            |
	// +-------------------------------------------------+
	if ( !bRemoteAPI )
	{
		//
		// Get a pointer to the kernel buffer as a temporary data buffer
		//
		pData = ( unsigned short * )pCArcDev.get()->CommonBufferVA();
	}

	// +-------------------------------------------------+
	// |  Remote API Interface                           |
	// +-------------------------------------------------+
	else
	{
		//
		// Determine the file size ( pixel count )
		//
		ifs.seekg( 0, ifstream::end );
		long size = ifs.tellg();
		ifs.seekg( 0 );

		try
		{
			pData = new unsigned short[ size ];

			pCArcClient.get()->CommonBufferPixels( pData, size );
		}
		catch ( std::bad_alloc &ba )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowJNIException( env,
					"( LoadBufferTextFile ): Failed to allocate pixel buffer!\n" +
					string( ba.what() ) );
		}
		catch ( ... )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowJNIException( env,
					"( LoadBufferTextFile ): Failed to read OR allocate pixel buffer!" );
		}
	}

	if ( pData == NULL )
	{
		env->ReleaseStringUTFChars( jFilename, pszFilename );

		ThrowJNIException( env,
				"( LoadBufferTextFile ): Failed to access image buffer!" );
	}

	while ( !ifs.eof() )
	{
		ifs >> dec >> dTemp >> hex >> pData[ dCount ];
		
		if ( ifs.fail() )
		{
			ifs.clear();
			getline( ifs, sLine );	// Read/throw away unwanted lines
			continue;
		}
		
		dCount++;
	}

	ifs.close();

	if ( bRemoteAPI )
	{
		delete [] pData;
	}

	// Free the allocated memory for the file name
	env->ReleaseStringUTFChars( jFilename, pszFilename );
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    ImageHistogram
 * Signature: (IIIII)[Ljava/awt/geom/Point2D/Float;
 */
JNIEXPORT jobjectArray JNICALL Java_owl_cameraAPI_CameraAPI_ImageHistogram
( JNIEnv *env, jclass clazz, jint row1, jint row2, jint col1, jint col2, jint rows, jint cols )
{
	jobjectArray jHistArr  = NULL;
	int*         pHist     = NULL;
	int          dHistSize = 0;
	int          index     = 0;
	CImage       cImg;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			if ( pCArcDev.get()->CommonBufferVA() != NULL )
			{
				pHist = cImg.Histogram( dHistSize,
										pCArcDev.get()->CommonBufferVA(),
										int( row1 ),
										int( row2 ),
										int( col1 ),
										int( col2 ),
										int( rows ),
										int( cols ),
										CImage::BPP16 );
			}
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "ImageHistogram" )

			arc::ulong ulData =
					arc::ulong( pCArcClient.get()->CommonBufferVA() );

			pHist = pCArcClient.get()->Histogram( dHistSize,
												  ( void * )ulData,
												  int( row1 ),
												  int( row2 ),
												  int( col1 ),
												  int( col2 ),
												  int( rows ),
												  int( cols ),
												  CImage::BPP16 );
		}
	}
	catch ( exception& e )
	{
		ThrowJNIException( env, e.what() );
	}

	if ( pHist == NULL )
	{
		ThrowJNIException( env,
						   "( ImageHistogram ): Failed to read image data!" );
	}

//	int elemCount = ( int )count_if( pHist, ( pHist + dHistSize ), NotZero );
	int elemCount = dHistSize;	// Force there to be 65535 elements for X-Y plotting
			
	jclass pointClass = env->FindClass( "java/awt/geom/Point2D$Float" );
			
	if ( pointClass == NULL )
	{
		if ( !bRemoteAPI )
		{
			cImg.FreeHistogram();
		}
		else
		{
			pCArcClient.get()->FreeHistogram();
		}

		ThrowJNIException( env, "( ImageHistogram ): Failed to find java Point2D.Float class!" );
	}
			
	jmethodID pointConstructor = env->GetMethodID( pointClass, "<init>", "(FF)V" );
			
	if ( pointConstructor == NULL )
	{
		if ( !bRemoteAPI )
		{
			cImg.FreeHistogram();
		}
		else
		{
			pCArcClient.get()->FreeHistogram();
		}

		ThrowJNIException( env, "( ImageHistogram ): Failed to find java Point2D.Float class constructor!" );
	}
			
	jHistArr = env->NewObjectArray( elemCount, pointClass, NULL );
			
	if ( jHistArr == NULL )
	{
		if ( !bRemoteAPI )
		{
			cImg.FreeHistogram();
		}
		else
		{
			pCArcClient.get()->FreeHistogram();
		}

		ThrowJNIException( env, "( ImageHistogram ): Failed to create new java object array!" );
	}
			
	for ( int i=0; i<dHistSize; i++ )
	{
//		if ( pHist[ i ] > 0 )
//		{
			jobject pointObj = env->NewObject( pointClass,
											   pointConstructor,
											   float( i ),
											   float( pHist[ i ] ) );

			if ( pointObj == NULL )
			{
				if ( !bRemoteAPI )
				{
					cImg.FreeHistogram();
				}
				else
				{
					pCArcClient.get()->FreeHistogram();
				}

				ThrowJNIException( env, "( ImageHistogram ): Failed to create new java Point2D.Float object!" );
			}
					
			env->SetObjectArrayElement( jHistArr, index, pointObj );
			index++;
//		}
	}
			
	if ( !bRemoteAPI )
	{
		cImg.FreeHistogram();
	}
	else
	{
		pCArcClient.get()->FreeHistogram();
	}
	
	return jHistArr;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetImageRow
 * Signature: (IIIII)[Ljava/awt/geom/Point2D/Float;
 */
JNIEXPORT jobjectArray JNICALL Java_owl_cameraAPI_CameraAPI_GetImageRow
( JNIEnv *env, jclass clazz, jint jlRow, jint jlCol1, jint jlCol2, jint jlRows, jint jlCols )
{
	jobjectArray    jPointArr   = NULL;
	ushort*			pBuf		= NULL;
	int             dElemCount  = 0;
	int             index       = 0;
	CImage          cImage;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			if ( pCArcDev.get()->CommonBufferVA() != NULL )
			{
				pBuf =
					( ushort * )cImage.GetImageRow( pCArcDev.get()->CommonBufferVA(),
													int( jlRow ),
													int( jlCol1 ),
													int( jlCol2 ),
													int( jlRows ),
													int( jlCols ),
													dElemCount );
			}
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetImageRow" )

			pBuf =
				( ushort * )pCArcClient.get()->GetImageRow( int( jlRow  ),
															int( jlCol1 ),
															int( jlCol2 ),
															int( jlRows ),
															int( jlCols ),
															dElemCount );
		}

		if ( pBuf == NULL )
		{
			ThrowJNIException( env,
				  "( GetImageRow ): Failed to allocate memory for image row data!" );
		}

		jclass pointClass = env->FindClass( "java/awt/geom/Point2D$Float" );
			
		if ( pointClass == NULL )
		{
			if ( !bRemoteAPI )
			{
				cImage.Free( pBuf );
			}
			else
			{
				pCArcClient.get()->FreeImageData( pBuf );
			}

			ThrowJNIException( env,
					"( GetImageRow ): Failed to find java Point2D.Float class!" );
		}
			
		jmethodID pointConstructor = env->GetMethodID( pointClass, "<init>", "(FF)V" );
			
		if ( pointConstructor == NULL )
		{
			if ( !bRemoteAPI )
			{
				cImage.Free( pBuf );
			}
			else
			{
				pCArcClient.get()->FreeImageData( pBuf );
			}

			ThrowJNIException( env,
					"( GetImageRow ): Failed to find java Point2D.Float class constructor!" );
		}
			
		jPointArr = env->NewObjectArray( jsize( dElemCount ), pointClass, NULL );
			
		if ( jPointArr == NULL )
		{
			if ( !bRemoteAPI )
			{
				cImage.Free( pBuf );
			}
			else
			{
				pCArcClient.get()->FreeImageData( pBuf );
			}

			ThrowJNIException( env,
					"( GetImageRow ): Failed to create new java object array!" );
		}
			
		for ( int i=0; i<dElemCount; i++ )
		{
			jobject pointObj = env->NewObject( pointClass,
											   pointConstructor,
											   float( jlCol1 + i ),
											   float( pBuf[ i ] ) );
				
			if ( pointObj == NULL )
			{
				if ( !bRemoteAPI )
				{
					cImage.Free( pBuf );
				}
				else
				{
					pCArcClient.get()->FreeImageData( pBuf );
				}

				ThrowJNIException( env,
					"( GetImageRow ): Failed to create new java Point2D.Float object!" );
			}
				
			env->SetObjectArrayElement( jPointArr, index, pointObj );
			index++;
		}
			
		if ( !bRemoteAPI )
		{
			cImage.Free( pBuf );
		}
		else
		{
			pCArcClient.get()->FreeImageData( pBuf );
		}
	}
	catch ( std::exception &e )
	{
		if ( pBuf != NULL )
		{
			if ( !bRemoteAPI )
			{
				cImage.Free( pBuf );
			}
			else
			{
				pCArcClient.get()->FreeImageData( pBuf );
			}
		}

		ThrowJNIException( env, e.what() );
	}

	return jPointArr;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetImageCol
 * Signature: (IIII)[Ljava/awt/geom/Point2D/Float;
 */
JNIEXPORT jobjectArray JNICALL Java_owl_cameraAPI_CameraAPI_GetImageCol
( JNIEnv *env, jclass clazz, jint jlCol, jint jlRow1, jint jlRow2, jint jlRows, jint jlCols )
{
	jobjectArray jPointArr  = NULL;
	ushort*      pBuf		= NULL;
	int          dElemCount = 0;
	int          index      = 0;
	CImage       cImage;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			if ( pCArcDev.get()->CommonBufferVA() != NULL )
			{
				pBuf = ( ushort * )
						cImage.GetImageCol( pCArcDev.get()->CommonBufferVA(),
											int( jlCol ),
											int( jlRow1 ),
											int( jlRow2 ),
											int( jlRows ),
											int( jlCols ),
											dElemCount );
			}
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetImageCol" )

			pBuf = ( ushort * )
					pCArcClient.get()->GetImageCol( int( jlCol  ),
													int( jlRow1 ),
													int( jlRow2 ),
													int( jlRows ),
													int( jlCols ),
													dElemCount );
		}

		if ( pBuf == NULL )
		{
			ThrowJNIException( env,
					"( GetImageCol ): Failed to allocate memory for image column data!" );
		}

		jclass pointClass = env->FindClass( "java/awt/geom/Point2D$Float" );
			
		if ( pointClass == NULL )
		{
			if ( !bRemoteAPI )
			{
				cImage.Free( pBuf );
			}
			else
			{
				pCArcClient.get()->FreeImageData( pBuf );
			}

			ThrowJNIException( env,
					"( GetImageCol ): Failed to find java Point2D.Float class!" );
		}
			
		jmethodID pointConstructor = env->GetMethodID( pointClass, "<init>", "(FF)V" );
			
		if ( pointConstructor == NULL )
		{
			if ( !bRemoteAPI )
			{
				cImage.Free( pBuf );
			}
			else
			{
				pCArcClient.get()->FreeImageData( pBuf );
			}

			ThrowJNIException( env,
					"( GetImageCol ): Failed to find java Point2D.Float class constructor!" );
		}
			
		jPointArr = env->NewObjectArray( jsize( dElemCount ), pointClass, NULL );
			
		if ( jPointArr == NULL )
		{
			if ( !bRemoteAPI )
			{
				cImage.Free( pBuf );
			}
			else
			{
				pCArcClient.get()->FreeImageData( pBuf );
			}

			ThrowJNIException( env,
					"( GetImageCol ): Failed to create new java object array!" );
		}
			
		for ( int i=0; i<dElemCount; i++ )
		{
			jobject pointObj = env->NewObject( pointClass,
											   pointConstructor,
											   float( jlRow1 + i ),
											   float( pBuf[ i ] ) );
				
			if ( pointObj == NULL )
			{
				if ( !bRemoteAPI )
				{
					cImage.Free( pBuf );
				}
				else
				{
					pCArcClient.get()->FreeImageData( pBuf );
				}

				ThrowJNIException( env,
						"( GetImageRow ): Failed to create new java Point2D.Float object!" );
			}
				
			env->SetObjectArrayElement( jPointArr, index, pointObj );
			index++;
		}
			
		if ( !bRemoteAPI )
		{
			cImage.Free( pBuf );
		}
		else
		{
			pCArcClient.get()->FreeImageData( pBuf );
		}
	}
	catch ( std::exception &e )
	{
		if ( pBuf != NULL )
		{
			if ( !bRemoteAPI )
			{
				cImage.Free( pBuf );
			}
			else
			{
				pCArcClient.get()->FreeImageData( pBuf );
			}
		}
			
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return jPointArr;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetImageRowArea
 * Signature: (IIIII)[Ljava/awt/geom/Point2D/Float;
 */
JNIEXPORT jobjectArray JNICALL Java_owl_cameraAPI_CameraAPI_GetImageRowArea
( JNIEnv *env, jclass clazz, jint jlRow1, jint jlRow2, jint jlCol1, jint jlCol2, jint jlRows, jint jlCols )
{
	jobjectArray    jPointArr   = NULL;
	int             dElemCount  = 0;
	int             index       = 0;

	// +-------------------------------------------------+
	// |  Local API Interface                            |
	// +-------------------------------------------------+
	if ( !bRemoteAPI )
	{
		float* pBuf = NULL;
		CImage  cImage;

		if ( pCArcDev.get()->CommonBufferVA() != NULL )
		{
			try
			{
				pBuf =
				 ( float * )cImage.GetImageRowArea( pCArcDev.get()->CommonBufferVA(),
													int( jlRow1 ),
													int( jlRow2 ),
													int( jlCol1 ),
													int( jlCol2 ),
													int( jlRows ),
													int( jlCols ),
													dElemCount );
				if ( pBuf == NULL )
				{
					ThrowJNIException( env,
								"( GetImageRowArea ): Failed to allocate memory for image row data!" );
				}
				
				jclass pointClass = env->FindClass( "java/awt/geom/Point2D$Float" );
				
				if ( pointClass == NULL )
				{
					cImage.Free( pBuf );

					ThrowJNIException( env,
								"( GetImageRowArea ): Failed to find java Point2D.Float class!" );
				}
				
				jmethodID pointConstructor = env->GetMethodID( pointClass, "<init>", "(FF)V" );
				
				if ( pointConstructor == NULL )
				{
					cImage.Free( pBuf );

					ThrowJNIException( env,
								"( GetImageRowArea ): Failed to find java Point2D.Float class constructor!" );
				}
				
				jPointArr = env->NewObjectArray( jsize( dElemCount ), pointClass, NULL );
				
				if ( jPointArr == NULL )
				{
					cImage.Free( pBuf );

					ThrowJNIException( env,
								"( GetImageRowArea ): Failed to create new java object array!" );
				}
				
				for ( int i=0; i<dElemCount; i++ )
				{
					jobject pointObj = env->NewObject( pointClass,
													   pointConstructor,
													   float( jlRow1 + i ),
													   float( pBuf[ i ] ) );

					if ( pointObj == NULL )
					{
						cImage.Free( pBuf );

						ThrowJNIException( env,
								"( GetImageRowArea ): Failed to create new java Point2D.Float object!" );
					}
					
					env->SetObjectArrayElement( jPointArr, index, pointObj );
					index++;
				}
				
				cImage.Free( pBuf );
			}
			catch ( std::exception &e )
			{
				if ( pBuf != NULL )
				{
					cImage.Free( pBuf );
				}
				
				ThrowJNIException( env, e.what() );
			}
			catch ( ... )
			{
				ThrowUnknownJNIException( env );
			}
		}
	}

	// +-------------------------------------------------+
	// |  Remote API Interface                           |
	// +-------------------------------------------------+
	else
	{
		float* pBuf = NULL;

		try
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetImageRowArea" )

			pBuf =
				( float * )pCArcClient.get()->GetImageRowArea( int( jlRow1 ),
															   int( jlRow2 ),
															   int( jlCol1 ),
															   int( jlCol2 ),
															   int( jlRows ),
															   int( jlCols ),
															   dElemCount );

			jclass pointClass = env->FindClass( "java/awt/geom/Point2D$Float" );

			if ( pointClass == NULL )
			{
				pCArcClient.get()->FreeImageData( pBuf, sizeof( float ) );

				ThrowJNIException( env,
						"( GetImageRowArea ): Failed to find java Point2D.Float class!" );
			}

			jmethodID pointConstructor = env->GetMethodID( pointClass, "<init>", "(FF)V" );

			if ( pointConstructor == NULL )
			{
				pCArcClient.get()->FreeImageData( pBuf, sizeof( float ) );

				ThrowJNIException( env,
						"( GetImageRowArea ): Failed to find java Point2D.Float class constructor!" );
			}

			jPointArr = env->NewObjectArray( jsize( dElemCount ), pointClass, NULL );

			if ( jPointArr == NULL )
			{
				pCArcClient.get()->FreeImageData( pBuf, sizeof( float ) );

				ThrowJNIException( env,
						"( GetImageRowArea ): Failed to create new java object array!" );
			}

			for ( int i=0; i<dElemCount; i++ )
			{
				jobject pointObj = env->NewObject( pointClass,
												   pointConstructor,
												   float( jlRow1 + i ),
												   float( pBuf[ i ] ) );

				if ( pointObj == NULL )
				{
					pCArcClient.get()->FreeImageData( pBuf, sizeof( float ) );

					ThrowJNIException( env,
						"( GetImageRowArea ): Failed to create new java Point2D.Float object!" );
				}

				env->SetObjectArrayElement( jPointArr, index, pointObj );
				index++;
			}

			pCArcClient.get()->FreeImageData( pBuf, sizeof( float ) );
		}
		catch ( std::exception &e )
		{
			pCArcClient.get()->FreeImageData( pBuf, sizeof( float ) );

			ThrowJNIException( env, e.what() );
		}
	}

	return jPointArr;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetImageColArea
 * Signature: (IIIII)[Ljava/awt/geom/Point2D/Float;
 */
JNIEXPORT jobjectArray JNICALL Java_owl_cameraAPI_CameraAPI_GetImageColArea
( JNIEnv *env, jclass clazz, jint jlRow1, jint jlRow2, jint jlCol1, jint jlCol2, jint jlRows, jint jlCols )
{
	jobjectArray    jPointArr   = NULL;
	int             dElemCount  = 0;
	int             index       = 0;

	// +-------------------------------------------------+
	// |  Local API Interface                            |
	// +-------------------------------------------------+
	if ( !bRemoteAPI )
	{
		float* pBuf = NULL;
		CImage  cImage;

		if ( pCArcDev.get()->CommonBufferVA() != NULL )
		{
			try
			{
				pBuf =
				 ( float * )cImage.GetImageColArea( pCArcDev.get()->CommonBufferVA(),
													int( jlRow1 ),
													int( jlRow2 ),
													int( jlCol1 ),
													int( jlCol2 ),
													int( jlRows ),
													int( jlCols ),
													dElemCount );
				if ( pBuf == NULL )
				{
					ThrowJNIException( env,
							"( GetImageColArea ): Failed to allocate memory for image column data!" );
				}
				
				jclass pointClass = env->FindClass( "java/awt/geom/Point2D$Float" );
				
				if ( pointClass == NULL )
				{
					cImage.Free( pBuf );

					ThrowJNIException( env,
							"( GetImageColArea ): Failed to find java Point2D.Float class!" );
				}
				
				jmethodID pointConstructor = env->GetMethodID( pointClass, "<init>", "(FF)V" );
				
				if ( pointConstructor == NULL )
				{
					cImage.Free( pBuf );

					ThrowJNIException( env,
							"( GetImageColArea ): Failed to find java Point2D.Float class constructor!" );
				}
				
				jPointArr = env->NewObjectArray( jsize( dElemCount ), pointClass, NULL );
				
				if ( jPointArr == NULL )
				{
					cImage.Free( pBuf );

					ThrowJNIException( env,
							"( GetImageColArea ): Failed to create new java object array!" );
				}

				for ( int i=0; i<dElemCount; i++ )
				{
					jobject pointObj = env->NewObject( pointClass,
													   pointConstructor,
													   float( jlCol1 + i ),
													   float( pBuf[ i ] ) );
					
					if ( pointObj == NULL )
					{
						cImage.Free( pBuf );

						ThrowJNIException( env,
							"( GetImageColArea ): Failed to create new java Point2D.Float object!" );
					}
					
					env->SetObjectArrayElement( jPointArr, index, pointObj );

					index++;
				}

				cImage.Free( pBuf );
			}
			catch ( std::exception &e )
			{
				if ( pBuf != NULL )
				{
					cImage.Free( pBuf );
				}
				
				ThrowJNIException( env, e.what() );
			}
			catch ( ... )
			{
				ThrowUnknownJNIException( env );
			}
		}
	}

	// +-------------------------------------------------+
	// |  Remote API Interface                           |
	// +-------------------------------------------------+
	else
	{
		float* pBuf = NULL;

		try
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetImageColArea" )

			pBuf =
				( float * )pCArcClient.get()->GetImageColArea( int( jlRow1 ),
															   int( jlRow2 ),
															   int( jlCol1 ),
															   int( jlCol2 ),
															   int( jlRows ),
															   int( jlCols ),
															   dElemCount );

			jclass pointClass = env->FindClass( "java/awt/geom/Point2D$Float" );

			if ( pointClass == NULL )
			{
				pCArcClient.get()->FreeImageData( pBuf, sizeof( float ) );

				ThrowJNIException( env,
					"( GetImageColArea ): Failed to find java Point2D.Float class!" );
			}

			jmethodID pointConstructor = env->GetMethodID( pointClass, "<init>", "(FF)V" );

			if ( pointConstructor == NULL )
			{
				pCArcClient.get()->FreeImageData( pBuf, sizeof( float ) );

				ThrowJNIException( env,
					"( GetImageColArea ): Failed to find java Point2D.Float class constructor!" );
			}

			jPointArr = env->NewObjectArray( jsize( dElemCount ), pointClass, NULL );

			if ( jPointArr == NULL )
			{
				pCArcClient.get()->FreeImageData( pBuf, sizeof( float ) );

				ThrowJNIException( env,
					"( GetImageColArea ): Failed to create new java object array!" );
			}

			for ( int i=0; i<dElemCount; i++ )
			{
				jobject pointObj = env->NewObject( pointClass,
												   pointConstructor,
												   float( jlCol1 + i ),
												   float( pBuf[ i ] ) );

				if ( pointObj == NULL )
				{
					pCArcClient.get()->FreeImageData( pBuf, sizeof( float ) );

					ThrowJNIException( env,
						"( GetImageColArea ): Failed to create new java Point2D.Float object!" );
				}

				env->SetObjectArrayElement( jPointArr, index, pointObj );
				index++;
			}

			pCArcClient.get()->FreeImageData( pBuf, sizeof( float ) );
		}
		catch ( std::exception &e )
		{
			pCArcClient.get()->FreeImageData( pBuf, sizeof( float ) );

			ThrowJNIException( env, e.what() );
		}
	}

	return jPointArr;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetImageStats
 * Signature: (IIIIII)Lowl/img/analysis/ImageStats;
 */
JNIEXPORT jobject JNICALL Java_owl_cameraAPI_CameraAPI_GetImageStats__IIIIII
( JNIEnv *env, jclass clazz, jint jlRow1, jint jlRow2, jint jlCol1, jint jlCol2, jint jlRows, jint jlCols )
{
	jobject jImgStatsObj = NULL;
	CImage  cImg;

	CImage::CImgStats cImgStats;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			//  Calculate stats
			// +-----------------------------------------------------+
			cImgStats = cImg.GetStats( pCArcDev.get()->CommonBufferVA(),
									   int( jlRow1 ),
									   int( jlRow2 ),
									   int( jlCol1 ),
									   int( jlCol2 ),
									   int( jlRows ),
									   int( jlCols ) );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetImageStats" )

			arc::ulong ulData =
					arc::ulong( pCArcClient.get()->CommonBufferVA() );

			//  Calculate stats
			// +-----------------------------------------------------+
			cImgStats =
					pCArcClient.get()->GetStats( ( void * )ulData,
												 int( jlRow1 ),
												 int( jlRow2 ),
												 int( jlCol1 ),
												 int( jlCol2 ),
												 int( jlRows ),
												 int( jlCols ) );
		}

		// +-------------------------------------------------------------------------------------+
		jclass jImgStatClass = env->FindClass( "owl/img/analysis/ImageStats" );
			
		if ( jImgStatClass == NULL )
		{
			throw std::runtime_error(
						"( GetImageStats ): Failed to find java ImageStats class!" );
		}
			
		jmethodID jImgStatInitMthd = env->GetMethodID( jImgStatClass, "<init>", "()V" );
			
		if ( jImgStatInitMthd == NULL )
		{
			throw std::runtime_error(
						"( GetImageStats ): Failed to find java ImageStats class constructor!" );
		}
			
		jImgStatsObj = env->NewObject( jImgStatClass, jImgStatInitMthd );
			
		if ( jImgStatsObj == NULL )
		{
			throw std::runtime_error(
						"( GetImageStats ): Failed to create new java ImageStats object!" );
		}
			
		//  Image statistics
		// +-------------------------------------------------------------------------------------+
		jfieldID jImgFieldId = env->GetFieldID( jImgStatClass, "gMin", "D" );
		env->SetDoubleField( jImgStatsObj, jImgFieldId, cImgStats.gMin );
			
		jImgFieldId = env->GetFieldID( jImgStatClass, "gMax", "D" );
		env->SetDoubleField( jImgStatsObj, jImgFieldId, cImgStats.gMax );
			
		jImgFieldId = env->GetFieldID( jImgStatClass, "gMean", "D" );
		env->SetDoubleField( jImgStatsObj, jImgFieldId, cImgStats.gMean );
			
		jImgFieldId = env->GetFieldID( jImgStatClass, "gVariance", "D" );
		env->SetDoubleField( jImgStatsObj, jImgFieldId, cImgStats.gVariance );
			
		jImgFieldId = env->GetFieldID( jImgStatClass, "gStdDev", "D" );
		env->SetDoubleField( jImgStatsObj, jImgFieldId, cImgStats.gStdDev );
			
		jImgFieldId = env->GetFieldID( jImgStatClass, "gSaturatedPixCnt", "D" );
		env->SetDoubleField( jImgStatsObj, jImgFieldId, cImgStats.gSaturatedPixCnt );
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return jImgStatsObj;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetImageStats
 * Signature: (Ljava/lang/String;IIII)Lowl/img/analysis/ImageStats;
 */
JNIEXPORT jobject JNICALL Java_owl_cameraAPI_CameraAPI_GetImageStats__Ljava_lang_String_2IIII
( JNIEnv *env, jclass clazz, jstring jFitsFile, jint jlRow1, jint jlRow2, jint jlCol1, jint jlCol2 )
{
	jobject jImgStatsObj = NULL;
	CImage::CImgStats cImgStats;

	const char* pszFilename = ( jFitsFile == NULL ? NULL : env->GetStringUTFChars( jFitsFile, 0 ) );

	try
	{
		if ( pszFilename == NULL )
		{
			throw std::runtime_error(
					"( GetImageStats ): Failed to convert FITS filename ( parameter #1 )!" );
		}

		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			void *pBuffer = NULL;
			
			//  Open the FITS file #1 and read the image data
			// +-----------------------------------------------------+
			CFitsFile cFitsFile( pszFilename );
			
			pBuffer = cFitsFile.ReadSubImage( int( jlRow1 ),
											  int( jlCol1 ),
											  int( jlRow2 ),
											  int( jlCol2 ) );

			if ( pBuffer == NULL )
			{
				ostringstream oss;

				oss << "( GetImageStats ): Failed to read image data from FITS: "
					<< pszFilename
					<< ends;

				env->ReleaseStringUTFChars( jFitsFile, pszFilename );

				throw std::runtime_error( oss.str() );
			}

			CImage cImage;

			cImgStats = cImage.GetStats( pBuffer,
										 int( jlRow2 - jlRow1 ),
										 int( jlCol2 - jlCol1 ) );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetImageStats" )

			cImgStats = pCArcClient.get()->GetStats( pszFilename,
													 int( jlRow1 ),
													 int( jlRow2 ),
													 int( jlCol1 ),
													 int( jlCol2 ) );
		}

		//  Get access to Owl's image statistics classes
		// +-------------------------------------------------------------------------------------+
		jclass jImgStatClass = env->FindClass( "owl/img/analysis/ImageStats" );
		
		if ( jImgStatClass == NULL )
		{
			env->ReleaseStringUTFChars( jFitsFile, pszFilename );

			throw std::runtime_error(
						"( GetImageStats ): Failed to find java ImageStats class!" );
		}
		
		jmethodID jImgStatInitMthd = env->GetMethodID( jImgStatClass, "<init>", "()V" );
		
		if ( jImgStatInitMthd == NULL )
		{
			env->ReleaseStringUTFChars( jFitsFile, pszFilename );

			throw std::runtime_error(
						"( GetImageStats ): Failed to find java ImageStats class constructor!" );
		}
		
		jImgStatsObj = env->NewObject( jImgStatClass, jImgStatInitMthd );
		
		if ( jImgStatsObj == NULL )
		{
			env->ReleaseStringUTFChars( jFitsFile, pszFilename );

			throw std::runtime_error(
						"( GetImageStats ): Failed to create new java ImageStats object!" );
		}
		
		//  Image statistics
		// +-------------------------------------------------------------------------------------+
		jfieldID jImgFieldId = env->GetFieldID( jImgStatClass, "gMin", "D" );
		env->SetDoubleField( jImgStatsObj, jImgFieldId, cImgStats.gMin );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gMax", "D" );
		env->SetDoubleField( jImgStatsObj, jImgFieldId, cImgStats.gMax );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gMean", "D" );
		env->SetDoubleField( jImgStatsObj, jImgFieldId, cImgStats.gMean );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gVariance", "D" );
		env->SetDoubleField( jImgStatsObj, jImgFieldId, cImgStats.gVariance );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gStdDev", "D" );
		env->SetDoubleField( jImgStatsObj, jImgFieldId, cImgStats.gStdDev );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gSaturatedPixCnt", "D" );
		env->SetDoubleField( jImgStatsObj, jImgFieldId, cImgStats.gSaturatedPixCnt );
		
		// Free the allocated memory for the file name
		env->ReleaseStringUTFChars( jFitsFile, pszFilename );
	}
	catch ( std::exception &e )
	{
		if ( pszFilename != NULL )
		{
			env->ReleaseStringUTFChars( jFitsFile, pszFilename );
		}

		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jImgStatsObj;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetImageDifStats
 * Signature: (Ljava/lang/String;Ljava/lang/String;IIII)Lowl/img/analysis/ImageDifStats;
 */
JNIEXPORT jobject JNICALL Java_owl_cameraAPI_CameraAPI_GetImageDifStats
( JNIEnv *env, jclass clazz, jstring jFitsFile1, jstring jFitsFile2, jint jlRow1, jint jlRow2, jint jlCol1, jint jlCol2 )
{
	jobject jImgDifStatsObj = NULL;
	CImage::CImgDifStats cImgStats;

	const char *pszFilename1 = ( jFitsFile1 == NULL ? NULL : env->GetStringUTFChars( jFitsFile1, 0 ) );
	const char *pszFilename2 = ( jFitsFile2 == NULL ? NULL : env->GetStringUTFChars( jFitsFile2, 0 ) );
	
	try
	{
		if ( pszFilename1 == NULL )
		{
			throw std::runtime_error(
					"( GetImageDifStats ): Failed to convert FITS filename ( parameter #1 )!" );
		}
		
		if ( pszFilename2 == NULL )
		{
			throw std::runtime_error(
					"( GetImageDifStats ): Failed to convert FITS filename ( parameter #2 )!" );
		}
	
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			void *pBuffer1 = NULL, *pBuffer2 = NULL;
			
			//  Open the FITS file #1 and read the image data
			// +-----------------------------------------------------+
			CFitsFile cFitsFile1( pszFilename1 );
			
			pBuffer1 = cFitsFile1.ReadSubImage( int( jlRow1 ),
												int( jlCol1 ),
												int( jlRow2 ),
												int( jlCol2 ) );
			
			if ( pBuffer1 == NULL )
			{
				ostringstream oss;

				oss << "( GetImageDiffStats ): Failed to read image data from FITS: "
					<< pszFilename1
					<< ends;

				throw std::runtime_error( oss.str() );
			}
			
			//  Open the FITS file #2 and read the image data
			// +-----------------------------------------------------+
			CFitsFile cFitsFile2( pszFilename2 );
			
			pBuffer2 = cFitsFile2.ReadSubImage( int( jlRow1 ),
												int( jlCol1 ),
												int( jlRow2 ),
												int( jlCol2 ) );
			
			if ( pBuffer2 == NULL )
			{
				ostringstream oss;

				oss << "( GetImageDiffStats ): Failed to read image data from FITS: "
					<< pszFilename2
					<< ends;

				throw std::runtime_error( oss.str() );
			}
			
			CImage cImage;

			cImgStats = cImage.GetDiffStats( pBuffer1,
											 pBuffer2,
											 int( jlRow2 - jlRow1 ),
											 int( jlCol2 - jlCol1 ) );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetImageDiffStats" )

			cImgStats = pCArcClient.get()->GetDiffStats( pszFilename1,
												   pszFilename2,
												   int( jlRow1 ),
												   int( jlRow2 ),
												   int( jlCol1 ),
												   int( jlCol2 ) );
		}

		//  Get access to Owl's image statistics classes
		// +-------------------------------------------------------------------------------------+
		jclass jImgStatClass = env->FindClass( "owl/img/analysis/ImageStats" );
		
		if ( jImgStatClass == NULL )
		{
			throw std::runtime_error(
					"( GetImageDiffStats ): Failed to find java ImageStats class!" );
		}
		
		jclass jImgDifStatClass = env->FindClass( "owl/img/analysis/ImageDifStats" );
		
		if ( jImgDifStatClass == NULL )
		{
			throw std::runtime_error(
					"( GetImageDiffStats ): Failed to find java ImageDifStats class!" );
		}
		
		jmethodID jImgStatInitMthd = env->GetMethodID( jImgDifStatClass, "<init>", "()V" );
		
		if ( jImgStatInitMthd == NULL )
		{
			throw std::runtime_error(
					"( GetImageDiffStats ): Failed to find java ImageDifStats class constructor!" );
		}
		
		jImgDifStatsObj = env->NewObject( jImgDifStatClass, jImgStatInitMthd );
		
		if ( jImgDifStatsObj == NULL )
		{
			throw std::runtime_error(
					"( GetImageDiffStats ): Failed to create new java ImageDifStats object!" );
		}
		
		//  Image #1 statistics
		// +-------------------------------------------------------------------------------------+
		jfieldID jImgId = env->GetFieldID( jImgDifStatClass, "img1Stats", "Lowl/img/analysis/ImageStats;" );
		jobject jImgObj = env->GetObjectField( jImgDifStatsObj, jImgId );
		
		jfieldID jImgFieldId = env->GetFieldID( jImgStatClass, "gMin", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImg1Stats.gMin );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gMax", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImg1Stats.gMax );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gMean", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImg1Stats.gMean );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gVariance", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImg1Stats.gVariance );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gStdDev", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImg1Stats.gStdDev );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gSaturatedPixCnt", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImg1Stats.gSaturatedPixCnt );
		
		//  Image #2 statistics
		// +-------------------------------------------------------------------------------------+
		jImgId = env->GetFieldID( jImgDifStatClass, "img2Stats", "Lowl/img/analysis/ImageStats;" );
		jImgObj = env->GetObjectField( jImgDifStatsObj, jImgId );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gMin", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImg2Stats.gMin );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gMax", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImg2Stats.gMax );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gMean", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImg2Stats.gMean );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gVariance", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImg2Stats.gVariance );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gStdDev", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImg2Stats.gStdDev );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gSaturatedPixCnt", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImg2Stats.gSaturatedPixCnt );
		
		//  Image Dif statistics
		// +-------------------------------------------------------------------------------------+
		jImgId = env->GetFieldID( jImgDifStatClass, "imgDifStats", "Lowl/img/analysis/ImageStats;" );
		jImgObj = env->GetObjectField( jImgDifStatsObj, jImgId );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gMin", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImgDiffStats.gMin );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gMax", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImgDiffStats.gMax );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gMean", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImgDiffStats.gMean );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gVariance", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImgDiffStats.gVariance );
		
		jImgFieldId = env->GetFieldID( jImgStatClass, "gStdDev", "D" );
		env->SetDoubleField( jImgObj, jImgFieldId, cImgStats.cImgDiffStats.gStdDev );
		
		// Free the allocated memory for the file name
		env->ReleaseStringUTFChars( jFitsFile1, pszFilename1 );
		env->ReleaseStringUTFChars( jFitsFile2, pszFilename2 );
	}
	catch ( std::exception &e )
	{
		if ( pszFilename1 != NULL )
		{
			env->ReleaseStringUTFChars( jFitsFile1, pszFilename1 );
		}

		if ( pszFilename2 != NULL )
		{
			env->ReleaseStringUTFChars( jFitsFile2, pszFilename2 );
		}

		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		if ( pszFilename1 != NULL )
		{
			env->ReleaseStringUTFChars( jFitsFile1, pszFilename1 );
		}

		if ( pszFilename2 != NULL )
		{
			env->ReleaseStringUTFChars( jFitsFile2, pszFilename2 );
		}
		
		ThrowJNIException( env,
				"( GetImageDiffStats ): Yikes! An unknown system exception occurred!" );
	}
	
	return jImgDifStatsObj;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    PixelSearch
 * Signature: (I)[Ljava/awt/Point;
 */
JNIEXPORT jobjectArray JNICALL
Java_owl_cameraAPI_CameraAPI_PixelSearch( JNIEnv *env, jclass clazz, jint jValue )
{
	jobjectArray    jPointArr   = NULL;
	unsigned short* pBuf		= NULL;
	const int       dElemCount  = 50;
	int             index       = 0;

	if ( pCArcDev.get()->CommonBufferVA() != NULL )
	{
		try
		{
			pBuf = ( unsigned short * )pCArcDev.get()->CommonBufferVA();

			jclass pointClass = env->FindClass( "java/awt/Point" );
			
			if ( pointClass == NULL )
			{
				ThrowJNIException( env,
							"( PixelSearch ): Failed to find java Point class!" );
			}
			
			jmethodID pointConstructor = env->GetMethodID( pointClass, "<init>", "(II)V" );
			
			if ( pointConstructor == NULL )
			{
				ThrowJNIException( env,
							"( PixelSearch ): Failed to find java Point class constructor!" );
			}
			
			jPointArr = env->NewObjectArray( jsize( dElemCount ), pointClass, NULL );
			
			if ( jPointArr == NULL )
			{
				ThrowJNIException( env,
							"( PixelSearch ): Failed to create new java object array!" );
			}

			int dCols = pCArcDev.get()->GetImageCols();

			for ( int dRow=0; dRow<pCArcDev.get()->GetImageRows(); dRow++ )
			{
				for ( int dCol=0; dCol<pCArcDev.get()->GetImageCols(); dCol++ )
				{
					if ( pBuf[ dCol + dRow * dCols ] == ( unsigned short )jValue )
					{
						jobject pointObj = env->NewObject( pointClass,
														   pointConstructor,
														   dCol,
														   dRow );
						
						if ( pointObj == NULL )
						{
							ThrowJNIException( env,
									"( PixelSearch ): Failed to create new java Point object!" );
						}
						
						env->SetObjectArrayElement( jPointArr, index, pointObj );
						index++;
					}

					if ( index >= dElemCount ) { break; }
				}

				if ( index >= dElemCount ) { break; }
			}
		}
		catch ( std::exception &e )
		{
			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			ThrowUnknownJNIException( env );
		}
	}
	
	return jPointArr;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    ClearDisplay
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_ClearDisplay
( JNIEnv *env, jclass clazz )
{
	try
	{
		if ( pCDisplay.get() != NULL )
		{
			pCDisplay.get()->Clear();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    FreeDisplay
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_FreeDisplay
( JNIEnv *env, jclass clazz )
{
	try
	{
		pCDisplay.reset();

		//if ( pCDisplay != NULL )
		//{
		//	delete pCDisplay;
		//}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    ClearDeviceStatusAPI
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_ClearDeviceStatusAPI( JNIEnv *env, jclass clazz )
{
	try
	{
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->ClearStatus();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "ClearDeviceStatusAPI" )

			pCArcClient.get()->ClearStatus();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetDeviceStatusAPI
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetDeviceStatusAPI( JNIEnv *env, jclass clazz )
{
	int dRetVal = 0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			dRetVal = pCArcDev.get()->GetStatus();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetDeviceStatsAPI" )

			dRetVal = pCArcClient.get()->GetStatus();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return dRetVal;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    CommandAPI
 * Signature: (IIIIII)I
 */
JNIEXPORT jint JNICALL Java_owl_cameraAPI_CameraAPI_CommandAPI
(JNIEnv *env, jclass clazz, jint boardId, jint cmd, jint arg1, jint arg2, jint arg3, jint arg4 )
{
	int dRetVal = 0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			dRetVal = pCArcDev.get()->Command( boardId,
											   cmd,
											   arg1,
											   arg2,
											   arg3,
											   arg4 );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "CommandAPI" )

			dRetVal = pCArcClient.get()->Command( boardId,
												  cmd,
												  arg1,
												  arg2,
												  arg3,
												  arg4 );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return dRetVal;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetPixelCountAPI
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetPixelCountAPI( JNIEnv *env, jclass clazz )
{
	int dRetVal = 0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			dRetVal = pCArcDev.get()->GetPixelCount();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetPixelCountAPI" )

			dRetVal = pCArcClient.get()->GetPixelCount();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return dRetVal;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetCRPixelCountAPI
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetCRPixelCountAPI( JNIEnv *env, jclass clazz )
{
	int dRetVal = 0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			dRetVal = pCArcDev.get()->GetCRPixelCount();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetCRPixelCountAPI" )

			dRetVal = pCArcClient.get()->GetCRPixelCount();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return dRetVal;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetFrameCountAPI
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_owl_cameraAPI_CameraAPI_GetFrameCountAPI( JNIEnv *env, jclass clazz )
{
	int dRetVal = 0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			dRetVal = pCArcDev.get()->GetFrameCount();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetFrameCountAPI" )

			dRetVal = pCArcClient.get()->GetFrameCount();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return dRetVal;
}

JNIEXPORT jboolean JNICALL
Java_owl_cameraAPI_CameraAPI_IsReadoutAPI( JNIEnv *env, jclass clazz )
{
	bool bIsReadout = false;
	
	try
	{
		if ( !bRemoteAPI )
		{
			bIsReadout = pCArcDev.get()->IsReadout();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "IsReadoutAPI" )

			bIsReadout = pCArcClient.get()->IsReadout();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return bIsReadout;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    LoadDeviceFileAPI
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_LoadDeviceFileAPI( JNIEnv *env, jclass clazz, jstring jFilename )
{
	//
	// Get the filename
	//
	const char* pszFilename = ( jFilename == NULL ? NULL : env->GetStringUTFChars( jFilename, 0 ) );
	
	if ( pszFilename != NULL )
	{
		try
		{
			if ( !bRemoteAPI )
			{
				pCArcDev.get()->LoadDeviceFile( ( const char * )pszFilename );
			}
			else
			{
				ASSERT_CLIENT_CLASS_PTR( env, "LoadDeviceFileAPI" )

				pCArcClient.get()->LoadDeviceFile( ( const char * )pszFilename );
			}
			
			//
			// Free the allocated memory for the filename
			//
			env->ReleaseStringUTFChars( jFilename, pszFilename );
		}
		catch ( std::exception &e )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );
			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );
			ThrowUnknownJNIException( env );
		}
	}
	else
	{
		ThrowJNIException( env,
					"( LoadPCIFileAPI ): Failed to create filename parameter!" );
	}
}

JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_LoadControllerFileAPI( JNIEnv *env, jclass clazz, jstring jFilename, jint validate )
{
	//
	// Get the filename
	//
	const char* pszFilename = ( jFilename == NULL ? NULL : env->GetStringUTFChars( jFilename, 0 ) );

	if ( pszFilename != NULL )
	{
		try
		{
			if ( !bRemoteAPI )
			{
				pCArcDev.get()->LoadControllerFile( ( const char * )pszFilename,
											  ( validate == 1 ? true : false ),
											  bAbort );
			}
			else
			{
				ASSERT_CLIENT_CLASS_PTR( env, "LoadControllerFileAPI" )

				pCArcClient.get()->LoadControllerFile( ( const char * )pszFilename,
												 ( validate == 1 ? true : false ) );
			}

			//
			// Free the allocated memory for the filename
			//
			env->ReleaseStringUTFChars( jFilename, pszFilename );
		}
		catch ( std::exception &e )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );
			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );
			ThrowUnknownJNIException( env );
		}
	}
	else
	{
		ThrowJNIException( env,
					"( LoadControllerFileAPI ): Failed to create filename parameter!" );
	}
}

JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_SetImageSizeAPI(JNIEnv *env, jclass clazz, jint rows, jint cols )
{
	try
	{
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->SetImageSize( rows, cols );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "SetImageSizeAPI" )

			pCArcClient.get()->SetImageSize( rows, cols );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetImageRowsAPI( JNIEnv *env, jclass clazz )
{
	int dRows = 0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			dRows = pCArcDev.get()->GetImageRows();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetImageRowsAPI" )

			dRows = pCArcClient.get()->GetImageRows();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return dRows;
}

JNIEXPORT jint
JNICALL Java_owl_cameraAPI_CameraAPI_GetImageColsAPI( JNIEnv *env, jclass clazz)
{
	int dCols = 0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			dCols = pCArcDev.get()->GetImageCols();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetImageColsAPI" )

			dCols = pCArcClient.get()->GetImageCols();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return dCols;
}

JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetCCParamsAPI(JNIEnv *env, jclass clazz )
{
	int dCcpVal = 0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			dCcpVal = pCArcDev.get()->GetCCParams();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetCCParamsAPI" )

			dCcpVal = pCArcClient.get()->GetCCParams();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return dCcpVal;
}

JNIEXPORT jboolean JNICALL
Java_owl_cameraAPI_CameraAPI_IsCCParamSupportedAPI( JNIEnv *env, jclass clazz, jint parameter )
{
	bool bIsSupported = false;
	
	try
	{
		if ( !bRemoteAPI )
		{
			bIsSupported = pCArcDev.get()->IsCCParamSupported( parameter );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "IsCCParamSupportedAPI" )

			bIsSupported = pCArcClient.get()->IsCCParamSupported( parameter );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return bIsSupported;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    IsControllerConnectedAPI
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_owl_cameraAPI_CameraAPI_IsControllerConnectedAPI( JNIEnv *env, jclass clazz )
{
	bool bResult = false;

	try
	{
		if ( !bRemoteAPI )
		{
			bResult = pCArcDev.get()->IsControllerConnected();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "IsControllerConnectedAPI" )

			bResult = pCArcClient.get()->IsControllerConnected();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jboolean( bResult );
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetArrayTemperatureAPI
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL
Java_owl_cameraAPI_CameraAPI_GetArrayTemperatureAPI( JNIEnv *env, jclass clazz )
{
	double fAvgTemp = 0.0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			fAvgTemp = pCArcDev.get()->GetArrayTemperature();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetArrayTemperatureAPI" )

			fAvgTemp = pCArcClient.get()->GetArrayTemperature();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return fAvgTemp;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetArrayTemperatureDNAPI
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL
Java_owl_cameraAPI_CameraAPI_GetArrayTemperatureDNAPI( JNIEnv *env, jclass clazz )
{
	double gDn = 0.0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			gDn = pCArcDev.get()->GetArrayTemperatureDN();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetArrayTemperatureDNAPI" )

			gDn = pCArcClient.get()->GetArrayTemperatureDN();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return gDn;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    SetArrayTemperatureAPI
 * Signature: (D)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_SetArrayTemperatureAPI( JNIEnv *env, jclass clazz, jdouble tempVal )
{
	try
	{
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->SetArrayTemperature( tempVal );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "SetArrayTemperatureAPI" )

			pCArcClient.get()->SetArrayTemperature( tempVal );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    SetBinningAPI
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_SetBinningAPI
( JNIEnv *env, jclass clazz, jint jRows, jint jCols, jint jRowFactor, jint jColFactor )
{
	try
	{
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->SetBinning( jRows,
								  jCols,
								  jRowFactor,
								  jColFactor );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "SetBinningAPI" )

			pCArcClient.get()->SetBinning( jRows,
									 jCols,
									 jRowFactor,
									 jColFactor );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    UnsetBinningAPI
 * Signature: (II)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_UnsetBinningAPI( JNIEnv *env, jclass clazz, jint jRows, jint jCols )
{
	try
	{
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->UnSetBinning( jRows, jCols );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "UnSetBinningAPI" )

			pCArcClient.get()->UnSetBinning( jRows, jCols );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    SetOpenShutterAPI
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_SetOpenShutterAPI( JNIEnv *env, jclass clazz, jboolean jShouldOpen )
{
	try
	{
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->SetOpenShutter( ( jShouldOpen == 1 ? true : false ) );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "SetOpenShutterAPI" )

			pCArcClient.get()->SetOpenShutter( ( jShouldOpen == 1 ? true : false ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    SetSyntheticImageModeAPI
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_SetSyntheticImageModeAPI( JNIEnv *env, jclass clazz, jboolean jUseSynMode )
{
	try
	{
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->SetSyntheticImageMode( ( jUseSynMode == 1 ? true : false ) );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "SetSyntheticImageModeAPI" )

			pCArcClient.get()->SetSyntheticImageMode( ( jUseSynMode == 1 ? true : false ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    IsSyntheticImageModeAPI
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_owl_cameraAPI_CameraAPI_IsSyntheticImageModeAPI( JNIEnv *env, jclass clazz )
{
	bool bIsSet = false;
	
	try
	{
		if ( !bRemoteAPI )
		{
			bIsSet = pCArcDev.get()->IsSyntheticImageMode();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "IsSyntheticImageModeAPI" )

			bIsSet = pCArcClient.get()->IsSyntheticImageMode();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return bIsSet;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    ResetDeviceAPI
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_ResetDeviceAPI( JNIEnv *env, jclass clazz )
{
	try
	{
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->Reset();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "ResetDeviceAPI" )

			pCArcClient.get()->Reset();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    ResetControllerAPI
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_ResetControllerAPI( JNIEnv *env, jclass clazz )
{
	try
	{
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->ResetController();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "ResetControllerAPI" )

			pCArcClient.get()->ResetController();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    StopExposureAPI
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_StopExposureAPI( JNIEnv *env, jclass clazz )
{
	try
	{
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->StopExposure();
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "StopExposureAPI" )

			pCArcClient.get()->StopExposure();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

///*
// * Class:     owl_cameraAPI_CameraAPI
// * Method:    Is2xFOReceiverAPI
// * Signature: ()Z
// */
//JNIEXPORT jboolean JNICALL
//Java_owl_cameraAPI_CameraAPI_Is2xFOReceiverAPI( JNIEnv *env, jclass clazz )
//{
//	bool bFO2x = false;
//
//	try
//	{
//		bFO2x = STATUS_FIBER_2X_RECEIVER( pCArcDev.get()->GetStatus() );
//	}
//	catch ( std::exception &e )
//	{
//		ThrowJNIException( env, e.what() );
//	}
//	catch ( ... )
//	{
//		ThrowUnknownJNIException( env );
//	}
//
//	return bFO2x;
//}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    Set2xFOTransmitterAPI
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_Set2xFOTransmitterAPI( JNIEnv *env, jclass clazz, jboolean jOnOff )
{
	try
	{
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->Set2xFOTransmitter( ( jOnOff != 0 ) );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "Set2xFOTransmitterAPI" )

			pCArcClient.get()->Set2xFOTransmitter( ( jOnOff != 0 ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetCfgByteAPI
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_owl_cameraAPI_CameraAPI_GetCfgByteAPI
( JNIEnv *env, jclass clazz, jint jOffset )
{
	int dRetVal = 0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			dRetVal =
				( ( CArcPCIBase * )pCArcDev.get() )->GetCfgSpByte( int( jOffset ) );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetCfgSpByteAPI" )

			dRetVal = pCArcClient.get()->GetCfgSpByte( int( jOffset ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return dRetVal;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetCfgWordAPI
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_owl_cameraAPI_CameraAPI_GetCfgWordAPI
( JNIEnv *env, jclass clazz, jint jOffset )
{
	int dRetVal = 0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			dRetVal =
				( ( CArcPCIBase * )pCArcDev.get() )->GetCfgSpWord( int( jOffset ) );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetCfgSpWordAPI" )

			dRetVal = pCArcClient.get()->GetCfgSpWord( int( jOffset ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return dRetVal;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetCfgDWordAPI
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_owl_cameraAPI_CameraAPI_GetCfgDWordAPI
( JNIEnv *env, jclass clazz, jint jOffset )
{
	int dRetVal = 0;
	
	try
	{
		if ( !bRemoteAPI )
		{
			dRetVal =
				( ( CArcPCIBase * )pCArcDev.get() )->GetCfgSpDWord( int( jOffset ) );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "GetCfgSpDWordAPI" )

			dRetVal = pCArcClient.get()->GetCfgSpDWord( int( jOffset ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
	
	return dRetVal;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    SetCfgByteAPI
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_SetCfgByteAPI
( JNIEnv *env, jclass clazz, jint jOffset, jint jValue )
{
	try
	{
		if ( !bRemoteAPI )
		{
			( ( CArcPCIBase * )pCArcDev.get() )->SetCfgSpByte( int( jOffset ),
															   int( jValue ) );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "SetCfgSpByteAPI" )

			pCArcClient.get()->SetCfgSpByte( int( jOffset ), int( jValue ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    SetCfgWordAPI
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_SetCfgWordAPI
( JNIEnv *env, jclass clazz, jint jOffset, jint jValue )
{
	try
	{
		if ( !bRemoteAPI )
		{
			( ( CArcPCIBase * )pCArcDev.get() )->SetCfgSpWord( int( jOffset ),
															   int( jValue ) );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "SetCfgSpWordAPI" )

			pCArcClient.get()->SetCfgSpWord( int( jOffset ), int( jValue ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    SetCfgDWordAPI
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_SetCfgDWordAPI
( JNIEnv *env, jclass clazz, jint jOffset, jint jValue )
{
	try
	{
		if ( !bRemoteAPI )
		{
			( ( CArcPCIBase * )pCArcDev.get() )->SetCfgSpDWord( int( jOffset ),
																int( jValue ) );
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "SetCfgSpDWordAPI" )

			pCArcClient.get()->SetCfgSpDWord( int( jOffset ), int( jValue ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetProperties
 * Signature: ()[[Ljava/lang/Object;
 */
JNIEXPORT jobjectArray JNICALL
Java_owl_cameraAPI_CameraAPI_GetProperties( JNIEnv *env, jclass clazz )
{
	jobjectArray	jPropObjArray		= NULL;
	jobjectArray	jValObjArray		= NULL;
	jclass			jStringClass		= NULL;
	jmethodID		jStringCtor			= NULL;

	try
	{
		const char*** pszPropList = pCArcDev.get()->GetProperties();

		if ( pszPropList != NULL )
		{
			int dPropCount = pCArcDev.get()->PropertyCount();

			jclass jStrArrClass = env->FindClass( "[Ljava/lang/String;");

			if ( jStrArrClass == NULL )
			{
				ThrowJNIException( env,
								   string( "( GetProperties ): " ) +
								   "Failed to find string array class!" );
			}

			jPropObjArray = env->NewObjectArray( dPropCount,
												 jStrArrClass,
												 NULL );

			if ( jPropObjArray == NULL )
			{
				ThrowJNIException( env,
								   string( "( GetProperties ): " ) +
								   "Failed to create property object array!" );
			}

			//  Find the java.lang.String class and its constructor
			// +--------------------------------------------------------------------------+
			jStringClass = env->FindClass( "java/lang/String" );

			if ( jStringClass == NULL )
			{
				ThrowJNIException( env,
								   string( "( GetProperties ): " ) +
								   "Failed to find java.lang.String class!" );
			}

			jStringCtor = env->GetMethodID( jStringClass, "<init>", "()V" );

			if ( jStringCtor == NULL )
			{
				ThrowJNIException( env,
								   string( "( GetProperties ): " ) +
								   "Failed to find java.lang.String class constructor!" );
			}

			for ( int i=0; i<dPropCount; i++ )
			{
				jValObjArray = env->NewObjectArray( 2,
													jStringClass,
													env->NewStringUTF( "" ) );

				if ( jValObjArray != NULL )
				{
					env->SetObjectArrayElement( jValObjArray,
												0,
												env->NewStringUTF( pszPropList[ i ][ 0 ] ) );

					env->SetObjectArrayElement( jValObjArray,
												1,
												env->NewStringUTF( pszPropList[ i ][ 1 ] ) );

					env->SetObjectArrayElement( jPropObjArray, i, jValObjArray );
					env->DeleteLocalRef( jValObjArray );
				}
			}	// end for loop

		} // end NULL property list
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jPropObjArray;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    DeinterlaceImage
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_DeinterlaceImage__III
( JNIEnv *env, jclass clazz, jint rows, jint cols, jint algorithm )
{
	try
	{
		if ( !bRemoteAPI )
		{
			if ( pCDeinterlace.get() == NULL )
			{
				pCDeinterlace.reset( new CDeinterlace() );
			}

			if ( pCDeinterlace.get() != NULL )
			{
				pCDeinterlace.get()->RunAlg( pCArcDev.get()->CommonBufferVA(),
											 rows,
											 cols,
											 algorithm );
			}
			else
			{
				ThrowJNIException( env, "Failed to create CDeinterlace object!" );
			}
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "DeinterlaceImage" )

			pCArcClient.get()->Deinterlace( rows, cols, algorithm );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    DeinterlaceImage
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_DeinterlaceImage__IIII
( JNIEnv *env, jclass clazz, jint rows, jint cols, jint pixelOffset, jint algorithm )
{
	try
	{
		if ( !bRemoteAPI )
		{
			if ( pCDeinterlace.get() == NULL )
			{
				pCDeinterlace.reset( new CDeinterlace() );
			}

			if ( pCDeinterlace.get() != NULL )
			{
				ushort* pU16Buf =
							static_cast<ushort *>( pCArcDev.get()->CommonBufferVA() );

				pCDeinterlace.get()->RunAlg( ( pU16Buf + pixelOffset ),
											   rows,
											   cols,
											   algorithm );
			}
			else
			{
				ThrowJNIException( env, "Failed to create CDeinterlace object!" );
			}
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "DeinterlaceImage" )

			pCArcClient.get()->Deinterlace( rows,
											cols,
											algorithm,
											-1,
											pixelOffset );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    DeinterlaceImage
 * Signature: (IIIII)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_DeinterlaceImage__IIIII
( JNIEnv *env, jclass clazz, jint rows, jint cols, jint pixelOffset, jint algorithm, jint arg )
{
	try
	{
		if ( !bRemoteAPI )
		{
			if ( pCDeinterlace.get() == NULL )
			{
				pCDeinterlace.reset( new CDeinterlace() );
			}

			if ( pCDeinterlace.get() != NULL )
			{
				ushort* pU16Buf =
						static_cast<ushort *>( pCArcDev.get()->CommonBufferVA() );

				pCDeinterlace.get()->RunAlg( ( pU16Buf + pixelOffset ),
											   rows,
											   cols,
											   algorithm,
											   arg );
			}
			else
			{
				ThrowJNIException( env, "Failed to create CDeinterlace object!" );
			}
		}
		else
		{
			ASSERT_CLIENT_CLASS_PTR( env, "DeinterlaceImage" )

			pCArcClient.get()->Deinterlace( rows,
											cols,
											algorithm,
											arg,
											pixelOffset );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetCustomDeinterlaceAlgorithms
 * Signature: (Ljava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_owl_cameraAPI_CameraAPI_GetCustomDeinterlaceAlgorithms
( JNIEnv *env, jclass clazz, jstring jLibPath )
{
	jobjectArray	jStrObjArray		= NULL;
	jclass			jStringClass		= NULL;
	jmethodID		jStringCtor			= NULL;

	const char *pszLibPath = ( jLibPath == NULL ? NULL : env->GetStringUTFChars( jLibPath, 0 ) );

	if ( pszLibPath != NULL )
	{
		try
		{
			if ( pCDeinterlace.get() == NULL )
			{
				pCDeinterlace.reset( new CDeinterlace() );

				if ( pCDeinterlace.get() == NULL )
				{
					ThrowJNIException(
								env,
								string( "( GetCustomDeinterlaceAlgorithms ): " ) +
								"Failed to create CDeinterlace object!" );
				}
			}

			if ( !pCDeinterlace.get()->FindCustomLibrary( pszLibPath ) )
			{
				return jStrObjArray;
			}

			int dCustomCount = pCDeinterlace.get()->GetCustomCount();

			if ( dCustomCount > 0 )
			{
				//  Find the java.util.TreeMap class and its constructor
				// +--------------------------------------------------------------------------+
				jStringClass = env->FindClass( "java/lang/String" );

				if ( jStringClass == NULL )
				{
					ThrowJNIException(
								env,
								string( "( GetCustomDeinterlaceAlgorithms ): " ) +
								"Failed to find java.lang.String class!" );
				}

				jStringCtor = env->GetMethodID( jStringClass, "<init>", "()V" );

				if ( jStringCtor == NULL )
				{
					ThrowJNIException(
								env,
								string( "( GetCustomDeinterlaceAlgorithms ): " ) +
								"Failed to find java.lang.String class constructor!" );
				}

				jStrObjArray = env->NewObjectArray( dCustomCount,
												    jStringClass,
												    env->NewStringUTF( "" ) );

				string sCustomName = "";
				int dAlgorithm = 0;

				//  Loop over the algorithms and add them to the object array
				// +--------------------------------------------------------------------------+
				for ( int i=0; i<dCustomCount; i++ )
				{
					pCDeinterlace.get()->GetCustomInfo( i, dAlgorithm, sCustomName );

					env->SetObjectArrayElement( jStrObjArray,
												i,
												env->NewStringUTF( sCustomName.c_str() ) );
				}
			}	// end if count > 0

			env->ReleaseStringUTFChars( jLibPath, pszLibPath );
		}
		catch ( std::exception &e )
		{
			env->ReleaseStringUTFChars( jLibPath, pszLibPath );
			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			env->ReleaseStringUTFChars( jLibPath, pszLibPath );
			ThrowUnknownJNIException( env );
		}
	}	// end if pszLibPath
	else
	{
		ThrowJNIException(
					env,
					string( "( GetCustomDeinterlaceAlgorithms ): " ) +
					"Failed to allocate memory for library path name!\n" );
	}

	return jStrObjArray;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    DeinterlaceFitsFile
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_DeinterlaceFitsFile__Ljava_lang_String_2I
( JNIEnv *env, jclass clazz, jstring jFilename, jint algorithm )
{
	try
	{
		Java_owl_cameraAPI_CameraAPI_DeinterlaceFitsFile__Ljava_lang_String_2II( env,
																				 clazz,
																				 jFilename,
																				 algorithm,
																				 -1 );
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    DeinterlaceFitsFile
 * Signature: (Ljava/lang/String;II)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_DeinterlaceFitsFile__Ljava_lang_String_2II
(JNIEnv *env, jclass clazz, jstring jFilename, jint algorithm, jint arg )
{
	//  Get the name of the file to write
	// +-----------------------------------------------------+
	const char* pszFilename = ( jFilename == NULL ? NULL : env->GetStringUTFChars( jFilename, 0 ) );

	if ( pszFilename != NULL )
	{
		try
		{
			// +-------------------------------------------------+
			// |  Local API Interface                            |
			// +-------------------------------------------------+
			if ( !bRemoteAPI )
			{
				//  Open the FITS file and read the image data
				// +-----------------------------------------------------+
				CFitsFile cFitsFile( pszFilename, CFitsFile::READWRITEMODE );
				void* pBuffer = cFitsFile.Read();
				
				if ( pBuffer == NULL )
				{
					ostringstream oss;

					oss << "Failed to read image data from FITS: "
						<< pszFilename
						<< ends;

					env->ReleaseStringUTFChars( jFilename, pszFilename );

					ThrowJNIException( env, oss.str() );
				}
			
				//  Read the image dimensions from the FITS file
				// +-----------------------------------------------------+
				long naxes[ 3 ] = { 0, 0, 0 };
				cFitsFile.GetParameters( naxes );
				
				if ( naxes[ CFitsFile::NAXES_ROW ] <= 0 && naxes[ CFitsFile::NAXES_COL ] <= 0 )
				{
					ostringstream oss;

					oss << "Failed to read image dimensions from FITS: "
						<< pszFilename
						<< ends;

					env->ReleaseStringUTFChars( jFilename, pszFilename );

					ThrowJNIException( env, oss.str() );
				}
				
				//  Deinterlace the image data and re-save in the FITS
				// +-----------------------------------------------------+
				if ( pCDeinterlace.get() == NULL )
				{
					pCDeinterlace.reset( new CDeinterlace() );

					if ( pCDeinterlace.get() == NULL )
					{
						ThrowJNIException( env, "Failed to create CDeinterlace object!" );
					}
				}

				pCDeinterlace.get()->RunAlg( pBuffer,
											 naxes[ CFitsFile::NAXES_ROW ],
											 naxes[ CFitsFile::NAXES_COL ],
											 algorithm,
											 arg );
				
				cFitsFile.Write( pBuffer );
			}

			// +-------------------------------------------------+
			// |  Remote API Interface                           |
			// +-------------------------------------------------+
			else
			{
				pCArcClient.get()->DeinterlaceFits( pszFilename,
													int( algorithm ),
													int( arg ) );
			}

			// Free the allocated memory for the file name
			env->ReleaseStringUTFChars( jFilename, pszFilename );
		}
		catch ( std::exception &e )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowUnknownJNIException( env );
		}
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    WriteTiffFile
 * Signature: (Ljava/lang/String;II)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_WriteTiffFile( JNIEnv *env, jclass clazz, jstring jFilename, jint rows, jint cols )
{
	// Get the name of the file to write
	const char* pszFilename = ( jFilename == NULL ? NULL : env->GetStringUTFChars( jFilename, 0 ) );

	if ( pszFilename != NULL )
	{
		try
		{
			// +-------------------------------------------------+
			// |  Local API Interface                            |
			// +-------------------------------------------------+
			if ( !bRemoteAPI )
			{
				CTiffFile cTiff( pszFilename );
				cTiff.Write( pCArcDev.get()->CommonBufferVA(), rows, cols );
			}

			// +-------------------------------------------------+
			// |  Remote API Interface                           |
			// +-------------------------------------------------+
			else
			{
				pCArcClient.get()->CTiffFile( pszFilename );
				pCArcClient.get()->WriteTiff( rows, cols );
				pCArcClient.get()->CloseTiff();
			}

			// Free the allocated memory for the file name
			env->ReleaseStringUTFChars( jFilename, pszFilename );
		}
		catch ( std::exception &e )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowUnknownJNIException( env );
		}
	}
	else
	{
		ThrowJNIException( env,
				"( WriteTiffFile ): Failed to allocate memory for filename!\n" );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    WriteFitsFile
 * Signature: (Ljava/lang/String;II)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_WriteFitsFile( JNIEnv *env, jclass clazz, jstring jFilename, jint rows, jint cols )
{
	// Get the name of the file to write
	const char* pszFilename = ( jFilename == NULL ? NULL : env->GetStringUTFChars( jFilename, 0 ) );

	if ( pszFilename != NULL )
	{
		try
		{
			// +-------------------------------------------------+
			// |  Local API Interface                            |
			// +-------------------------------------------------+
			if ( !bRemoteAPI )
			{
				CFitsFile cFits( pszFilename, rows, cols );
				cFits.Write( pCArcDev.get()->CommonBufferVA() );
			}

			// +-------------------------------------------------+
			// |  Remote API Interface                           |
			// +-------------------------------------------------+
			else
			{
				pCArcClient.get()->CFitsFile( pszFilename, rows, cols );
				pCArcClient.get()->WriteFits();
				pCArcClient.get()->CloseFits();
			}

			// Free the allocated memory for the file name
			env->ReleaseStringUTFChars( jFilename, pszFilename );
		}
		catch ( std::exception &e )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowUnknownJNIException( env );
		}
	}
	else
	{
		ThrowJNIException( env,
					"Failed to allocate memory for filename!\n" );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    WriteFitsKeyword
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_WriteFitsKeyword
( JNIEnv * env, jclass clazz, jstring jkey, jstring jkeyVal, jstring jComment, jstring jFilename )
{
	jclass stringClass = env->FindClass( "Ljava/lang/String;" );
	
	if ( stringClass != NULL )
	{
		jmethodID stringMatchesID = env->GetMethodID( stringClass,
													  "matches",
													  "(Ljava/lang/String;)Z" );
		
		if ( stringMatchesID != NULL )
		{
			jstring integerRegEx = env->NewStringUTF( "[0-9]+" );
			jstring doubleRegEx  = env->NewStringUTF( "[0-9]*[.]{1}[0-9]+" );
			jstring logicalRegEx = env->NewStringUTF( "[TF]{1}" );
			
			if ( integerRegEx == NULL || doubleRegEx == NULL || logicalRegEx == NULL )
			{
				ThrowJNIException( env, 
						"( WriteFitsKeyword ): Internal error. Regular expressions for parsing are NULL!" );
			}

			char* pszKey      = ( ( jkey      != NULL ) ? ( char * )env->GetStringUTFChars( jkey, 0 )      : NULL );
			char* pszKeyVal   = ( ( jkeyVal   != NULL ) ? ( char * )env->GetStringUTFChars( jkeyVal, 0 )   : NULL );
			char* pszComment  = ( ( jComment  != NULL ) ? ( char * )env->GetStringUTFChars( jComment, 0 )  : NULL );
			char* pszFilename = ( ( jFilename != NULL ) ? ( char * )env->GetStringUTFChars( jFilename, 0 ) : NULL );

			//
			// NOTE: The comment field can be NULL
			//
			if ( pszKey == NULL )
			{
				ThrowJNIException( env,
							"( WriteFitsKeyword ): Internal error. FITS keyword failed to be parsed!" );
			}

			else if ( pszKeyVal == NULL )
			{
				ThrowJNIException( env,
							"( WriteFitsKeyword ): Internal error. FITS keyword value failed to be parsed!" );
			}

			else if ( pszFilename == NULL )
			{
				ThrowJNIException( env,
							"( WriteFitsKeyword ): Internal error. FITS filename failed to be parsed!" );
			}
			
			try
			{
				// +-------------------------------------------------+
				// |  Local API Interface                            |
				// +-------------------------------------------------+
				if ( !bRemoteAPI )
				{
					CFitsFile cFits( pszFilename, CFitsFile::READWRITEMODE );
					
					if ( env->CallObjectMethod( jkeyVal, stringMatchesID, doubleRegEx ) )
					{
						double doubleKeyVal = atof( pszKeyVal );
						
						cFits.WriteKeyword( pszKey,
											&doubleKeyVal,
											CFitsFile::FITS_DOUBLE_KEY,
											pszComment );
					}
					
					else if ( env->CallObjectMethod( jkeyVal, stringMatchesID, integerRegEx ) )
					{
						int intKeyVal = atoi( pszKeyVal );
						
						cFits.WriteKeyword( pszKey,
											&intKeyVal,
											CFitsFile::FITS_INTEGER_KEY,
											pszComment );
					}
					
					else if ( env->CallObjectMethod( jkeyVal, stringMatchesID, logicalRegEx ) )
					{
						cFits.WriteKeyword( pszKey,
											pszKeyVal,
											CFitsFile::FITS_LOGICAL_KEY,
											pszComment );
					}
					
					else
					{
						cFits.WriteKeyword( pszKey,
											pszKeyVal,
											CFitsFile::FITS_STRING_KEY,
											pszComment );
					} 
				}

				// +-------------------------------------------------+
				// |  Remote API Interface                           |
				// +-------------------------------------------------+
				else
				{
					pCArcClient.get()->CFitsFile( pszFilename, CFitsFile::READWRITEMODE );

					if ( env->CallObjectMethod( jkeyVal, stringMatchesID, doubleRegEx ) )
					{
						double doubleKeyVal = atof( pszKeyVal );

						pCArcClient.get()->WriteFitsKeyword( pszKey,
														     &doubleKeyVal,
															 CFitsFile::FITS_DOUBLE_KEY,
															 pszComment );
					}

					else if ( env->CallObjectMethod( jkeyVal, stringMatchesID, integerRegEx ) )
					{
						int intKeyVal = atoi( pszKeyVal );

						pCArcClient.get()->WriteFitsKeyword( pszKey,
															 &intKeyVal,
															 CFitsFile::FITS_INTEGER_KEY,
															 pszComment );
					}

					else if ( env->CallObjectMethod( jkeyVal, stringMatchesID, logicalRegEx ) )
					{
						pCArcClient.get()->WriteFitsKeyword( pszKey,
															 pszKeyVal,
															 CFitsFile::FITS_LOGICAL_KEY,
															 pszComment );
					}

					else
					{
						pCArcClient.get()->WriteFitsKeyword( pszKey,
															 pszKeyVal,
															 CFitsFile::FITS_STRING_KEY,
															 pszComment );
					} 

					pCArcClient.get()->CloseFits();
				}

				//
				// Free the allocated memory for strings
				//
				env->ReleaseStringUTFChars( jkey, pszKey );
				env->ReleaseStringUTFChars( jkeyVal, pszKeyVal );
				env->ReleaseStringUTFChars( jFilename, pszFilename );

				if ( pszComment != NULL ) { env->ReleaseStringUTFChars( jComment, pszComment ); }
			}
			catch ( std::exception &e )
			{
				env->ReleaseStringUTFChars( jkey, pszKey );
				env->ReleaseStringUTFChars( jkeyVal, pszKeyVal );
				env->ReleaseStringUTFChars( jFilename, pszFilename );

				if ( pszComment != NULL ) { env->ReleaseStringUTFChars( jComment, pszComment ); }

				ThrowJNIException( env, e.what() );
			}
			catch ( ... )
			{
				env->ReleaseStringUTFChars( jkey, pszKey );
				env->ReleaseStringUTFChars( jkeyVal, pszKeyVal );
				env->ReleaseStringUTFChars( jFilename, pszFilename );

				if ( pszComment != NULL ) { env->ReleaseStringUTFChars( jComment, pszComment ); }

				ThrowUnknownJNIException( env );
			}
		}
		else
		{
			ThrowJNIException( env,
					"( WriteFitsKeyword ): Internal error. Failed to find \"String.matches()\" method!" );
		}
	}
	else
	{
		ThrowJNIException( env,
					"( WriteFitsKeyword ): Internal error. Failed to find \"String\" class!" );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    WriteFitsComment
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_WriteFitsComment( JNIEnv *env, jclass clazz, jstring jComment, jstring jFilename )
{
	// Get the comment and the name of the file to write
	const char* pszComment  = ( ( jComment  != NULL ) ? env->GetStringUTFChars( jComment,  0 ) : NULL );
	const char* pszFilename = ( ( jFilename != NULL ) ? env->GetStringUTFChars( jFilename, 0 ) : NULL );
	
	if ( pszComment != NULL && pszFilename != NULL )
	{
		try
		{
			// +-------------------------------------------------+
			// |  Local API Interface                            |
			// +-------------------------------------------------+
			if ( !bRemoteAPI )
			{
				CFitsFile cFits( pszFilename, CFitsFile::READWRITEMODE );
				
				cFits.WriteKeyword( NULL,
								    ( void * )pszComment,
								    CFitsFile::FITS_COMMENT_KEY,
								    NULL );
			}

			// +-------------------------------------------------+
			// |  Remote API Interface                           |
			// +-------------------------------------------------+
			else
			{
				pCArcClient.get()->CFitsFile( pszFilename, CFitsFile::READWRITEMODE );

				pCArcClient.get()->WriteFitsKeyword( NULL,
												     ( void * )pszComment,
												     CFitsFile::FITS_COMMENT_KEY,
												     NULL );
				pCArcClient.get()->CloseFits();
			}

			// Free the allocated memory for the strings
			env->ReleaseStringUTFChars( jComment,  pszComment );
			env->ReleaseStringUTFChars( jFilename, pszFilename );
		}
		catch ( std::exception &e )
		{
			env->ReleaseStringUTFChars( jComment,  pszComment );
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			env->ReleaseStringUTFChars( jComment,  pszComment );
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowUnknownJNIException( env );
		}
	}
	else
	{
		ThrowJNIException( env,
				"( WriteFitsComment ): Failed to convert the comment and/or filename parameter!" );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    WriteFitsHistory
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_WriteFitsHistory( JNIEnv *env, jclass clazz, jstring jHistory, jstring jFilename )
{
	// Get the history and the name of the file to write
	const char* pszHistory  = ( ( jHistory  != NULL ) ? env->GetStringUTFChars( jHistory,  0 ) : NULL );
	const char* pszFilename = ( ( jFilename != NULL ) ? env->GetStringUTFChars( jFilename, 0 ) : NULL );
	
	if ( pszHistory != NULL && pszFilename != NULL )
	{
		try
		{
			// +-------------------------------------------------+
			// |  Local API Interface                            |
			// +-------------------------------------------------+
			if ( !bRemoteAPI )
			{
				CFitsFile cFits( pszFilename, CFitsFile::READWRITEMODE );
				
				cFits.WriteKeyword( NULL,
								    ( void * )pszHistory,
								    CFitsFile::FITS_HISTORY_KEY,
								    NULL );
			}

			// +-------------------------------------------------+
			// |  Remote API Interface                           |
			// +-------------------------------------------------+
			else
			{
				pCArcClient.get()->CFitsFile( pszFilename, CFitsFile::READWRITEMODE );

				pCArcClient.get()->WriteFitsKeyword( NULL,
											   ( void * )pszHistory,
											   CFitsFile::FITS_HISTORY_KEY,
											   NULL );
				pCArcClient.get()->CloseFits();
			}

			// Free the allocated memory for the strings
			env->ReleaseStringUTFChars( jHistory,  pszHistory );
			env->ReleaseStringUTFChars( jFilename, pszFilename );
		}
		catch ( std::exception &e )
		{
			env->ReleaseStringUTFChars( jHistory,  pszHistory );
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			env->ReleaseStringUTFChars( jHistory,  pszHistory );
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowUnknownJNIException( env );
		}
	}
	else
	{
		ThrowJNIException( env,
				"( WriteFitsHistory ): Failed to convert the comment and/or filename parameter!" );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    WriteFitsDate
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_WriteFitsDate( JNIEnv *env, jclass clazz, jstring jFilename )
{
	// Get the name of the file to write
	const char* pszFilename = ( ( jFilename != NULL ) ? env->GetStringUTFChars( jFilename, 0 ) : NULL );
	
	if ( pszFilename != NULL )
	{
		try
		{
			// +-------------------------------------------------+
			// |  Local API Interface                            |
			// +-------------------------------------------------+
			if ( !bRemoteAPI )
			{
				CFitsFile cFits( pszFilename, CFitsFile::READWRITEMODE );
				
				cFits.WriteKeyword( NULL,
									NULL,
									CFitsFile::FITS_DATE_KEY,
									NULL );
			}

			// +-------------------------------------------------+
			// |  Remote API Interface                           |
			// +-------------------------------------------------+
			else
			{
				pCArcClient.get()->CFitsFile( pszFilename, CFitsFile::READWRITEMODE );

				pCArcClient.get()->WriteFitsKeyword( NULL,
											   NULL,
											   CFitsFile::FITS_DATE_KEY,
											   NULL );

				pCArcClient.get()->CloseFits();
			}

			// Free the allocated memory for the filename
			env->ReleaseStringUTFChars( jFilename, pszFilename );
		}
		catch ( std::exception &e )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowUnknownJNIException( env );
		}
	}
	else
	{
		ThrowJNIException( env,
				"( WriteFitsDate ): Failed to convert the comment and/or filename parameter!" );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    CreateFitsFile
 * Signature: (Ljava/lang/String;III)J
 */
JNIEXPORT jlong JNICALL Java_owl_cameraAPI_CameraAPI_CreateFitsFile
( JNIEnv *env, jclass clazz, jstring jFilename, jint rows, jint cols, jint bitsPerPixel )
{
	INT_PTR dRetVal = 0;

	// Get the FITS filename
	const char* pszFilename = ( jFilename != NULL ? env->GetStringUTFChars( jFilename, 0 ) : NULL );

	if ( pszFilename == NULL )
	{
		ThrowJNIException( env,
				"( CreateFitsFile ): Failed to convert filename parameter!" );
	}
	
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			CFitsFile* pCFits =
						new CFitsFile( pszFilename,
									   rows,
									   cols,
									   bitsPerPixel );

			dRetVal = INT_PTR( pCFits );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			pCArcClient.get()->CFitsFile( pszFilename,
									rows,
									cols,
									bitsPerPixel );

			dRetVal = 1;
		}

		env->ReleaseStringUTFChars( jFilename, pszFilename );
	}
	catch ( std::exception &e )
	{
		env->ReleaseStringUTFChars( jFilename, pszFilename );

		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		env->ReleaseStringUTFChars( jFilename, pszFilename );

		ThrowUnknownJNIException( env );
	}

	return jlong( dRetVal );
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    WriteToFitsFile
 * Signature: (JIII)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_WriteToFitsFile
( JNIEnv *env, jclass clazz, jlong fitsFileHandle, jint bytes, jint bytesToSkip, jint fPixel )
{
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			ushort *pUShortBuf =
					( ushort * )pCArcDev.get()->CommonBufferVA() +
					 bytesToSkip / sizeof( ushort );
			
			if ( pUShortBuf == NULL )
			{
				ThrowJNIException( env,
						"( WriteToFitsFile ): Image data buffer is NULL!" );
			}
			
			( ( CFitsFile * )fitsFileHandle )->Write( pUShortBuf,
													  bytes,
													  fPixel );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			pCArcClient.get()->WriteFits( bytesToSkip,
									bytes,
									fPixel );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    Create3DFitsFile
 * Signature: (Ljava/lang/String;III)J
 */
JNIEXPORT jlong JNICALL Java_owl_cameraAPI_CameraAPI_Create3DFitsFile
( JNIEnv *env, jclass clazz, jstring jFilename, jint rows, jint cols, jint bitsPerPixel )
{
	INT_PTR dRetVal = 0;
	
	// Get the FITS filename
	const char* pszFilename = ( jFilename != NULL ? env->GetStringUTFChars( jFilename, 0 ) : NULL );

	if ( pszFilename != NULL )
	{
		try
		{
			// +-------------------------------------------------+
			// |  Local API Interface                            |
			// +-------------------------------------------------+
			if ( !bRemoteAPI )
			{
				CFitsFile* pCFits =
								new CFitsFile( pszFilename,
											   rows,
											   cols,
											   bitsPerPixel,
											   true );

				dRetVal = INT_PTR( pCFits );
			}

			// +-------------------------------------------------+
			// |  Remote API Interface                           |
			// +-------------------------------------------------+
			else
			{
				pCArcClient.get()->CFitsFile( pszFilename,
										rows,
										cols,
										bitsPerPixel,
										true );

				dRetVal = 1;
			}

			( env )->ReleaseStringUTFChars( jFilename, pszFilename );
		}
		catch ( std::exception &e )
		{
			( env )->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			( env )->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowUnknownJNIException( env );
		}
	}
	else
	{
		ThrowJNIException( env,
				"( Create3DFitsFile ): Failed to convert filename parameter!" );
	}

	return jlong( dRetVal );
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    WriteTo3DFitsFile
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_WriteTo3DFitsFile
( JNIEnv *env, jclass clazz, jlong fitsFileHandle, jint byteOffset )
{
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			CFitsFile* pCFits = ( CFitsFile * )fitsFileHandle;
			
			if ( pCFits == NULL )
			{
				ThrowJNIException( env,
						"( WriteTo3DFitsFile ): Invalid FITS 3D file handle!" );
			}

			ubyte *pCharBuf =
					( ( ubyte * )pCArcDev.get()->CommonBufferVA() )
					+ byteOffset;

			pCFits->Write3D( pCharBuf );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			pCArcClient.get()->WriteFits3D( byteOffset );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    Deinterlace3DFitsFile
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_Deinterlace3DFitsFile__Ljava_lang_String_2I
( JNIEnv *env, jclass clazz, jstring jFilename, jint algorithm )
{
	try
	{
		Java_owl_cameraAPI_CameraAPI_Deinterlace3DFitsFile__Ljava_lang_String_2II( env,
																				   clazz,
																				   jFilename,
																				   algorithm,
																				   -1 );
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    Deinterlace3DFitsFile
 * Signature: (Ljava/lang/String;II)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_Deinterlace3DFitsFile__Ljava_lang_String_2II
( JNIEnv *env, jclass clazz, jstring jFilename, jint algorithm, jint arg )
{
	// Get the name of the file to write
	const char* pszFilename = ( jFilename != NULL ? env->GetStringUTFChars( jFilename, 0 ) : NULL );

	if ( pszFilename != NULL )
	{
		try
		{
			// +-------------------------------------------------+
			// |  Local API Interface                            |
			// +-------------------------------------------------+
			if ( !bRemoteAPI )
			{
				long naxes[ 3 ] = { 0, 0, 0 };
				
				CFitsFile cFits( pszFilename, CFitsFile::READWRITEMODE );
				cFits.GetParameters( naxes );
				
				if ( naxes[ CFitsFile::NAXES_ROW ] <= 0 && naxes[ CFitsFile::NAXES_COL ] <= 0 )
				{
					ostringstream oss;

					oss << "Failed to read image dimensions from FITS: "
						<< pszFilename
						<< ends;
					
					throw std::runtime_error( oss.str() );
				}
				
				for ( long i=0; i<naxes[ CFitsFile::NAXES_NOF ]; i++ )
				{
					unsigned short *usBuffer = ( unsigned short * )cFits.Read();
					
					if ( usBuffer != NULL )
					{
						if ( pCDeinterlace.get() == NULL )
						{
							pCDeinterlace.reset( new CDeinterlace() );

							if ( pCDeinterlace.get() == NULL )
							{
								throw std::runtime_error( "Failed to create CDeinterlace object!" );
							}
						}
						
						pCDeinterlace.get()->RunAlg( usBuffer,
													 naxes[ CFitsFile::NAXES_ROW ],
													 naxes[ CFitsFile::NAXES_COL ],
													 algorithm,
													 arg );
					}
					
					cFits.ReWrite3D( usBuffer, i );
				}
			}

			// +-------------------------------------------------+
			// |  Remote API Interface                           |
			// +-------------------------------------------------+
			else
			{
				pCArcClient.get()->DeinterlaceFits3D( pszFilename,
												algorithm,
												arg );
			}

			// Free the allocated memory for the file name
			env->ReleaseStringUTFChars( jFilename, pszFilename );
		}
		catch ( std::exception &e )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			env->ReleaseStringUTFChars( jFilename, pszFilename );

			ThrowUnknownJNIException( env );
		}
	}
	else
	{
		ThrowJNIException( env, "Failed to convert filename parameter!" );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    CloseFitsFile
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_CloseFitsFile
( JNIEnv *env, jclass clazz, jlong fitsFileHandle )
{
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			if ( fitsFileHandle > 0 )
			{
				delete ( ( CFitsFile * )( INT_PTR( fitsFileHandle ) ) );
			}
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			pCArcClient.get()->CloseFits();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    SubtractImageHalves
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_SubtractImageHalves
( JNIEnv *env, jclass clazz, jint rows, jint cols )
{
	CImage cImg;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			cImg.SubtractImageHalves(
							pCArcDev.get()->CommonBufferVA(),
							rows,
							cols );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			pCArcClient.get()->SubtractImageHalves( rows, cols );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    VerifySyntheticImage
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_VerifySyntheticImage
( JNIEnv *env, jclass clazz, jint rows, jint cols )
{
	CImage cImg;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			cImg.VerifyImageAsSynthetic(
							pCArcDev.get()->CommonBufferVA(),
							rows,
							cols );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			pCArcClient.get()->VerifyImageAsSynthetic( rows, cols );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    LogAPICmds
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_LogAPICmds( JNIEnv *env, jclass clazz, jboolean jLogCmds )
{
	// +-------------------------------------------------+
	// |  Local API Interface                            |
	// +-------------------------------------------------+
	if ( !bRemoteAPI )
	{
		pCArcDev.get()->SetLogCmds(
						( jLogCmds == 1 ? true : false ) );
	}

	// +-------------------------------------------------+
	// |  Remote API Interface                           |
	// +-------------------------------------------------+
	else
	{
		pCArcClient.get()->SetLogCmds(
						( jLogCmds == 1 ? true : false ) );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetLoggedAPICmds
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL
Java_owl_cameraAPI_CameraAPI_GetLoggedAPICmds( JNIEnv *env, jclass clazz )
{
	jobjectArray jObjArray = NULL;
	char*        buffer    = NULL;
	int          dLogCount = 0;

	// +-------------------------------------------------+
	// |  Local API Interface                            |
	// +-------------------------------------------------+
	if ( !bRemoteAPI )
	{
		dLogCount = pCArcDev.get()->GetLoggedCmdCount();
	}

	// +-------------------------------------------------+
	// |  Remote API Interface                           |
	// +-------------------------------------------------+
	else
	{
		dLogCount = pCArcClient.get()->GetLoggedCmdCount();
	}

	if ( dLogCount > 0 )
	{
		jObjArray = env->NewObjectArray( pCArcDev.get()->GetLoggedCmdCount(),
										 env->FindClass( "java/lang/String" ),
										 env->NewStringUTF( "" ) );

		if ( jObjArray != NULL )
		{
			// +-------------------------------------------------+
			// |  Local API Interface                            |
			// +-------------------------------------------------+
			if ( !bRemoteAPI )
			{
				buffer = ( char * )pCArcDev.get()->GetNextLoggedCmd();
			}

			// +-------------------------------------------------+
			// |  Remote API Interface                           |
			// +-------------------------------------------------+
			else
			{
				buffer = ( char * )pCArcClient.get()->GetNextLoggedCmd();
			}

			int elem = 0;

			while ( buffer != NULL && !string( buffer ).empty() )
			{
				env->SetObjectArrayElement( jObjArray,
										    elem,
										    env->NewStringUTF( buffer ) );

				if ( env->ExceptionOccurred() != NULL )
				{
					ThrowJNIException( env,
							"( GetLoggedAPICmds ): Failed to add logged API command to list!" );
				}
				
				// +-------------------------------------------------+
				// |  Local API Interface                            |
				// +-------------------------------------------------+
				if ( !bRemoteAPI )
				{
					buffer = ( char * )pCArcDev.get()->GetNextLoggedCmd();
				}

				// +-------------------------------------------------+
				// |  Remote API Interface                           |
				// +-------------------------------------------------+
				else
				{
					buffer = ( char * )pCArcClient.get()->GetNextLoggedCmd();
				}

				elem++;
			}
		}
		else
		{
			ThrowJNIException( env,
					"( GetLoggedAPICmds ): Failed create list for logged API commands!" );
		}
	}

	return jObjArray;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    SetupController
 * Signature: (ZZZIILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_owl_cameraAPI_CameraAPI_SetupController
( JNIEnv *env, jclass clazz, jboolean jbReset, jboolean jbTdl, jboolean jbPower, jint jdRows,
 jint jdCols, jstring jsTimFile, jstring jsUtilFile, jstring jsPciFile )
{
	// Get the lod filenames
	const char* pszTimFile  = ( jsTimFile  == NULL ? NULL : env->GetStringUTFChars( jsTimFile,  0 ) );
	const char* pszUtilFile = ( jsUtilFile == NULL ? NULL : env->GetStringUTFChars( jsUtilFile, 0 ) );
	const char* pszPciFile  = ( jsPciFile  == NULL ? NULL : env->GetStringUTFChars( jsPciFile,  0 ) );
	
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			pCArcDev.get()->SetupController( ( jbReset != 0 ),
											 ( jbTdl != 0 ),
											 ( jbPower != 0 ),
											 static_cast<int>( jdRows ),
											 static_cast<int>( jdCols ),
											 pszTimFile,
											 pszUtilFile,
											 pszPciFile );
		}

		// +-------------------------------------------------+
		// |  Remote API Interface                           |
		// +-------------------------------------------------+
		else
		{
			pCArcClient.get()->SetupController( ( jbReset != 0 ),
												( jbTdl != 0 ),
												( jbPower != 0 ),
												static_cast<int>( jdRows ),
												static_cast<int>( jdCols ),
												pszTimFile,
												pszUtilFile,
												pszPciFile );
		}

		// Free the allocated memory for the files
		if ( pszTimFile  != NULL ) env->ReleaseStringUTFChars( jsTimFile,  pszTimFile );
		if ( pszUtilFile != NULL ) env->ReleaseStringUTFChars( jsUtilFile, pszUtilFile );
		if ( pszPciFile  != NULL ) env->ReleaseStringUTFChars( jsPciFile,  pszPciFile );
	}
	catch ( std::exception &e )
	{
		// Free the allocated memory for the files
		if ( pszTimFile  != NULL ) env->ReleaseStringUTFChars( jsTimFile,  pszTimFile );
		if ( pszUtilFile != NULL ) env->ReleaseStringUTFChars( jsUtilFile, pszUtilFile );
		if ( pszPciFile  != NULL ) env->ReleaseStringUTFChars( jsPciFile,  pszPciFile );
		
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		// Free the allocated memory for the files
		if ( pszTimFile  != NULL ) env->ReleaseStringUTFChars( jsTimFile,  pszTimFile );
		if ( pszUtilFile != NULL ) env->ReleaseStringUTFChars( jsUtilFile, pszUtilFile );
		if ( pszPciFile  != NULL ) env->ReleaseStringUTFChars( jsPciFile,  pszPciFile );
		
		ThrowUnknownJNIException( env );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetCfgSpCount
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetCfgSpCount( JNIEnv *env, jclass clazz )
{
	int dCount = 0;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			( ( CArcPCIBase * )pCArcDev.get() )->GetCfgSp();
	
			dCount =
				( ( CArcPCIBase * )pCArcDev.get() )->GetCfgSpCount();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jint( dCount );
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetCfgSpAddr
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetCfgSpAddr( JNIEnv *env, jclass clazz, jint jIndex )
{
	int dAddr = 0;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			dAddr =
				( ( CArcPCIBase * )pCArcDev.get() )->GetCfgSpAddr( int( jIndex ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jint( dAddr );
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetCfgSpValue
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetCfgSpValue( JNIEnv *env, jclass clazz, jint jIndex )
{
	int dValue = 0;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			dValue =
				( ( CArcPCIBase * )pCArcDev.get() )->GetCfgSpValue( int( jIndex ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jint( dValue );
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetCfgSpName
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_owl_cameraAPI_CameraAPI_GetCfgSpName( JNIEnv *env, jclass clazz, jint jIndex )
{
	jstring jsName = NULL;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			jsName = env->NewStringUTF(
					( ( CArcPCIBase * )pCArcDev.get() )->GetCfgSpName(
							int( jIndex ) ).c_str() );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jsName;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetCfgSpBitList
 * Signature: (I)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL
Java_owl_cameraAPI_CameraAPI_GetCfgSpBitList( JNIEnv *env, jclass clazz, jint jIndex )
{
	jobjectArray jBitList   = NULL;
	string*      pBitList   = NULL;
	int          dListCount = 0;
	
	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			//
			// User must free returned array
			//
			pBitList =
				( ( CArcPCIBase * )pCArcDev.get() )->GetCfgSpBitList( int( jIndex ),
																	  dListCount );
			
			if ( pBitList != NULL && dListCount > 0 )
			{
				jBitList =
						( jobjectArray )env->NewObjectArray( dListCount,
															 env->FindClass( "java/lang/String" ),
															 env->NewStringUTF( "" ) );
				
				if ( jBitList != NULL )
				{
					//
					// Add the bit list items
					//
					for ( int i=0; i<dListCount; i++ )
					{
						env->SetObjectArrayElement( jBitList,
													i,
													env->NewStringUTF( pBitList[ i ].c_str() ) );
						
						if ( env->ExceptionOccurred() != NULL )
						{
							throw std::runtime_error(
										"( GetCfgSpBitList ): Failed to add bit to list!" );
						}
					}
				}
				else
				{
					throw std::runtime_error(
								"( GetCfgSpBitList ): Failed create array for bit list!" );
				}
			}
		}
	}
	catch ( std::exception &e )
	{
		if ( pBitList != NULL )
		{
			delete [] pBitList;
		}
		
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		if ( pBitList != NULL )
		{
			delete [] pBitList;
		}
		
		ThrowUnknownJNIException( env );
	}
	
	return jBitList;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetBarCount
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetBarCount( JNIEnv *env, jclass clazz )
{
	int dCount = 0;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			( ( CArcPCIBase * )pCArcDev.get() )->GetBarSp();
		
			dCount =
				( ( CArcPCIBase * )pCArcDev.get() )->GetBarCount();
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jint( dCount );
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetBarName
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_owl_cameraAPI_CameraAPI_GetBarName( JNIEnv *env, jclass clazz, jint jIndex )
{
	jstring jsName = NULL;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			jsName = env->NewStringUTF(
					( ( CArcPCIBase * )pCArcDev.get() )->GetBarName(
							int( jIndex ) ).c_str() );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jsName;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetBarRegCount
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetBarRegCount( JNIEnv *env, jclass clazz, jint jIndex )
{
	int dCount = 0;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			dCount =
				( ( CArcPCIBase * )pCArcDev.get() )->GetBarRegCount( int( jIndex ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jint( dCount );
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetBarRegAddr
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetBarRegAddr( JNIEnv *env, jclass clazz, jint jIndex, jint jRegIndex )
{
	int dAddr = 0;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			dAddr =
				( ( CArcPCIBase * )pCArcDev.get() )->GetBarRegAddr( int( jIndex ),
																	int( jRegIndex ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jint( dAddr );
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetBarRegValue
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetBarRegValue( JNIEnv *env, jclass clazz, jint jIndex, jint jRegIndex )
{
	int dValue = 0;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			dValue =
				( ( CArcPCIBase * )pCArcDev.get() )->GetBarRegValue( int( jIndex ),
																	 int( jRegIndex ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return dValue;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetBarRegName
 * Signature: (II)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_owl_cameraAPI_CameraAPI_GetBarRegName( JNIEnv *env, jclass clazz, jint jIndex, jint jRegIndex )
{
	jstring jsName = NULL;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			jsName = env->NewStringUTF(
						( ( CArcPCIBase * )pCArcDev.get() )->GetBarRegName( int( jIndex ),
																			int( jRegIndex ) ).c_str() );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jsName;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetBarRegBitListCount
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_GetBarRegBitListCount( JNIEnv *env, jclass clazz, jint jIndex, jint jRegIndex )
{
	int dCount = 0;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			dCount =
				( ( CArcPCIBase * )pCArcDev.get() )->GetBarRegBitListCount( int( jIndex ),
																			int( jRegIndex ) );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jint( dCount );
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    GetBarRegBitListDef
 * Signature: (III)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_owl_cameraAPI_CameraAPI_GetBarRegBitListDef( JNIEnv *env, jclass clazz, jint jIndex, jint jRegIndex, jint jBitListIndex )
{
	jstring jsBitDef = NULL;

	try
	{
		// +-------------------------------------------------+
		// |  Local API Interface                            |
		// +-------------------------------------------------+
		if ( !bRemoteAPI )
		{
			jsBitDef = env->NewStringUTF(
					  ( ( CArcPCIBase * )pCArcDev.get() )->GetBarRegBitListDef( int( jIndex ),
																				int( jRegIndex ),
																				int( jBitListIndex ) ).c_str() );
		}
	}
	catch ( std::exception &e )
	{
		ThrowJNIException( env, e.what() );
	}
	catch ( ... )
	{
		ThrowUnknownJNIException( env );
	}

	return jsBitDef;
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    WriteBar
 * Signature: (III)V
 */
JNIEXPORT void JNICALL
Java_owl_cameraAPI_CameraAPI_WriteBar( JNIEnv *env, jclass clazz, jint jBar, jint jReg, jint jValue )
{
	string sID( pCArcDev.get()->ToString() );

	if ( sID.find( "PCIe" ) != string::npos )
	{
		try
		{
			// +-------------------------------------------------+
			// |  Local API Interface                            |
			// +-------------------------------------------------+
			if ( !bRemoteAPI )
			{
				( ( CArcPCIe * )pCArcDev.get() )->WriteBar( int( jBar ),
															int( jReg ),
															( 0xAC000000 | int( jValue ) ) );
			}
		}
		catch ( std::exception &e )
		{
			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			ThrowUnknownJNIException( env );
		}
	}
	else
	{
		ThrowJNIException( env, "WriteBar only available for ARC-66 ( PCIe )!" );
	}
}

/*
 * Class:     owl_cameraAPI_CameraAPI
 * Method:    ReadBar
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL
Java_owl_cameraAPI_CameraAPI_ReadBar( JNIEnv *env, jclass clazz, jint jBar, jint jReg )
{
	int dValue = 0;

	string sID( pCArcDev.get()->ToString() );

	if ( sID.find( "PCIe" ) != string::npos )
	{
		try
		{
			// +-------------------------------------------------+
			// |  Local API Interface                            |
			// +-------------------------------------------------+
			if ( !bRemoteAPI )
			{
				dValue =
					( ( CArcPCIe * )pCArcDev.get() )->ReadBar( int( jBar ),
															   int( jReg ) );
			}
		}
		catch ( std::exception &e )
		{
			ThrowJNIException( env, e.what() );
		}
		catch ( ... )
		{
			ThrowUnknownJNIException( env );
		}
	}
	else
	{
		ThrowJNIException( env, "ReadBar only available for ARC-66 ( PCIe )!" );
	}

	return dValue;
}


// +-----------------------------------------------------------------------------------------+
// | PRIVATE INTERNAL FUNCTIONS ONLY                                                         |
// +-----------------------------------------------------------------------------------------+
void ThrowJNIException( JNIEnv* env, string msg )
{
	jclass newExcCls = env->FindClass( "java/lang/Exception" );
	
	if ( newExcCls != NULL )
	{
		env->ThrowNew( newExcCls, msg.c_str() );
	}
}

void ThrowUnknownJNIException( JNIEnv* env )
{
	jclass newExcCls = env->FindClass( "java/lang/Exception" );
	
	if ( newExcCls != NULL )
	{
		env->ThrowNew( newExcCls, "An unknown error caused an exception!" );
	}
}
