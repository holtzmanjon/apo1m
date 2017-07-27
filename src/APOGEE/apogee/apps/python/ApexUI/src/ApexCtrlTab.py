'''
Created on Mar 9, 2010

@author: ltraynor
'''

import TabBase
import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import ApexUi
import pyApg as apg


class ApexCtrlTab(TabBase.TabBase):
    '''
    Supports the controls ui tab widget
    '''


    def __init__(self,theUi,parent=None):
        '''
        Constructor
        '''
        super(ApexCtrlTab,self).__init__(theUi,parent)
        
        #mapping fan enums to fan combo box indices
        self.FanModeDict = {  0 : apg.FanMode_Off,
                            1 : apg.FanMode_Low,
                            2 : apg.FanMode_Medium,
                            3 : apg.FanMode_High }
        
        self.LedModeDict = { 0 : apg.LedMode_EnableAll,
                            1 : apg.LedMode_DisableWhileExpose,
                            2 : apg.LedMode_DisableAll }
        
        self.LedStateDict = { 0 : apg.LedState_Expose,
                             1 : apg.LedState_ImageActive,
                             2 : apg.LedState_Flushing,
                             3 : apg.LedState_ExtTriggerWaiting,
                             4 : apg.LedState_ExtTriggerReceived,
                             5 : apg.LedState_ExtShutterInput,
                             6 : apg.LedState_ExtStartReadout,
                             7 : apg.LedState_AtTemp }
        
        self.Sig1Mask = 1
        self.Sig2Mask = 2
        self.Sig3Mask = 4
        self.Sig4Mask = 8
        self.Sig5Mask = 16
        self.Sig6Mask = 32
        
        #manually added rb's to button groups here for 2 reasons
        #one is that rb's with the same parent are mutally excluisve,
        #the button group allows the buttons on the right and left of
        #the group box to be dependent of one another.  second
        #the pyuic isn't picking up the button groups created in the
        #qt designer???
        self.btnGroupA = qtGui.QButtonGroup()
        self.btnGroupA.addButton( self.ui.rbIoSig1In )
        self.btnGroupA.addButton( self.ui.rbIoSig1Out )
        
        self.btnGroupB = qtGui.QButtonGroup()
        self.btnGroupB.addButton( self.ui.rbIoSig2In )
        self.btnGroupB.addButton( self.ui.rbIoSig2Out )
        
        self.btnGroupC = qtGui.QButtonGroup()
        self.btnGroupC.addButton( self.ui.rbIoSig3In )
        self.btnGroupC.addButton( self.ui.rbIoSig3Out )
        
        self.btnGroupD = qtGui.QButtonGroup()
        self.btnGroupD.addButton( self.ui.rbIoSig4In )
        self.btnGroupD.addButton( self.ui.rbIoSig4Out )
        
        self.btnGroupE = qtGui.QButtonGroup()
        self.btnGroupE.addButton( self.ui.rbIoSig5In )
        self.btnGroupE.addButton( self.ui.rbIoSig5Out )
        
        self.btnGroupF = qtGui.QButtonGroup()
        self.btnGroupF.addButton( self.ui.rbIoSig6In )
        self.btnGroupF.addButton( self.ui.rbIoSig6Out )
        
        
    def OnToggleCooler(self):
        if self.ui.btnCtrCoolerToggle.isChecked() :
            self.OnWriteSetPoint()
            self.OnWriteBackoffPoint()
            self.camRef.SetCooler( True )
            self.ui.btnCtrCoolerToggle.setText( unicode("Cooler Off") )
        else:
            self.camRef.SetCooler( False )
            self.ui.btnCtrCoolerToggle.setText( unicode("Cooler On") )
            
    def OnWriteSetPoint(self):
        self.camRef.SetCoolerSetPoint( self.ui.spinCtrlCoolerSet.value() )
        
        #read back the value to update ui
        self.OnReadSetPoint()
        
    def OnReadSetPoint(self):
        self.ui.spinCtrlCoolerSet.setValue( self.camRef.GetCoolerSetPoint() )

    def OnWriteBackoffPoint(self):
        self.camRef.SetCoolerBackoffPoint( self.ui.spinCtrlCoolerBack.value() )
        
        #read back the value to update ui
        self.OnReadBackoffPoint()
        
    def OnReadBackoffPoint(self):
        self.ui.spinCtrlCoolerBack.setValue( self.camRef.GetCoolerBackoffPoint() )
        
    def OnFanMode(self,index):
        self.camRef.SetFanMode( self.FanModeDict[ index ] )
        
    def UpdateFanMode(self):
        mode = self.camRef.GetFanMode()
        index = -1
        for k, v in self.FanModeDict.iteritems():
            if mode == v:
                index = k
                
        if -1 == index:
            raise RuntimeError(" Invalid fan mode %d" % mode)  
              
        self.ui.cbCtrlFan.setCurrentIndex( index )
        
    def OnShutterOperation(self):
        if( self.ui.rbCtrlShutNorm.isChecked() ):
            self.camRef.SetShutterState( apg.ShutterState_Normal )
        elif( self.ui.rbCtrlShutClose.isChecked() ):
            self.camRef.SetShutterState( apg.ShutterState_ForceClosed )
        elif( self.ui.rbCtrlShutOpen.isChecked() ):
            self.camRef.SetShutterState( apg.ShutterState_ForceOpen )
            
    def OnLedMode(self, index):
        self.camRef.SetLedMode( self.LedModeDict[index] )
            
    def UpdateLedMode(self):
        mode =self.camRef.GetLedMode()
        index = -1
        for k, v in self.LedModeDict.iteritems():
            if mode == v:
                index = k
                
        if -1 == index:
            raise RuntimeError(" Invalid led mode %d" % mode)  
              
        self.ui.cbCtrlLedMode.setCurrentIndex( index )
        
    def OnLedAState(self,index):
        self.camRef.SetLedAState( self.LedStateDict[index] )
            
    def OnLedBState(self,index):
        self.camRef.SetLedBState( self.LedStateDict[index] )
        
    def UpdateLedState(self, state, ledCb):
        index = -1
        for k, v in self.LedStateDict.iteritems():
            if state == v:
                index = k
                
        if -1 == index:
            raise RuntimeError(" Invalid led state %d" % state)  
              
        ledCb.setCurrentIndex( index )
        
    def OnGetCamInfo(self):
        camInfo = unicode( self.camRef.GetInfo() )
        self.Write2OutputWin( camInfo )
        qtGui.QMessageBox.information(None, "INFO", camInfo )
        
    def OnWriteShutterCloseDelay(self):
        self.camRef.SetShutterCloseDelay( 
                                         self.ui.spinCtrlShutDelay.value() )
         #read back to display the value we actually wrote
        self.OnReadShutterCloseDelay()
        
    def OnReadShutterCloseDelay(self):
        self.ui.spinCtrlShutDelay.setValue( 
                                        self.camRef.GetShutterCloseDelay() )
        
    def OnWriteShutterStrobePeriod(self):
        self.camRef.SetShutterStrobePeriod( 
                                         self.ui.spinCtrlShutStrobePeriod.value() )
        #read back to display the value we actually wrote
        self.OnReadShutterStrobePeriod()
        
    def OnReadShutterStrobePeriod(self):
        self.ui.spinCtrlShutStrobePeriod.setValue( 
                                        self.camRef.GetShutterStrobePeriod() )
        
    def OnWriteShutterStrobePosition(self):
        self.camRef.SetShutterStrobePosition( 
                                         self.ui.spinCtrlShutStrobePos.value() )
        
         #read back to display the value we actually wrote
        self.OnReadShutterStrobePosition()
        
    def OnReadShutterStrobePosition(self):
        self.ui.spinCtrlShutStrobePos.setValue( 
                                        self.camRef.GetShutterStrobePosition() )
        
    def OnDisableFlushingCmds(self):
        if self.ui.checkCtrlFlushCmdDisabled.isChecked():
            self.camRef.SetFlushCommands( True )
        else:
            self.camRef.SetFlushCommands( False )
            
    def OnPostExposeFlushDisabled(self):
        if self.ui.checkCtrlPostExposeFlushDisabled.isChecked():
            self.camRef.SetPostExposeFlushing( True )
        else:
            self.camRef.SetPostExposeFlushing( False )
            
    def OnWriteIoPort(self):
        value = 0
        
        if( self.ui.rbIoSig1Trig.isChecked() ):
            value = value | self.Sig1Mask
            
        if( self.ui.rbIoSig2Shutter.isChecked() ):
            value = value | self.Sig2Mask
        
        if( self.ui.rbIoSig3ShutterStrobe.isChecked() ):
            value = value | self.Sig3Mask
        
        if( self.ui.rbIoSig4ExternalShutter.isChecked() ):
            value = value | self.Sig4Mask
            
        if( self.ui.rbIoSig5ExtReadout.isChecked() ):
            value = value | self.Sig5Mask
            
        if( self.ui.rbIoSig6TimerPulse.isChecked() ):
            value = value | self.Sig6Mask
        
        self.camRef.SetIoPortAssignment( value )   
        
        direction = 0 
        
        if( self.ui.rbIoSig1Out.isChecked() ):
            direction = direction | self.Sig1Mask
            
        if( self.ui.rbIoSig2Out.isChecked() ):
            direction = direction | self.Sig2Mask
            
        if( self.ui.rbIoSig3Out.isChecked() ):
            direction = direction | self.Sig3Mask
            
        if( self.ui.rbIoSig4Out.isChecked() ):
            direction = direction | self.Sig4Mask
            
        if( self.ui.rbIoSig5Out.isChecked() ):
            direction = direction | self.Sig5Mask
            
        if( self.ui.rbIoSig6Out.isChecked() ):
            direction = direction | self.Sig6Mask
            
        self.camRef.SetIoPortDirection( direction )
        
    def OnSig1(self):
        isUserDefined = False
        if( self.ui.rbIoSig1User.isChecked() ):
            isUserDefined = True
        else:
            self.ui.rbIoSig1In.setChecked( True )
            
        self.SetSig1(isUserDefined)
        
    def SetSig1(self, isUserDefined):
        #sig 1
        if( isUserDefined ):
            self.ui.rbIoSig1User.setChecked( True )
            self.ui.rbIoSig1Trig.setChecked( False )
            self.ui.rbIoSig1In.setEnabled( True )
            self.ui.rbIoSig1Out.setEnabled( True )
        else:
            self.ui.rbIoSig1User.setChecked( False )
            self.ui.rbIoSig1Trig.setChecked( True )
            self.ui.rbIoSig1In.setEnabled( False)
            self.ui.rbIoSig1Out.setEnabled( False )
            
    def OnSig2(self):
        isUserDefined = False
        if( self.ui.rbIoSig2User.isChecked() ):
            isUserDefined = True
        else:
            self.ui.rbIoSig2In.setChecked( True )
            
        self.SetSig2(isUserDefined)
        
    def SetSig2(self, isUserDefined):

        if( isUserDefined ):
            self.ui.rbIoSig2User.setChecked( True )
            self.ui.rbIoSig2Shutter.setChecked( False )
            self.ui.rbIoSig2In.setEnabled( True )
            self.ui.rbIoSig2Out.setEnabled( True )
        else:
            self.ui.rbIoSig2User.setChecked( False )
            self.ui.rbIoSig2Shutter.setChecked( True )
            self.ui.rbIoSig2In.setEnabled( False )
            self.ui.rbIoSig2Out.setEnabled( False )
        
    def OnSig3(self):
        isUserDefined = False
        if( self.ui.rbIoSig3User.isChecked() ):
            isUserDefined = True
        else:
            self.ui.rbIoSig3In.setChecked( True )
            
        self.SetSig3(isUserDefined)
        
    def SetSig3(self, isUserDefined):
        
        if( isUserDefined ):
            self.ui.rbIoSig3User.setChecked( True )
            self.ui.rbIoSig3ShutterStrobe.setChecked( False )
            self.ui.rbIoSig3In.setEnabled( True )
            self.ui.rbIoSig3Out.setEnabled( True )
        else:
            self.ui.rbIoSig3User.setChecked( False )
            self.ui.rbIoSig3ShutterStrobe.setChecked( True )
            self.ui.rbIoSig3In.setEnabled( False )
            self.ui.rbIoSig3Out.setEnabled( False )
            
    def OnSig4(self):
        isUserDefined = False
        if( self.ui.rbIoSig4User.isChecked() ):
            isUserDefined = True
        else:
            self.ui.rbIoSig4In.setChecked( True )
            
        self.SetSig4(isUserDefined)
        
    def SetSig4(self, isUserDefined):
        if( isUserDefined ):
            self.ui.rbIoSig4User.setChecked( True )
            self.ui.rbIoSig4ExternalShutter.setChecked( False )
            self.ui.rbIoSig4In.setEnabled( True )
            self.ui.rbIoSig4Out.setEnabled( True )
        else:
            self.ui.rbIoSig4User.setChecked( False )
            self.ui.rbIoSig4ExternalShutter.setChecked( True )
            self.ui.rbIoSig4In.setEnabled( False )
            self.ui.rbIoSig4Out.setEnabled( False )
            
    def OnSig5(self):
        isUserDefined = False
        if( self.ui.rbIoSig5User.isChecked() ):
            isUserDefined = True
        else:
            self.ui.rbIoSig5In.setChecked( True )
            
        self.SetSig5(isUserDefined)
        
    def SetSig5(self, isUserDefined):
        if( isUserDefined ):
            self.ui.rbIoSig5User.setChecked( True )
            self.ui.rbIoSig5ExtReadout.setChecked( False )
            self.ui.rbIoSig5In.setEnabled( True )
            self.ui.rbIoSig5Out.setEnabled( True )
        else:
            self.ui.rbIoSig5User.setChecked( False )
            self.ui.rbIoSig5ExtReadout.setChecked( True )
            self.ui.rbIoSig5In.setEnabled( False )
            self.ui.rbIoSig5Out.setEnabled( False )
            
    def OnSig6(self):
        isUserDefined = False
        if( self.ui.rbIoSig6User.isChecked() ):
            isUserDefined = True
        else:
            self.ui.rbIoSig6In.setChecked( True )
            
        self.SetSig6(isUserDefined)
        
    def SetSig6(self, isUserDefined):
        if( isUserDefined ):
            self.ui.rbIoSig6User.setChecked( True )
            self.ui.rbIoSig6TimerPulse.setChecked( False )
            self.ui.rbIoSig6In.setEnabled( True )
            self.ui.rbIoSig6Out.setEnabled( True )
        else:
            self.ui.rbIoSig6User.setChecked( False )
            self.ui.rbIoSig6TimerPulse.setChecked( True )
            self.ui.rbIoSig6In.setEnabled( False )
            self.ui.rbIoSig6Out.setEnabled( False )
            
    def OnReadIoPort(self):
        portAssignment = self.camRef.GetIoPortAssignment()
        
        #sig 1
        if( self.Sig1Mask & portAssignment ):
            self.SetSig1( False )
        else:
            self.SetSig1( True )
            
        #sig 2
        if( self.Sig2Mask & portAssignment ):
            self.SetSig2( False )
        else:
            self.SetSig2( True )
            
        #sig 3
        if( self.Sig3Mask & portAssignment ):
            self.SetSig3( False )
        else:
            self.SetSig3( True )
            
        #sig 4
        if( self.Sig4Mask & portAssignment ):
            self.SetSig4( False )
        else:
            self.SetSig4( True )
            
        #sig 5
        if( self.Sig5Mask & portAssignment ):
            self.SetSig5( False )
        else:
            self.SetSig5( True )
                
        #sig 6
        if( self.Sig6Mask & portAssignment ):
            self.SetSig6( False )
        else:
            self.SetSig6( True )
            
        #direction
        portDirection = self.camRef.GetIoPortDirection()
        
        #sig 1
        if( self.Sig1Mask & portDirection  ):
            self.ui.rbIoSig1Out.setChecked( True )
            self.ui.rbIoSig1In.setChecked( False )
        else:
            self.ui.rbIoSig1Out.setChecked( False )
            self.ui.rbIoSig1In.setChecked( True )
            
        #sig 2
        if( self.Sig2Mask & portDirection  ):
            self.ui.rbIoSig2Out.setChecked( True )
            self.ui.rbIoSig2In.setChecked( False )
        else:
            self.ui.rbIoSig2Out.setChecked( False )
            self.ui.rbIoSig2In.setChecked( True )
            
        #sig 3
        if( self.Sig3Mask & portDirection  ):
            self.ui.rbIoSig3Out.setChecked( True )
            self.ui.rbIoSig3In.setChecked( False )
        else:
            self.ui.rbIoSig3Out.setChecked( False )
            self.ui.rbIoSig3In.setChecked( True )
            
        #sig 4
        if( self.Sig4Mask & portDirection  ):
            self.ui.rbIoSig4Out.setChecked( True )
            self.ui.rbIoSig4In.setChecked( False )
        else:
            self.ui.rbIoSig4Out.setChecked( False )
            self.ui.rbIoSig4In.setChecked( True )
            
        #sig 5
        if( self.Sig5Mask & portDirection  ):
            self.ui.rbIoSig5Out.setChecked( True )
            self.ui.rbIoSig5In.setChecked( False )
        else:
            self.ui.rbIoSig5Out.setChecked( False )
            self.ui.rbIoSig5In.setChecked( True )
            
         #sig 6
        if( self.Sig6Mask & portDirection ):
            self.ui.rbIoSig6Out.setChecked( True )
            self.ui.rbIoSig6In.setChecked( False )
        else:
            self.ui.rbIoSig6Out.setChecked( False )
            self.ui.rbIoSig6In.setChecked( True )   
        
    def Update4Cam(self):
        #TODO - incomplete
        self.OnReadBackoffPoint()
        self.OnReadSetPoint()
        self.OnReadShutterCloseDelay()
        self.OnReadShutterStrobePeriod()
        self.OnReadShutterStrobePosition()
        self.UpdateFanMode()
        self.UpdateLedMode()
        self.UpdateLedState( self.camRef.GetLedAState(),
                             self.ui.cbCtrlLedA )
        self.UpdateLedState( self.camRef.GetLedBState(),
                             self.ui.cbCtrlLedB )
        
        if self.camRef.IsCoolerOn:
            self.ui.btnCtrCoolerToggle.setChecked( True )
            self.ui.btnCtrCoolerToggle.setText( unicode("Off") )
        else:
            self.ui.btnCtrCoolerToggle.setChecked( False )
            self.ui.btnCtrCoolerToggle.setText( unicode("On") )
            
        if self.camRef.IsShutterAmpCtrlOn():
            self.ui.cbCtrlShutterAmpCtrl.setChecked( True )
        else:
            self.ui.cbCtrlShutterAmpCtrl.setChecked( False )
            
        #set all the IO 
        self.camRef.SetIoPortAssignment(0) 
        self.camRef.SetIoPortDirection(0)
        
        self.OnReadIoPort()
        
    def OnShutterAmpCtrl(self):
        if( self.ui.cbCtrlShutterAmpCtrl.isChecked() ):
            self.camRef.SetShutterAmpCtrl( True )
        else:
            self.camRef.SetShutterAmpCtrl( False )
            
    def OnTriggerExtShutter(self):
        if( self.ui.cbCtrlTrigExternalShutter.isChecked() ):
            self.camRef.SetExternalTrigger( True, 
                                            apg.TriggerMode_ExternalShutter,
                                            apg.TriggerType_Each )
        else:
            self.camRef.SetExternalTrigger( False, 
                                            apg.TriggerMode_ExternalShutter,
                                            apg.TriggerType_Each )
            
    def OnTriggerExtReadoutIo(self):
        if( self.ui.cbCtrlTrigExtIoReadout.isChecked() ):
            self.camRef.SetExternalTrigger( True, 
                                            apg.TriggerMode_ExternalReadoutIo,
                                            apg.TriggerType_Each )
        else:
            self.camRef.SetExternalTrigger( False, 
                                            apg.TriggerMode_ExternalReadoutIo,
                                            apg.TriggerType_Each )
            
    def OnTriggerNormEach(self):
        if( self.ui.cbCtrlTrigNormEach.isChecked() ):
           self.camRef.SetExternalTrigger( True, 
                                            apg.TriggerMode_Normal,
                                            apg.TriggerType_Each )
        else:
            self.camRef.SetExternalTrigger( False, 
                                            apg.TriggerMode_Normal,
                                            apg.TriggerType_Each )
            
    def OnTriggerNormGroup(self):
        if( self.ui. cbCtrlTrigNormGroup.isChecked() ):
           self.camRef.SetExternalTrigger( True, 
                                            apg.TriggerMode_Normal,
                                            apg.TriggerType_Group )
        else:
            self.camRef.SetExternalTrigger( False, 
                                            apg.TriggerMode_Normal,
                                            apg.TriggerType_Group )
            
    def OnTriggerTdiKinEach(self):
        if( self.ui.cbCtrlTrigTdiKinEach.isChecked() ):
           self.camRef.SetExternalTrigger( True, 
                                            apg.TriggerMode_TdiKinetics,
                                            apg.TriggerType_Each )
        else:
            self.camRef.SetExternalTrigger( False, 
                                            apg.TriggerMode_Normal,
                                            apg.TriggerType_Each )
            
    def OnTriggerTdiKinGroup(self):
        if( self.ui.cbCtrlTrigTdiKinGroup.isChecked() ):
           self.camRef.SetExternalTrigger( True, 
                                            apg.TriggerMode_TdiKinetics,
                                            apg.TriggerType_Group )
        else:
            self.camRef.SetExternalTrigger( False, 
                                            apg.TriggerMode_TdiKinetics,
                                            apg.TriggerType_Group )
        
        
        
            

        