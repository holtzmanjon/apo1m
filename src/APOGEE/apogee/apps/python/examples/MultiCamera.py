import numpy
import pylibapogee.pylibapogee as apg
import pylibapogee.pylibapogee_setup as SetupDevice

print "Trying to find and connect with camera"
#look for usb cameras first
devices = SetupDevice.GetUsbDevices()

# no usb cameras, then look for ethernet cameras
if( len(devices) < 1 ):
    devices = SetupDevice.GetEthernetDevices()
    
# exception....no cameras anywhere....
if( len(devices) < 1 ):
    raise RuntimeError( "two cameras not found on usb or ethernet" )
    
camList = []
# connect to the first camera
camA = SetupDevice.CreateAndConnectCam( devices[0] )
camA.SetCooler( True )
camA.SetImageCount( 1 )
camList.append( camA  )

# connect to the second camera
camB = SetupDevice.CreateAndConnectCam( devices[1] )
camB.SetCooler( True )
camB.SetImageCount( 1 )

camList.append( camB  )

i = 0
exposeTime = 0.2
for cam in camList:

    #print some basic info
    print "Cam %d: rows = %d, columns = %d" % ( i,
            cam.GetMaxImgRows(), cam.GetMaxImgCols() )

    print "Starting %f sec light exposure with cam %d" % (exposeTime, i) 
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
    
    imgName = "cam-%d-img-.bin" % (i)
    print "Saving image %s to file" % ( imgName )
    data.tofile( imgName )
    i += 1
    
camA.CloseConnection()
camB.CloseConnection()