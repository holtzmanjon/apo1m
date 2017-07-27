import Camera
import logging
import sys
import time
import os

# set up logging to file - see previous section for more details
logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    datefmt='%m-%d %H:%M',
                    filename='camera.log',
                    filemode='w')
# define a Handler which writes INFO messages or higher to the sys.stderr
console = logging.StreamHandler()
console.setLevel(logging.INFO)
# set a format which is simpler for console use
formatter = logging.Formatter('%(name)-12s: %(levelname)-8s %(message)s')
# tell the handler to use this format
console.setFormatter(formatter)
# add the handler to the root logger
logging.getLogger('').addHandler(console)

testLog = logging.getLogger('camera')

try:

    theCam = Camera.Camera()
    
    theCam.setup()

    for i in range(0, 10000):
        t0=time.time()
        numStr = "%05d" % ( i )
        
        #Check Temp
        status = theCam.coolerStatus()
        if status != 2:
            temperature = theCam.temperatureCCD()
            testLog.info("Camera is not at setpoint temperature expected: %.1f at: %.1f"\
                         % (Camera.TEMPERATURE_CCD, temperature))
        
        #Clear
        testLog.info( "Run %s: starting clear image exposure" % numStr )
        #LT: edit for updated apogee library
        #1 = ShutterState_Normal
        #2 = ShutterState_ForceOpen
        #3 = ShutterState_ForceClosed
        theCam.shutterState( 2 )
        theCam.clear(forceShutterClosed=False)
        
        #Take actual image
        expTime = 4.0
        theBin = 7
        #LT: edit for updated apogee library
        #1 = AdcSpeed_Normal    
        #2 = AdcSpeed_Fast
        theCam.digitizationSpeed( 2 )

        theCam.binHorz( theBin )
        theCam.binVert( theBin )
        testLog.info( "Run %s: starting fast %f sec exposure, bin = %d"\
                     % (numStr, expTime, theBin ) )
        theCam.expose( expTime+0.2, True )
        time.sleep(expTime) #Here, we wait and apply fluorescence (no camera operations)
        img = theCam.getImage() #Error occurs here
        
        #Save image camera calls
        imageSize=theCam._calcuateDownloadImageSize()
        
        #periodic image function sleep
        periodInterval = 10.0
        functionTime = time.time() - t0
        nextIntervalTime = periodInterval - functionTime
        if nextIntervalTime<0:
            testLog.warning("Function time %f longer than period %f"\
                        % (functionTime, periodInterval))            
        time.sleep( max(0, nextIntervalTime) )
        
    testLog.info( "test succssuflly COMPLETED" )
    
except Exception, e:
    testLog.exception( "FALIED with exception" )


