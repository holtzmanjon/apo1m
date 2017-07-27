
import numpy
import os
import random
import time
import logging
import shutil
import pylibapogee.pylibapogee as apg
import pylibapogee.pylibapogee_setup as SetupDevice

testName = "irregular roi bin test"
testLog = logging.getLogger('apogeeTest')

def run( baseDir ):
	try:
		testLog.info( "%s STARTED" % (testName) )
		
		#make dir name, deleting or if
		outDir = os.path.join( baseDir, "irRoiAndBin" )
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
		cam.SetImageCount( 1 )
		binMax = cam.GetMaxBinCols() 	
		binList = []
		
		runCount = 0
		random.seed()
		
		for v in range(0,8):
			binList.append( random.randint( 1, binMax ) )
	
		for binItem in binList:
			
			numAcqImgs = 0
			for i in range(0,25):
				numStr = "%05d" % ( runCount )
	
				maxStartRow = cam.GetMaxImgRows()-binItem
				startRow = random.randint( 0, maxStartRow )
				cam.SetRoiStartRow( startRow )
	
				rows = (cam.GetMaxImgRows()-startRow ) / binItem
				cam.SetRoiNumRows( rows )
				cam.SetRoiBinRow( binItem )
	
				maxStartCol = cam.GetMaxImgCols()-binItem
				startCol = random.randint( 0, maxStartCol )
				cam.SetRoiStartCol( startCol )
	
				cols = (cam.GetMaxImgCols()- startCol ) / binItem
				cam.SetRoiNumCols( cols )
				cam.SetRoiBinCol( binItem )
							
				expTime = random.uniform(0.001, 60.0 )   
				testLog.info( "Run %s: starting %f sec exposure" % (numStr,expTime ) )
				cam.StartExposure( expTime, True )
	
				status = None
				while status != apg.Status_ImageReady:
					
					if( expTime > 10.0):
						time.sleep( 2 )
						
					status = cam.GetImagingStatus()
	
					if( apg.Status_ConnectionError == status or
						apg.Status_DataError == status or
						apg.Status_PatternError == status ):
							msg = "Run %s: FAILED - error in camera status = %d" % (numStr, status)
							raise RuntimeError( msg )
					
	
				testLog.info( "Getting Image %s" %(numStr) )
				data = cam.GetImage()
				#save only every 10th image
				#in order to not fill up the harddrive
				if( 0 == numAcqImgs % 10 ):
					imgName = "irb-img%s-r%d-c%d.bin" % (numStr, rows, cols)
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
	