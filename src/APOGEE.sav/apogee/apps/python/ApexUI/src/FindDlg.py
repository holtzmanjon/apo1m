'''
Copyright(c) 2010 Apogee Instruments, Inc.
Created on Feb 23, 2010

@author: ltraynor
'''
import re
import socket
import string
import time
import subprocess
import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import dlgFindUi
import pyApg as apg

            
class FindDlg(qtGui.QDialog):
    '''
    Simple dialog for searching for Apogee devices
    on the USB bus and on the local network
    '''
    def __init__(self,parent=None):
        '''
        Constructor
        '''
        super(FindDlg,self).__init__(parent)
        
        self.apgIcon = qtGui.QIcon("apogeeIcon.ico")
        self.setWindowIcon( self.apgIcon  )
        # Set up the user interface from Designer.
        self.ui = dlgFindUi.Ui_dlgFind()
        self.ui.setupUi(self)   
        
        self.connect(self.ui.btnFind, 
                     qtCore.SIGNAL("clicked()"),
                     self.OnFind);
                     
        self.deviceDictList = []
        self.selectedDevice = {}
        
        ipAddr = socket.gethostbyname( socket.gethostname() )
        temp = ipAddr.split(".")
        temp[3] = '255'
        subNet = string.join(temp, ".")
        self.ui.editEthernetSubnet.setText( unicode(subNet) )
        
    def accept(self):
        row = self.ui.tblDevices.currentRow()
        self.selectedDevice = self.deviceDictList[ row  ]
        qtGui.QDialog.accept(self)
        
    def OnDevicePickedInTable(self):
        row = self.ui.tblDevices.currentRow()
        status = self.deviceDictList[ row  ]["interfaceStatus"]
        
        #if the item selected is occupied
        # tell the user an unselect the item 
        if(-1 != status.find("Occupied") ):
            self.ui.tblDevices.setCurrentRow(0)
        else:
            self.ui.btnSelect.setEnabled( True )
                
    def OnFind(self):
        self.ui.tblDevices.clearContents ()
        self.ui.tblDevices.setRowCount( 0 )
        resultStr = ""
        if self.ui.cBoxUsb.isChecked():
            self.ui.pBarFind.setValue(0)
            usbFind = apg.FindDeviceUsb()
            self.ui.pBarFind.setValue(10)
            resultStr += usbFind.Find()
            
        if self.ui.cBoxEthernet.isChecked():
            subnet = unicode(self.ui.editEthernetSubnet.text()).encode()
            args = ['EthernetFinder.exe', '-s',subnet]
    
            p = subprocess.Popen(args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            returnVal = None
            num = 0
            while( None == returnVal):
                self.ui.pBarFind.setValue(num)
                time.sleep(1)
                num += 1
                returnVal = p.poll() 

            if returnVal:
                raise RuntimeError(p.communicate()[1])
            
            resultStr += p.communicate()[0]
        
        self.ui.pBarFind.setValue(0)
        
        self.deviceDictList = self.ParseDeviceStr( resultStr )
        if( len(self.deviceDictList) == 0 ):
            qtGui.QMessageBox.information(self,"INFO", "No devices found.")
            return
        
            
        #update the table with the information
        for row in self.deviceDictList:
            numRows = self.ui.tblDevices.rowCount()
            self.ui.tblDevices.insertRow( numRows )
            
            modelItem = qtGui.QTableWidgetItem( row["model"] )
            self.ui.tblDevices.setItem(numRows, 0, modelItem)
            
            interfaceItem = qtGui.QTableWidgetItem( row["interface"] )
            self.ui.tblDevices.setItem(numRows, 1, interfaceItem)
            
            addressItem = qtGui.QTableWidgetItem( row["address"] )
            self.ui.tblDevices.setItem(numRows, 2, addressItem)
            
            interfaceStatus = qtGui.QTableWidgetItem(  row["interfaceStatus"])
            self.ui.tblDevices.setItem(numRows, 3, interfaceStatus)
                                   
    def ParseDeviceStr(self,deviceStr):
        
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
            mm = re.search("deviceType=(.*?)[,|<]", device)
            devDict["deviceType"] = mm.group(1)
            
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
                        
         
            
            if( "filterWheel" == devDict["deviceType"] ):
                devDict["model"] = "Filter Wheel"       
                devDict["camType"] =  "NA"  
                devDict["interfaceStatus"] = "NA"
            else:
                mm = re.search("interfaceStatus=(.*?)[,|<]", device)
                status = mm.group(1).replace("\"","")
                devDict["interfaceStatus"] = status
                  
                mm = re.search("model=(.*?)[,|<]", device)
                devDict["model"] = mm.group(1)        
                        
                devDict["camType"] = devDict["model"].split("-")[0]
                
            deviceDictList.append(devDict)
        
        return deviceDictList
        
          
    