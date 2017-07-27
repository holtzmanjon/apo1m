import numpy
import os
import random
import time
import logging
import shutil
import pylibapogee.pylibapogee as apg
import pylibapogee.pylibapogee_setup as SetupDevice

testName = "tdi stress"
testLog = logging.getLogger('apogeeTest')

#------------------------
def BulkRun( cam, outDir, runNum ):
    
    if( False == cam.IsBulkDownloadOn() ):
        cam.SetBulkDownload( True )
        
    cam.StartExposure(0.001, True)
    
    status = None
    while status != apg.Status_ImageReady:
        #error handling
        if( apg.Status_ConnectionError == status or
            apg.Status_DataError == status or
            apg.Status_PatternError == status ):
            msg = "Run %04d: FAILED - error in camera status = %d" % (runNum, status)
            raise RuntimeError( msg )
            
        status = cam.GetImagingStatus()
    

    testLog.info( "Run %04d: get image bulk" % (runNum) )
    data = cam.GetImage()
    
    #save only every 10th image
    #in order to not fill up the harddrive
    if( 0 == runNum % 10 ):
        imgName = "tdi-img%04d-r%d-c%d.bin" \
                  % (runNum, cam.GetTdiRows(), cam.GetRoiNumCols())
        fullImgName = os.path.join( outDir, imgName )
        data.tofile( fullImgName )
        
    testLog.info( "Run %04d: got image, size = %d" % (runNum, data.size ) )
#------------------------  
def LineRun( cam, outDir, runNum ):
    if( True == cam.IsBulkDownloadOn() ):
        cam.SetBulkDownload( False )
        
    cam.StartExposure(0.001, True)
    
    testLog.info( "Run %04d: get image stream/lines" % (runNum) )
    
    line = 1    
    while(line <= cam.GetTdiRows() ):
        
        while( cam.GetTdiCounter() != line ):
            h = 1
            
        tdiRow = cam.GetImage()
        
        line += 1 
        
        
        #TODO save data throw on the floor for now
        
    testLog.info( "Run %04d: stream/lines finished succesfully" % (runNum) )

#------------------------
def run( baseDir ):
    try:
        testLog.info( "%s STARTED" % (testName) )

        #make dir name, deleting or if
        outDir = os.path.join( baseDir, "tdiStress" )
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
        

        if( cam.IsInterline () ):
             raise RuntimeError( "TDI mode not valid for interline cameras" )

        cam.SetCameraMode( apg.CameraMode_TDI )             
        cam.SetImageCount( 1 )
        cam.SetLedMode( apg.LedMode_EnableAll )
        cam.SetLedAState( apg.LedState_ImageActive )
        cam.SetLedBState( apg.LedState_Flushing )
        
        
        cols = cam.GetMaxImgCols()
        maxTdiRows = 4000
        
        runCount = 0
        random.seed()
        
        for num in range(0,50):
            numStr = "%04d" % ( num )
            
            tdiRate = random.uniform(0.006, 0.3 )
            cam.SetTdiRate( tdiRate )
            #bin every other image
            if( 0 == num % 2):
                tdiBinning = random.randint( 1, 750 )
                cam.SetTdiBinningRows( tdiBinning )
                #this is always fixed????
                cam.SetTdiRows( 500 )
                cam.SetRoiStartCol( 0 )
                cam.SetRoiNumCols( cols )
            else:
                cam.SetTdiBinningRows( 1 )
                tdiRows = random.randint( 1, maxTdiRows )
                cam.SetTdiRows( tdiRows )
                cam.SetTdiBinningRows( 1 )
                cam.SetRoiStartCol( 0 )
                cam.SetRoiNumCols( cols )
                
                     
            testLog.info( "Run %s: tdi image info: rate=%f," \
                          "tdi bin = %d, tdi rows = %d" % (numStr,
                                 cam.GetTdiRate(),
                                 cam.GetTdiBinningRows(),
                                 cam.GetTdiRows() ) )
                        
                
            #switch between bulk and single line down loads
            if( 0 == num % 3):
                testLog.info( "Run %s: Starting bulk download"  % (numStr) )
                BulkRun( cam, outDir, num )
            else:
                testLog.info( "Run %s: Starting streaming download"  % (numStr) )
                LineRun( cam, outDir, num )
                
                
            
        cam.SetCameraMode( apg.CameraMode_Normal )
        cam.CloseConnection()
        testLog.info( "%s COMPLETED" % (testName) )
        return True
    except:
        testLog.exception("%s FALIED with exception" % (testName) )
        return False