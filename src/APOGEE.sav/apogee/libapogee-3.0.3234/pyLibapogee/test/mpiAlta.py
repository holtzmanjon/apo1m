import numpy as np
import os
import random
import time
import logging
import shutil
import pylibapogee.pylibapogee as apg
import pylibapogee.pylibapogee_setup as SetupDevice

testName = "MPI Alta 13"
testLog = logging.getLogger('apogeeTest')

def run( baseDir ):
    try:
        testLog.info( "%s STARTED" % (testName) )
        
        #make dir name, deleting or if
        outDir = os.path.join( baseDir, "MPI-Alta" )
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
        
        startTemp = cam.GetTempCcd()
        initalSetPt = startTemp - 60.0
        cam.SetCoolerSetPoint( initalSetPt  )
        
        cam.SetCooler( True )
        cam.SetFanMode( apg.FanMode_High )
        
        #waiting for the cooler to settle
        CoolStatusDict = {apg.CoolerStatus_Off : "Off", 
                         apg.CoolerStatus_RampingToSetPoint : "Ramping", 
                         apg.CoolerStatus_AtSetPoint : "At Set Pt",
                         apg.CoolerStatus_Revision : "Temp Revisied",
                         apg.CoolerStatus_Suspended : "Suspended"}
        
        atSetCount = 0
        while( atSetCount < 5 ):
            time.sleep( 20 )
            
            cstatus = cam.GetCoolerStatus()
            if( apg.CoolerStatus_AtSetPoint == cstatus ):
                atSetCount += 1
        
            msg = "ccd temp = %0.2f, drive = %0.2f, setpt = %0.2f, status = %s" % ( cam.GetTempCcd(),
                                       cam.GetCoolerDrive(),
                                       cam.GetCoolerSetPoint(),
                                       CoolStatusDict[ cstatus ] )
            testLog.info( msg )
        
        cam.SetImageCount( 1 )
        
        maxBinR = cam.GetMaxBinRows()
        maxImgR = cam.GetMaxImgRows()
        
        maxBinC = cam.GetMaxBinCols()
        maxImgC = cam.GetMaxImgCols()
        
        cam.SetRoiStartRow( 0 )
        rows = maxImgR 
        cam.SetRoiNumRows( rows )
        cam.SetRoiBinRow( 1 )
          
        cam.SetRoiStartCol( 0 )
        cols = maxImgC 
        cam.SetRoiNumCols( cols )
        cam.SetRoiBinCol( 1 )
        
        # take two bais frames
        biasList = []
        for i in range(0,2):
            
            status = None
                   
            cam.StartExposure( 0.00001, False )
            
            while status != apg.Status_ImageReady:
                status = cam.GetImagingStatus()
            
            msg = "Getting Image %d" %(i)
            testLog.info( msg )
            data = cam.GetImage()
            imgName = "bais-img%d-r%d-c%d.bin" % (i, rows, cols)
            fullImgName = os.path.join( outDir, imgName )
            data.tofile( fullImgName )
            
            resultStr = ""
            resultStr +=  "bias info for " + imgName +"\n"
            resultStr +=  "average value,%f,\n" % np.average( data )  
            resultStr +=  "min value,%d,\n" % data.min() 
            resultStr +=  "max value,%d,\n" % data.max() 
            resultStr +=  "median value,%d,\n" % np.median( data )
            
            testLog.info( resultStr )
            biasList.append( data )
            
   
        #subtract the two bais frames
        biasSub = biasList[0] - biasList[1]
        
        minSubValue = biasSub.min()
        testLog.info( "min bias subtract value = %d" % minSubValue )
        if( minSubValue < 0 ):
            testLog.info( "adding abs of min value" )
            biasSub += abs( minSubValue )
            testLog.info( "new bias subtract value = %d" %  biasSub.min() )
            
        testLog.info( "bias subtract stdv = %d" %  biasSub.std() )
        imgName = "subBias-img%d-r%d-c%d.bin" % (i, rows, cols)
        fullImgName = os.path.join( outDir, imgName )
        biasSub.tofile( fullImgName )
        
        #take 2 min dark image
        cam.StartExposure( 120, False )
        status = None
        while status != apg.Status_ImageReady:
            status = cam.GetImagingStatus()
                
        msg = "Getting dark image"
        testLog.info( msg )
        data = cam.GetImage()
        imgName = "dark-img%d-r%d-c%d.bin" % (i, rows, cols)
        fullImgName = os.path.join( outDir, imgName )
        data.tofile( fullImgName )
        
        resultStr = ""
        resultStr +=  "dark img info for " + imgName +"\n"
        resultStr +=  "average value,%f,\n" % np.average( data )  
        resultStr +=  "min value,%d,\n" % data.min() 
        resultStr +=  "max value,%d,\n" % data.max() 
        resultStr +=  "median value,%d,\n" % np.median( data )
        
        testLog.info( resultStr )
            
        cam.CloseConnection()
        testLog.info( "%s COMPLETED" % (testName) )
        return True
    except:
         testLog.exception("%s FALIED with exception" % (testName) )
         return False
        