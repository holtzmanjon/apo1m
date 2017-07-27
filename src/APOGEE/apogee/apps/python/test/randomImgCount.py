
import numpy
import os
import random
import time
import logging
import shutil
import pylibapogee.pylibapogee as apg
import pylibapogee.pylibapogee_setup as SetupDevice

testName = "random image count"
testLog = logging.getLogger('apogeeTest')

def run( baseDir ):
	try:
		
		testLog.info( "%s STARTED" % (testName) )
		
		#make dir name, deleting or if
		outDir = os.path.join( baseDir, "randomImg" )
		if( os.path.exists(outDir) ):
			shutil.rmtree(outDir)

		os.mkdir( outDir )
		
		runCount = 0
		random.seed()
		
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
		if( cam.IsBulkDownloadOn() ):
			cam.SetBulkDownload( False ) 
		
		maxBinR = cam.GetMaxBinRows()
		maxImgR = cam.GetMaxImgRows()
		
		maxBinC = cam.GetMaxBinCols()
		maxImgC = cam.GetMaxImgCols()
			
		for i in range(0,25):
			numStr = "%05d" % ( runCount )
			binMe = random.randint( 1, cam.GetMaxBinCols() )
			cam.SetRoiBinRow( binMe )
			cam.SetRoiBinCol( binMe )
			
			cam.SetCcdAdcSpeed( apg.AdcSpeed_Fast )
			
			cam.SetRoiStartRow( 0 )
			rows = maxImgR / binMe
			cam.SetRoiNumRows( rows )
			
			cam.SetRoiStartCol( 0 )
			cols = maxImgC / binMe
			cam.SetRoiNumCols( cols )
	
			numImgs = random.randint( 1, 75 )
			numAcqImgs = 0
			cam.SetImageCount( numImgs )

			if( cam.GetImageCount() != numImgs ):
				msg = "Run %s: FAILED - invalid image count expected=%d, actual=%d" % (numStr, numImgs, cam.GetImageCount() )
				raise RuntimeError( msg )
			
			expTime = random.uniform(0.01, 2.0 ) 
			stopTime =  expTime + 10.0
			
			testLog.info( "Run %s: starting %f sec exposure of %d images" % (numStr,expTime,numImgs ) )
			t0 = time.time()
			cam.StartExposure( expTime, True )
			
			status = None
	
			while( numAcqImgs < numImgs ):
				numStr = "%05d" % ( runCount )
				while status != apg.Status_ImageReady:
					status = cam.GetImagingStatus()
					if( apg.Status_ConnectionError == status or
					   apg.Status_DataError == status or
					   apg.Status_PatternError == status ):
						
						msg = "Run %s: FAILED - error in camera status = %d" % (numStr, status)
						raise RuntimeError( msg )
									
					#break the while if we have been trying to
					#get the image
					t1 = time.time()
					diffTime = t1-t0
					if( diffTime > stopTime ):
						msg = "Run %s: FAILED - camera stauts = %d waited %f for image" % (numStr, status, diffTime)
						raise RuntimeError( msg )

						
				msg = "Run %s: getting image %d" % (numStr, numAcqImgs)
				testLog.info( msg )
				data = cam.GetImage()
				
				#save only every 20th image
				#in order to not fill up the harddrive
				if( 0 == numAcqImgs % 20 ):
					imgName = "ric-img%s-r%d-c%d.bin" % (numStr, rows, cols)
					fullImgName = os.path.join( outDir, imgName )
					data.tofile( fullImgName )
				
				numAcqImgs += 1
				runCount += 1
		
		cam.CloseConnection()
		
		testLog.info( "%s COMPLETED" % (testName) )
		return True
	except:
		 testLog.exception("%s FALIED with exception" % (testName) )
		 return False
