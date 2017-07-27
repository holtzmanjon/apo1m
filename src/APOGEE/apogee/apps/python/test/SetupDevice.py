'''
Copyright(c) 2011 Apogee Imaging Systems


@author: ltraynor
'''
import re
import logging
from pylibapogee import pylibapogee as apg

testLog = logging.getLogger('apogeeTest')

def GetUsbDevices():
    f = apg.FindDeviceUsb()
    msg = f.Find()
    
    return ParseDeviceStr( msg )
    
    
def GetEthernetDevices():
    f = apg.FindDeviceEthernet()
    msg = f.Find( "192.168.0.255" )
    
    return ParseDeviceStr( msg )
    
def ParseDeviceStr(deviceStr):
    #MUST include the < in the grouping, so the regex
        #search functions below will find the last item in the
        #string
        deviceStrList = re.findall("<d>(.*?<)/d>", deviceStr)
 
        deviceDictList = []
        
        for device in deviceStrList:
            # 1 here, because the match above will
            # always pick up the < making the device
            # string len == 1
            if( 1 >= len(device) ):
                #nothing to parse move to the next 
                #item in the list
                continue
        
            devDict = {}
            mm = re.search("interface=(.*?)[,|<]", device)
            devDict["interface"] = mm.group(1)
            
            mmA = re.search("address=(.*?)[,|<]", device)
            
            if "ethernet" == devDict["interface"]:
                mmP = re.search("port=(.*?)[,|<]", device)
                devDict["address"] = mmA.group(1) + ":" + mmP.group(1)
            else:
                devDict["address"] = mmA.group(1)
                
            mm = re.search("id=(.*?)[,|<]", device)
            devDict["id"] = mm.group(1)  
            
            mm = re.search("firmwareRev=(.*?)[,|<]", device)
            devDict["firmwareRev"] = mm.group(1)
            
            mm = re.search("model=(.*?)[,|<]", device)
            devDict["model"] = mm.group(1) 
            
            mm = re.search("interfaceStatus=(.*?)[,|<]", device)
            status = mm.group(1).replace("\"","")
            devDict["interfaceStatus"] = status  
            
            if( len(devDict) != 6):
                #this device didn't contain the correct data
                #go to the next device
                continue
                    
            devDict["camType"] = devDict["model"].split("-")[0]
            deviceDictList.append(devDict)
        
        return deviceDictList
    
def CreateAndConnectCam( devDict ):
    cam = None
    if( "AltaU" == devDict["camType"] or
       "AltaE" == devDict["camType"] ):
        cam = apg.Alta()
    
    if( "Ascent" == devDict["camType"] ):
        cam = apg.Ascent()
    
    cam.OpenConnection( devDict["interface"],
                       devDict["address"],
                       int(devDict["firmwareRev"],16),
                       int(devDict["id"],16) )
    
    cam.Init()
    
    testLog.info( "Camera %s connected and initialized via %s" %
                 ( cam.GetModel(), devDict["interface"] ) )
    return cam
    