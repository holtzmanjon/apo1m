import numpy
import os
import random
import time
import logging
import shutil
import pylibapogee.pylibapogee as apg
import pylibapogee.pylibapogee_setup as SetupDevice

testName = "fullThenSubFrame"
testLog = logging.getLogger('apogeeTest')

def run( baseDir ):
    try:
        testLog.info( "%s STARTED" % (testName) )
        outDir = os.path.join( baseDir, "fullThenSub" )
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
        
        maxStartRow = cam.GetMaxImgRows()-1
        maxStartCol = cam.GetMaxImgCols()-1
        cam.SetCooler( True )
        cam.SetImageCount( 1 )
        cam.SetRoiBinRow( 1 )
        cam.SetRoiBinCol( 1 )
        
        runCount = 0
        random.seed()
        
        numAcqImgs = 0

        for i in range(0,250):
            numStr = "%04d" % ( runCount )
            
            startRow = 0
            rows = cam.GetMaxImgRows()
            startCol = 0
            cols = cam.GetMaxImgCols()
            
            if( 0 == ( i %2 ) ):
                startRow = random.randint( 0, maxStartRow )     
                rows = random.randint( 1, cam.GetMaxImgRows()-startRow )
               
                startCol = random.randint( 0, maxStartCol )
                cols = random.randint( 1, cam.GetMaxImgCols()-startCol )
                
                   
            cam.SetRoiStartRow( startRow )
            cam.SetRoiNumRows( rows )
            cam.SetRoiStartCol( startCol )
            cam.SetRoiNumCols( cols )
            
            status = None
            
            cam.StartExposure( 0.1, True )
            
            while status != apg.Status_ImageReady:
                status = cam.GetImagingStatus()
    
                if( apg.Status_ConnectionError == status or
                    apg.Status_DataError == status or
                    apg.Status_PatternError == status ):
                    msg = "Run %s: FAILED - error in camera status = %d" % (numStr, status)
                    raise RuntimeError( msg )
            
            msg = "Getting Image %s, r=%d, c=%d" %(numStr, rows, cols)
            testLog.info( msg )
            data = cam.GetImage()
            #save only every 10th image
            #in order to not fill up the harddrive
            if( 0 == numAcqImgs % 10 ):
                imgName = "fs-img%s-r%d-c%d.bin" % (numStr, rows, cols)
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
