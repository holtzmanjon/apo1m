
import numpy
import os
import random
import time
import logging
import shutil
import pylibapogee.pylibapogee as apg
import pylibapogee.pylibapogee_setup as SetupDevice

testName = "asymmetrical binning"
testLog = logging.getLogger('apogeeTest')

def run( baseDir ):
	try:
		testLog.info( "%s STARTED" % (testName) )
		
		#make dir name, deleting or if
		outDir = os.path.join( baseDir, "asymBin" )
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
		
		maxBinR = cam.GetMaxBinRows()
		maxImgR = cam.GetMaxImgRows()
		
		maxBinC = cam.GetMaxBinCols()
		maxImgC = cam.GetMaxImgCols()
		
		numAcqImgs = 0
		
		for i in range(0,250):
			numStr = "%04d" % ( runCount )
			
			rBin = random.randint( 1, maxBinR )
			cam.SetRoiStartRow( 0 )
			rows = maxImgR / rBin
			cam.SetRoiNumRows( rows )
			cam.SetRoiBinRow( rBin )
			
			
			cBin = random.randint( 1, maxBinC )
			cam.SetRoiStartCol( 0 )
			cols = maxImgC / cBin
			cam.SetRoiNumCols( cols )
			cam.SetRoiBinCol( cBin )
	
			status = None
				   
			cam.StartExposure( 0.001, True )
			
			while status != apg.Status_ImageReady:
				status = cam.GetImagingStatus()
			
			msg = "Getting Image %s, br=%d, r=%d, bc=%d c=%d" %(numStr, rBin, rows, cBin, cols)
			testLog.info( msg )
			data = cam.GetImage()
			
			#save only every 10th image
			#in order to not fill up the harddrive
			if( 0 == numAcqImgs % 10 ):
				imgName = "ab-img%s-r%d-c%d.bin" % (numStr, rows, cols)
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
		