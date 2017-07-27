"""Apogee Camera Module

This module is the interface between Protien Simple's application code
and Apogee camera libraries.  It provides the camera functionality such
as setting image ROI, digitization speeds, and image acquistion.

Changes:
 * Aug-07-2012: LT : updated this module to work with the libapogee and pylibapogee
library and python module.
"""

#Camera device
import os
import base64
import Image
import time
from logging import getLogger
import multiprocessing
import pylibapogee.pylibapogee as apg
import pylibapogee.SetupDevice as SetupDevice
import numpy    


#Image types
IMAGE_PNG="png"
IMAGE_FITS="fits"

#Supported camera models
MODEL_ALTA_U8300='AltaU-8300'
MODEL_ALTA_U2='AltaU-2'
MODEL_ALTA_F32='AltaF-32'
_DEFAULT_MODEL=MODEL_ALTA_U8300

#Image size by camera type (Horz, Vert)
_U2_IMAGE_SIZE=(1536, 1024)
_U8300_IMAGE_SIZE=(3326, 2504)

#Image file extensions
_EXTN_PNG=".png"
_EXTN_FITS=".fits"

#initialization parameters
_CAM_ID_ONE=1   #Camera enumerated by OS on USB
_CAM_ID_TWO=0   #Not used for USB always zero
_CAM_OPTION=0   #Not used always zero

_TIME_WAIT_IMAGE_READY=5.0       #total time to wait for the image to be ready
_TIME_POLL_IMAGE_READY=0.25      #time to poll for the image to be ready

_PROP_READ_FUNC="Get"
_PROP_WRITE_FUNC="Set"

_PROP_FAN_MODE="FanMode"
_PROP_IMAGING_STATUS="ImagingStatus"
_PROP_COOLER_BACK_OFF_POINT="CoolerBackoffPoint"
_PROP_COOLER_DRIVE="CoolerDrive"
_PROP_COOLER_WRITE="SetCooler"
_PROP_COOLER_READ="IsCoolerOn"
_PROP_COOLER_SET_POINT="CoolerSetPoint"
_PROP_COOLER_STATUS="CoolerStatus"
_PROP_TEMP_CCD="TempCcd"
_PROP_TEMP_HEAT_SINK="TempHeatsink"
_PROP_ROI_BINNING_H="RoiBinCol" 
_PROP_ROI_BINNING_V="RoiBinRow"
_PROP_ROI_PIXELS_H="RoiNumCols"
_PROP_ROI_PIXELS_V="RoiNumRows" 
_PROP_ROI_START_X="RoiStartCol" 
_PROP_ROI_START_Y="RoiStartRow"
_PROP_MAX_BINNNING_H="MaxBinCols"
_PROP_MAX_BINNNING_V="MaxBinRows"
_PROP_FLUSH_BINNING_V ="FlushBinningRows"
_PROP_MAX_EXPOSURE_TIME="MaxExposureTime " 
_PROP_DIGITIZATION_SPEED="CcdAdcSpeed"
_PROP_DIGITIZE_OVER_SCAN_WRITE="SetDigitizeOverscan"
_PROP_DIGITIZE_OVER_SCAN_READ="IsOverscanDigitized"
_PROP_SHUTTER_STATE="ShutterState"
_PROP_IO_PORT_ASSIGNMENT="IoPortAssignment"
_PROP_IO_PORT_DIRECTION="IoPortDirection"
_PROP_IO_PORT_DATA="IoPortData"
_PROP_ADC_BITS="CcdAdcResolution"

_FAN_SPEED_HIGH=apg.FanMode_High
#Camera temperature deg C.
TEMPERATURE_CCD=-20.0

#Camera status
tableCameraStatus={}
tableCameraStatus[apg.Status_DataError]="Data Error"
tableCameraStatus[apg.Status_PatternError]="Pattern Error"
tableCameraStatus[apg.Status_Idle]="Idle"
tableCameraStatus[apg.Status_Exposing]="Exposing"
tableCameraStatus[apg.Status_ImagingActive]="Imaging Active"
tableCameraStatus[apg.Status_ImageReady]="Image Ready"
tableCameraStatus[apg.Status_Flushing]="Flushing"
tableCameraStatus[apg.Status_WaitingOnTrigger]="Waiting On Trigger"
tableCameraStatus[apg.Status_ConnectionError]="Connection Error"

# imaging speed
NORMAL_SPEED=apg.AdcSpeed_Normal
FAST_SPEED=apg.AdcSpeed_Fast

