'''
Created on Aug 2, 2010

@author: ltraynor
'''

import time
import os
import TabBase
import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import ApexUi
import pyApg as apg

class ApexStatusBar(TabBase.TabBase):
    '''
    Class that updates the camera status bar at
    the bottom of the Apex window.  This class
    is derived from the TabBase for the support
    functions even though it isn't strictly
    a tab in the UI.
    '''
    def __init__(self,theUi,parent=None):
        '''
        Constructor
        '''
        super(ApexStatusBar,self).__init__(theUi,parent)
        
        self.CoolStatusDict = {apg.CoolerStatus_Off : unicode("Off"), 
                         apg.CoolerStatus_RampingToSetPoint : unicode("Ramping"), 
                         apg.CoolerStatus_AtSetPoint : unicode("At Set"),
                         apg.CoolerStatus_Revision : unicode("Revision"),
                         apg.CoolerStatus_Suspended : unicode("Suspended")}
        
        self.CamStatusDict = {apg.Status_ConnectionError : unicode("Cnnct Err"), 
                        apg.Status_DataError : unicode("Data Err"), 
                        apg.Status_PatternError : unicode("Ptrn Err"),
                        apg.Status_Idle : unicode("Idle"),
                        apg.Status_Exposing : unicode("Exposing"),
                        apg.Status_ImagingActive : unicode("Img Active"),
                        apg.Status_ImageReady : unicode("Img Ready"),
                        apg.Status_Flushing : unicode("Flushing"),
                        apg.Status_WaitingOnTrigger : unicode("Wait on Trig")}
        
        self.LoggingOn = False
        self.LogFile = None
        self.LogInterval = 100
        self.LogDelay = 0
        
        
    def GetCoolerStatusStr(self):
        status = self.camRef.GetCoolerStatus()
        return self.CoolStatusDict[ status ]
    
    def CheckCamStatus(self):
        status = self.camRef.GetImagingStatus()
        return self.CamStatusDict[ status ]
        
    def UpdateStatus(self):
        camStatusStr  = self.CheckCamStatus()
        self.ui.editStatusCamStatus.setText( camStatusStr )
            
        ccdTempStr = unicode( " %3.1f" % self.camRef.GetTempCcd() )
        self.ui.editStatusTempCcd.setText( ccdTempStr )
        
        heatsinkTempStr = unicode( " %3.1f" % self.camRef.GetTempHeatsink() )
        self.ui.ediStatusTempHeatsink.setText( heatsinkTempStr )
        
        coolerStatusStr = self.GetCoolerStatusStr()
        self.ui.editStatusCoolState.setText( coolerStatusStr )
        
        camDriveStr = unicode( " %3.1f" % self.camRef.GetCoolerDrive() )
        self.ui.editStatusCoolDrive.setText( camDriveStr )
        
        if self.LoggingOn:
            if( self.LogDelay >= self.LogInterval ):
                self.LogDelay = 0 
                coolerSetPtStr = unicode( " %3.1f" % self.camRef.GetCoolerSetPoint() )
                self.LogFile.write( "%s,%s,%s,%s,%s\n" % (coolerSetPtStr,camDriveStr,
                                                 ccdTempStr, heatsinkTempStr, coolerStatusStr) )
            else:
                self.LogDelay += 1
  
           

        