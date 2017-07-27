import numpy
import os
import random
import time
import logging
import shutil
import pylibapogee.pylibapogee as apg
import pylibapogee.pylibapogee_setup as SetupDevice

testName = "startStop Image count"
testLog = logging.getLogger('apogeeTest')

def run( baseDir ):
	
	try:
		testLog.info( "%s STARTED" % (testName) )
		
		#make dir name, deleting or if
		outDir = os.path.join( baseDir, "ssImgCountBulk" )
		if( os.path.exists(outDir) ):
			shutil.rmtree(outDir)

		os.mkdir( outDir )
		
		#look for usb cameras first
		devices = SetupDevice.GetUsbDevices()
		
		# no usb cameras, then look for ethernet cameras
		if( len(devices) == 0 ):
			devices = SetupDevice.GetEthernetDevices()
			
		# exception....no cameras anywhere....
		if( len(devices) == 0 ):
			raise RuntimeError( "No devices found on usb or ethernet" )
			
		cam = SetupDevice.CreateAndConnectCam( devices[0] )
		
		cam.SetCooler( True )
		
		imgCount = 5
		cam.SetImageCount( imgCount )
		cam.SetBulkDownload( True )
		
		exposeTime = 0.1
		runCount = 0;
		random.seed()
			

		getNum = 0
		numStops = 0
		i = 0
			
		for num in range(0,50):
			runCount += 1
			runStr = "Run %04d:" % (runCount) 
			
			testLog.info( "%s Starting Exposure" % (runStr) )
			cam.StartExposure( exposeTime, True )
			
			kOut = random.randint( 0, imgCount )
			
			status = None
			while status != apg.Status_ImageReady:
				status = cam.GetImagingStatus()
				
				if( apg.Status_ConnectionError == status or
					apg.Status_DataError == status or
					apg.Status_PatternError == status ):
					
					msg = "Run %s: FAILED - error in camera status = %d" % (runStr, status)
					raise RuntimeError( msg )
					
				if( kOut == getNum and numStops < 5):
					kOut = random.randint( 0, imgCount )
					numStops += 1
					testLog.info( "%s Canceling Exposure" % (runStr) )
					cam.StopExposure( False )
					getNum = 0
					cam.SetImageCount( imgCount )
					testLog.info( "%s Starting Exposure" % (runStr) )
					cam.StartExposure( exposeTime, True )

			
			testLog.info( "%s Getting image %04d, img count %02d" % (runStr, i, getNum) )
			data = cam.GetImage()
			testLog.info( "%s Getting image completed" % (runStr) )
			getNum += 1
			i += 1
					
		cam.CloseConnection()
		testLog.info( "%s COMPLETED" % (testName) )
		return True
	except:
		 testLog.exception("%s FALIED with exception" % (testName) )
		 return False