log = getLogger('cellbio.device.camera')

class CameraError(Exception):
    def __init__(self, message):
        self.message=message      
    def __str__(self):
        return "Camera Error: "+self.message
    def __repr__(self):
        return "Camera Error: "+self.message
    
class CameraImageNeverReadyError(CameraError):
    def __init__(self, status):
        self.status=status
        self.message="Image never ready [%s]: %s" % (self.status, tableCameraStatus.get(self.status, "Unknown"))   


class Camera(object):

    def __init__(self):
        self._model = None
        self._imageSizeFull = None
        
        #Useful state to persist
        self._bitsResolution = None 
        self._imageNumpyArray = None
        
        self._isSetup = False

    def setup(self):
        "Setup the camera"
        
        #Precondition
        if self._isSetup:
            return
                
        #look for usb cameras 
        devices = SetupDevice.GetUsbDevices()

        # exception....no cameras anywhere....
        if( len(devices) == 0 ):
                raise CameraError( "No apogee cameras found on usb the bus" )

        # exception....too many cameras....
        if( len(devices) > 1 ):
                raise CameraError( "more that one apogee camera found on the usb bus" )

       
        try:
            # connect to the
            # this will throw if there is an error thus no longer  
            self.cam = SetupDevice.CreateAndConnectCam( devices[0] )

            self.reset()

            #Performed on initialization only:

            #Persisted since called often
            self._model=self.readModel()

            #Determine the full image size
            sizeX=self.roiPixelsH()
            sizeY=self.roiPixelsV()
            self._imageSizeFull=(sizeX, sizeY)
            
            self._isSetup = True
        except:
            #TODO log error message from exception
            raise CameraError("Camera driver initialization failed")
        
    def reset(self):
        """Reset camera to initialized known state
        """
        #Do a system reset to ensure known state, flushing enabled etc 
        self.cam.Reset()

        #Normal mode
        self.cam.SetCameraMode( apg.CameraMode_Normal )

        #IO ports
        self.ioPortAssignment(0)    #all user defined
        self.ioPortDirection(0)     #Input for all

        self.digitizeOverScan(False)
                              
        #Important for CCD clearing
        self.flushBinningV(16)
        
        self.digitizationSpeed( NORMAL_SPEED )
        
        self.coolerEnable(True)
        self.coolerSetPoint(TEMPERATURE_CCD)
        self.fanSpeed(_FAN_SPEED_HIGH)

        #Shutter
        self.shutterState( apg.ShutterState_Normal )
        
    def isSetup(self):
        return self._isSetup

    def shutdown(self):
        "Shutdown the camera"
        self.cam.CloseConnection()
        self._isSetup = False
        
    def clear(self, forceShutterClosed=True):
        """Clear the camera
        -takes a short image and flushes it
        -retains shutter state
        """
        #Switch to normal speed for clearing
        self.digitizationSpeed( NORMAL_SPEED )

        #Save shutter state to reset it
        storedShutterState = self.shutterState()
        
        #Force shutter closed
        if forceShutterClosed:
           self.shutterState( apg.ShutterState_ForceClosed )

        #Set binning to clear
        self.roiPixelsH(1)
        self.roiPixelsV(1)

        self.expose(0.1, True)
        self.getImage()

        #Reset shutter state
        self.shutterState( storedShutterState )        

    def expose(self, exposureTime=1.0, light=True):
        """Expose to take a image and save to a file if a file is given
        Note: Every image acquired by the camera has a sort of life cycle associated with it. An image begins with a
        call to the Expose method. It ends with a call to either the StopExposure to GetImage method
        """
        self.cam.StartExposure(exposureTime, light)
        
    def stopExposure(self, digitize=False):
        """Stops an exposure already in progress
        If StopExposure is called, and there is no exposure in progress, the method has no effect.
        digitize - False indicates the application will not try to retrieve the image data
        """
        self.cam.StopExposure(digitize)
        
    def getImage(self):
        """Get a image
        """        
        endTime=time.time()+_TIME_WAIT_IMAGE_READY
        status=self.cam.GetImagingStatus()
        while status!=apg.Status_ImageReady and time.time()<endTime:
            time.sleep(_TIME_POLL_IMAGE_READY)
            status=self.cam.GetImagingStatus()
            
        if status!=apg.Status_ImageReady:
            raise CameraImageNeverReadyError(status)
        
        #Get image into a array
        self._imageNumpyArray = self.cam.GetImage()        
    
        
    def getImageSize(self):
        "Return a tuple containing the image size for this camera (Horiz, Vert)"
        return self._imageSizeFull        
    
    def saveImageProc(self, fileName, imageType=IMAGE_PNG, make8Bit=False,
                      rotation=0, cropBox=None):
        "Save last exposured image to a file in a separate process and return its path"
        filePath=self._fileExtension(fileName, imageType)
        imageSize=self._calcuateDownloadImageSize()
        
  
        def inner(imageArray, filePath, imageSize, imageType, bitsResolution,
                  make8Bit, rotation, cropBox):  
            if imageType != IMAGE_FITS:
                #Write out image to file            
                imagePng=self._convertPng(imageArray, imageSize, bitsResolution,
                                          make8Bit)
                #Rotate image
                if rotation:
                    imagePng=imagePng.rotate(rotation)
                    
                #Crop image
                if cropBox:
                    imagePng=imagePng.crop(cropBox)
                
                imagePng.save(filePath)
            else:
                imageFits=self._convertFits(self._imageNumpyArray)
                
                #Rotate image
                if rotation:
                    imageFits=imageFits.rotate(rotation)
                    
                #Crop image
                if cropBox:
                    imageFits=imageFits.crop(cropBox)

                imageFits.writeto(filePath)
                
        p = multiprocessing.Process(target=inner, args=(self._imageNumpyArray, filePath,
                                                        imageSize, imageType, self._bitsResolution,
                                                        make8Bit, rotation, cropBox))
        p.start()
        p.join()
            
        return filePath

    def imagingStatus(self):
        return self._propertyReadWrite(_PROP_IMAGING_STATUS)
    
    def _calcuateDownloadImageSize(self):
        "Determine the image size"
        sizeX=self.roiPixelsH()
        sizeY=self.roiPixelsV()
        return (sizeX, sizeY)

    def serialNumber(self):
        "Camera serial number"
        return self.cam.GetSerialNumber()
       
    def model(self):
        "Get the camera model (obtained during setup)"
        return self._model
        
    def readModel(self):
        "Read the camera model"
        return self.cam.GetModel()
        
    def shutterState(self,  *val):
        """Shutter State
        0 = ShutterState_Unkown
        1 = ShutterState_Normal
        2 = ShutterState_ForceOpen
        3 = ShutterState_ForceClosed 	
        """
        return self._propertyReadWrite(_PROP_SHUTTER_STATE, *val)

    def fanSpeed(self, *speed):
        """Fan speed:
        FanMode_Off
        FanMode_Low
        FanMode_Medium
        FanMode_High
        FanMode_Unknown     
        """
        return self._propertyReadWrite(_PROP_FAN_MODE, *speed)

    def coolerEnable(self, *enable):    
        """Cooler enable
        True = on
        False = off
        """
        return self._propertyReadWriteEx(_PROP_COOLER_WRITE, _PROP_COOLER_READ, *enable)
               
    def coolerSetPoint(self, *setPoint):    
        "Cooler set point"
        return self._propertyReadWrite(_PROP_COOLER_SET_POINT, *setPoint)        
    
    def coolerBackOffPoint(self, *backOffPoint):    
        "Cooler back off point"
        return self._propertyReadWrite(_PROP_COOLER_BACK_OFF_POINT, *backOffPoint)        

    def coolerStatus(self):    
        """Cooler status. Read Only.
        0 = CoolerStatus_Off    
        1 = CoolerStatus_RampingToSetPoint  
        2 = CoolerStatus_AtSetPoint     
        3 = CoolerStatus_Revision   (until setpoint read)
        4 = CoolerStatus_Suspended 
        """
        return self._propertyRead(_PROP_COOLER_STATUS)        
    
    def coolerDrive(self):    
        """Cooler drive level as a percentage 0% to 100%
        Read only.
        """
        return self._propertyRead(_PROP_COOLER_DRIVE)        
    
    def temperatureCCD(self):    
        """CCD Temperature. Read only.
        """
        return self._propertyRead(_PROP_TEMP_CCD)        

    def temperatureHeatSink(self):    
        """Heat Sink Temperature. Read only
        """
        return self._propertyRead(_PROP_TEMP_HEAT_SINK)        
    
    def binHorz(self, binH=1):
        "bin horizontal"
        pixels=self._imageSizeFull[0]/binH
        self._propertyReadWrite(_PROP_ROI_BINNING_H, binH)        
        self._propertyReadWrite(_PROP_ROI_PIXELS_H, pixels)  

    def binVert(self, binV=1):
        "bin Vertical"
        pixels=self._imageSizeFull[1]/binV
        self._propertyReadWrite(_PROP_ROI_BINNING_V, binV)        
        self._propertyReadWrite(_PROP_ROI_PIXELS_V, pixels)
         
    def roiBinningH(self, *binning):    
        "ROI binning horizontal"
        return self._propertyReadWrite(_PROP_ROI_BINNING_H, *binning)        
    
    def roiBinningV(self, *binning):    
        "ROI binning vertical"
        return self._propertyReadWrite(_PROP_ROI_BINNING_V, *binning)        
    
    def roiPixelsH(self, *pixels):    
        "ROI pixels horizontal"
        return self._propertyReadWrite(_PROP_ROI_PIXELS_H, *pixels)        
    
    def roiPixelsV(self, *pixels):    
        "ROI pixels vertical"
        return self._propertyReadWrite(_PROP_ROI_PIXELS_V, *pixels)        

    def roiStartX(self, *start):    
        "ROI start X"
        return self._propertyReadWrite(_PROP_ROI_START_X, *start)        
    
    def roiStartY(self, *start):    
        "ROI start Y"
        return self._propertyReadWrite(_PROP_ROI_START_Y, *start)        

    def maxBinningH(self, *max):    
        "Maximum binning horizontal"
        return self._propertyReadWrite(_PROP_MAX_BINNNING_H, *max)        
    
    def maxBinningV(self, *max):    
        "Maximum binning vertical"
        return self._propertyReadWrite(_PROP_MAX_BINNNING_V, *max)
    
    def flushBinningV(self, *flush):    
        "Flush binning vertical"
        return self._propertyReadWrite(_PROP_FLUSH_BINNING_V, *flush)
      
    def maxExposureTime(self, *time):    
        "Maximum exposure Time"
        return self._propertyReadWrite(_PROP_MAX_EXPOSURE_TIME, *time)

    def adcResolution(self):
        """Get adc bits. Read only.
        Resolution_SixteenBit   
        Resolution_TwelveBit 
        """
        return self._propertyRead(_PROP_ADC_BITS)   

    def digitizationSpeed(self, *digSpeed):    
        """Digitization Speed: 
        0 = AdcSpeed_Unknown    
        1 = AdcSpeed_Normal     
        2 = AdcSpeed_Fast   
        3 = AdcSpeed_Video
        """        
        if 0<len(digSpeed):
            result = self._propertyReadWrite(_PROP_DIGITIZATION_SPEED, *digSpeed )
            # LT: for altau's setting the digitization speed to fast sets 12bit mode
            # and normal is 16bit.  ascent's and altaf's only have 16bit digitization
            # reguardless of speed
            self._bitsResolution = self.adcResolution()
            return result 
        else:
            return self._propertyReadWrite(_PROP_DIGITIZATION_SPEED, *digSpeed)      

    def digitizeOverScan(self, *overScan):    

        "Digitize over scan"

        return self._propertyReadWriteEx(_PROP_DIGITIZE_OVER_SCAN_WRITE,
                                         _PROP_DIGITIZE_OVER_SCAN_READ, *overScan)
                                       
    def ioPortAssignment(self, *assignment):    
        "IO port assignment"
        return self._propertyReadWrite(_PROP_IO_PORT_ASSIGNMENT, *assignment)        

    def ioPortDirection(self, *direction):    
        "IO port direction"
        return self._propertyReadWrite(_PROP_IO_PORT_DIRECTION, *direction)        

    def ioPortData(self, *data):    
        "IO port data"
        return self._propertyReadWrite(_PROP_IO_PORT_DATA, *data)    
    
    def takeTestImage(self, binH=2, binV=2, expTime=0.01, rotation=0, cropBox=None, parentDir=".", name=""):
        "Take a test image, return (true, file path) or (false, error str) if not succesful"
        result=None
        isSuccesful=None
        
        try:
            fileName="%s_%s_H%d_V%d_E%.3f" % (self._timeStamp(), name, binH, binV, expTime)
            filePath=parentDir+'/'+fileName #saveImage adds file extension, so full path not yet complete
            
            self.clear() #Needed not sure why
            self.binHorz(binH);
            self.binVert(binV)
            self.expose(expTime)
            time.sleep(expTime)
            self.getImage()
            result=self.saveImageProc(filePath, rotation=rotation, cropBox=cropBox)
            isSuccesful = True
        
        except Exception, e:
            result = e.__str__()
            isSuccesful = False            
        
        return (isSuccesful, result)               
    
    def _propertyRead(self, property):
        """read helper function"""
        getter=getattr(self.cam, _PROP_READ_FUNC+property)
        value=getter()
        
        return value
                              
    def _propertyReadWrite(self, property, *value):
        """read and write helper function"""
        lenValue=len(value)
        if lenValue==0:
            getter=getattr(self.cam, _PROP_READ_FUNC+property)
            value=getter()
        elif lenValue==1:
            setter=getattr(self.cam, _PROP_WRITE_FUNC+property)
            setter(value[0])          
            
            getter=getattr(self.cam, _PROP_READ_FUNC+property)
            value=getter()
            
        else:
            raise CameraError("Property write Only 1 value allowed")
            
        return value

    def _propertyReadWriteEx(self, write_property, read_property, *value):
        """read and write helper function for functions that are not
        Get/Set symmetrical"""
        lenValue=len(value)
        if lenValue==0:
            getter=getattr(self.cam, read_property)
            value=getter()
        elif lenValue==1:
            setter=getattr(self.cam, write_property)
            setter(value[0])          
            
            getter=getattr(self.cam, read_property)
            value=getter()
            
        else:
            raise CameraError("Property write Only 1 value allowed")
            
        return value
    
    def _timeStamp(self):
        return time.strftime('%Y-%m-%d-%H%M.%S')
    
    @staticmethod
    def _convertPng(imageNumpyArray, imageSize, bitsResolution=apg.Resolution_SixteenBit, make8Bit=False):
        "Convert the image pixel short array to a 16 bit gray scale png"        
        
        #Convert to string I 32 bit gray scale or 8 bit
        # see http://www.pythonware.com/library/pil/handbook/decoder.htm
        #Image.fromstring(mode, size, data, raw mode, stride, orientation)
        #orientation 1 start at top
            
        #LT: sticking with PIL fromstring function, because it appears that the PIL fromarray
        # function does not support the uint16 data type, which of course is what the camera
        # produces.
        if make8Bit: #save as 8 bit
            if apg.Resolution_TwelveBit==bitsResolution:
                #"I;16" 16 bits
                imgPng=Image.fromstring("I", imageSize, imageNumpyArray.tostring(), "raw", "I;16", 0,1)
                #Shift 4 bits and save a 8-bit
                imgPng = imgPng.point(lambda i: i*0.0625, "L")
                imgPng = imgPng.convert('L')
            else:
                #Shift string and use only MSB of each 2 bytes
                imgStrShift=imageNumpyArray.tostring()[1::2]
                #8 bit gray scale image
                imgPng=Image.fromstring("L", imageSize, imgStrShift, "raw", "L", 0,1)
                    
        else: #16 bit image save as 16 bit
            #"I;16" 16 bits
            imgPng=Image.fromstring("I", imageSize, imageNumpyArray.tostring(), "raw", "I;16", 0,1)
            
        return imgPng
    
    @staticmethod
    def _convertFits(imgNumpyArray):
        "Convert the image string to a 16 bit FITS image"
        
        #Convert string to numpy array    
        imageArray= numpy.reshape(imgNumpyArray, (1024, -1))

        #Create fits image using pyFits
        import pyfits
        hdu=pyfits.PrimaryHDU(imageArray)        
        hdulist=pyfits.HDUList([hdu])

        return hdulist
        
    def _fileExtension(self, fileName, imageType):
        "Add a file extension if needed"

        #Add file extension if needed
        extn = _EXTN_FITS if imageType == IMAGE_FITS else _EXTN_PNG

        if fileName[-4:].lower() != extn:
            fileName+=extn

        return fileName

    def convertBase64(self, fileName):    
        """get a image data encoded as binary64"""
        import array
        imageFileName=fileName+EXTN     
    
        imageBinArray=array.array('B')
        imageFileSize=statinfo=os.stat(imageFileName).st_size
        
        #Read from file into binary array
        fh=open(imageFileName, 'rb')
        imageBinArray.fromfile(fh, imageFileSize)
        fh.close()
                  
        #encode array string into base 64
        imageEncodeStr=base64.standard_b64encode(imageBinArray.tostring())
          
        #print "imagesize ",len(imageEncodeStr), " imagesize encoded ",imageFileSize
            
        return imageEncodeStr

if __name__=="__main__":
    from cellbio.core.Configuration import config 
    c=Camera()
    c.setup()
    result = c.takeTestImage(name='Dev', parentDir=config.getTestImagesDir())
    print "Is successful: %s, FileName: %s" % result
    
