import numpy
import pylibapogee.pylibapogee as apg
import pylibapogee.pylibapogee_setup as SetupDevice
import random

random.seed()

print "Trying to find and connect with camera"
#look for usb cameras first
devices = SetupDevice.GetUsbDevices()

# no usb cameras, then look for ethernet cameras
if( len(devices) == 0 ):
    devices = SetupDevice.GetEthernetDevices()
    
# exception....no cameras anywhere....
if( len(devices) == 0 ):
    raise RuntimeError( "No devices found on usb or ethernet" )
    
# connect to the first camera
devDict = devices[0]
cam = None
if( "AltaU" == devDict["camType"] or
   "AltaE" == devDict["camType"] ):
    cam = apg.Alta()

if( "Ascent" == devDict["camType"] ):
    cam = apg.Ascent()

if( "AltaF" == devDict["camType"]  ):
    cam = apg.AltaF()
    
statusList = [apg.Status_Exposing, apg.Status_Flushing, apg.Status_ImageReady]
statusDict = { apg.Status_Exposing : "exposing", apg.Status_Flushing : "flushing",
           apg.Status_ImageReady : "image ready"}

for i in range(0,25):
    
    index = random.randint( 0, len(statusList)-1 )

    closeMeStatus = statusList[index]
    print "testing close on status " + statusDict[closeMeStatus]
    
    cam.OpenConnection( devDict["interface"],
                       devDict["address"],
                       int(devDict["firmwareRev"],16),
                       int(devDict["id"],16) )
    
    cam.Init()
    
    #print some basic info
    print "Imaging rows = %d, columns = %d" % ( cam.GetMaxImgRows(), cam.GetMaxImgCols() )
    
    cam.SetCooler( True )
    
    cam.SetImageCount( 1 )
    
    exposeTime = 0.2
    print "Starting %f sec light exposure" % (exposeTime) 
    cam.StartExposure( exposeTime, True )
                    
        
    if(cam.GetImagingStatus() == closeMeStatus):
        print "stoping tests on status " + statusDict[closeMeStatus]
        cam.CloseConnection()
        continue
    
    status = None
    while status != apg.Status_ImageReady:
        
        status = cam.GetImagingStatus() 
        if( apg.Status_ConnectionError == status or
            apg.Status_DataError == status or
            apg.Status_PatternError == status ):
            msg = "Run %s: FAILED - error in camera status = %d" % (runStr, status)
            raise RuntimeError( msg )
    
    if(cam.GetImagingStatus() == closeMeStatus):
        print "stoping tests on status " + statusDict[closeMeStatus]
        cam.CloseConnection()
        continue
    
    print "getting image"
    img = cam.GetImage()
    
    # sometimes the altau doesn't go right to the flushing state
    # waiting a couple of cycles for it to go to flushing, probably
    # a stale status vaule b/c of old FX2 code
    status = cam.GetImagingStatus()
    while status != apg.Status_Flushing:
        print "waiting for flushing %d " % (status)
        status = cam.GetImagingStatus()

        
    if(cam.GetImagingStatus() == closeMeStatus):
        print "stoping tests on status " + statusDict[closeMeStatus]
        cam.CloseConnection()
        continue
    
    print "closing connection without vaild status"
    cam.CloseConnection()
    