import numpy
import pylibapogee.pylibapogee as apg
import pylibapogee.pylibapogee_setup as SetupDevice

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
cam = SetupDevice.CreateAndConnectCam( devices[0] )

#print some basic info
print "Imaging rows = %d, columns = %d" % ( cam.GetMaxImgRows(),
								cam.GetMaxImgCols() )

cam.SetCooler( True )

cam.SetImageCount( 1 )

exposeTime = 0.2
print "Starting %f sec light exposure" % (exposeTime) 
cam.StartExposure( exposeTime, True )
                
status = None
while status != apg.Status_ImageReady:
    
    status = cam.GetImagingStatus()	
    if( apg.Status_ConnectionError == status or
        apg.Status_DataError == status or
        apg.Status_PatternError == status ):
        msg = "Run %s: FAILED - error in camera status = %d" % (runStr, status)
        raise RuntimeError( msg )
        
print "Getting image"
data = cam.GetImage()

print "Saving image to file"
imgName = "simpleExpose.bin"
data.tofile( imgName )
					
cam.CloseConnection()