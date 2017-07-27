import numpy
import os
import random
import time
import logging
import shutil
import pylibapogee.pylibapogee as apg
import pylibapogee.pylibapogee_setup as SetupDevice

testName = "startStop Single exposures"
testLog = logging.getLogger('apogeeTest')

def run( baseDir ):
	try:
		testLog.info( "%s STARTED" % (testName) )
		
		
		#make dir name, deleting or if
		outDir = os.path.join( baseDir, "ssSingleImage" )
		if( os.path.exists(outDir) ):
			shutil.rmtree(outDir)

		os.mkdir( outDir )
		
		# take 3 images, cancel the 4th at different points in the 
		# during the exposure
		 
		exposeTime = 120.0
		delayCount = [1, 10, 50, 100]
		cancelMe = 0
		runCount = 0;
		
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
		
		cam.SetImageCount( 1 )
		
		for item in delayCount:
			dcount = 0
			for num in range(0,20):
				runStr = "Run %04d:" % (runCount) 
				
				testLog.info( "%s Starting Exposure" % (runStr) )
				cam.StartExposure( exposeTime, True )
				
				status = None
				while status != apg.Status_ImageReady:
					time.sleep( 1 )
					
					status = cam.GetImagingStatus()	
					if( apg.Status_ConnectionError == status or
						apg.Status_DataError == status or
						apg.Status_PatternError == status ):
						msg = "Run %s: FAILED - error in camera status = %d" % (runStr, status)
						raise RuntimeError( msg )
											
					if( dcount == item ):
						dcount = 0
						if( cancelMe >= 3 ):
							cancelMe = 0
							testLog.info( "%s Canceling Exposure" % (runStr) )
							
							#every other cancel fetch the image
							if( 0 == runCount % 2 ):
								cam.StopExposure( False )
							else:
								cam.StopExposure( True )
								testLog.info( "%s getting and saved canceled image" % (runStr) )
								data = cam.GetImage()
								imgName = "canceled-img%04d-r%d-c%d.bin" % (runCount,
																			cam.GetRoiNumRows(),
																			cam.GetRoiNumCols() )
								fullImgName = os.path.join( outDir, imgName )
								data.tofile( fullImgName )
				
							#get out of while loop
							break
					else:
						dcount += 1
						
				runCount += 1
				
				if(status == apg.Status_ImageReady):
					cancelMe += 1
					testLog.info( "%s Getting image" % (runStr) )
					data = cam.GetImage()
					
		cam.CloseConnection()
		testLog.info( "%s COMPLETED" % (testName) )
		return True
	except:
		 testLog.exception("%s FALIED with exception" % (testName) )
		 return False
