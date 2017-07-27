'''
Copyright(c) 2010 Apogee Instruments, Inc.
Created on Feb 11, 2010

@author: ltraynor
'''
import sys
import os
import traceback
import string
import json
import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import ApexUi
import ApexSetupTab
import ApexPrgmTab
import ApexTestTab
import ApexCtrlTab
import ApexAcqTab
import ApexInfoTab
import ApexFilterWheelTab
import ApexStatusBar
import pyApg as apg

#-----------------------------
#GLOBAL EXCEPTION HANDLER
def handle_exception(exc_type, exc_value, exc_traceback):

    filename, line, dummy, dummy = traceback.extract_tb(exc_traceback).pop()
    filename = os.path.basename(filename)
    type = str(exc_type).split(".")[-1]
    type = type.rstrip('\'>')
    error = "%s: %s" % (type, exc_value)
  
    errMsg = "A %s exception has occurred on line %d, file %s" % (error, line, filename)
    qtGui.QMessageBox.critical(None, "ERROR", errMsg)
    sys.stderr.write(errMsg)
  
sys.excepthook=handle_exception

#-----------------------------
#WRITE OUTPUT TO WINDOW
class RedirectText:
    def __init__(self,cutieTextEdit):
        self.out=cutieTextEdit

    def write(self,text):
        #add newline if there isn't one
        if( -1 == text.find("\n") ):
            text += "\n"
            
        #make sure we at the end of the document
        #enum QTextCursor::End = 11
        self.out.moveCursor(11)
        self.out.insertPlainText( unicode(text) )
        
class ApexDlg(qtGui.QDialog):
    '''
    Main application for the Apex Camera tool
    '''
    def __init__(self,parent=None):
        '''
        Constructor
        '''
        super(ApexDlg,self).__init__(parent)
  
        self.setWindowIcon( qtGui.QIcon("apogeeIcon.ico") )
        
        # Set up the user interface from Designer.
        self.ui = ApexUi.Ui_dlgMain()
        self.ui.setupUi(self)        
        
        
        #redirect stdout and stderr to output window
        redir=RedirectText(self.ui.editSetupOutput)
        sys.stdout=redir
        sys.stderr=redir
        
        #create the supporting ui classes
        
        #setup tab
        self.SetupObj = ApexSetupTab.ApexSetupTab(self.ui)
        
        self.connect(self.ui.btnFindCam, 
                     qtCore.SIGNAL("clicked()"), 
                     self.SetupObj.OnFindBtn)
        
        self.connect(self.ui.btnSetupScriptBrowse, 
                     qtCore.SIGNAL("clicked()"), 
                     self.SetupObj.OnScriptBrowse)
        
        self.connect(self.ui.btnIniBrowse, 
                     qtCore.SIGNAL("clicked()"), 
                     self.SetupObj.OnMatrixBrowse)
        
        self.connect(self.ui.btnSetupSaveOutput, 
                     qtCore.SIGNAL("clicked()"), 
                     self.SetupObj.OnSaveOutput)
        
        self.connect(self.ui.btnSetupClearOutput, 
                     qtCore.SIGNAL("clicked()"), 
                     self.SetupObj.OnClearOutput)
                     
        #connect this signal to this class because
        #this is where we are creating and owning the
        #camera object
        self.connect(self.ui.btnCamConnect,
                     qtCore.SIGNAL("clicked()"),
                     self.OnCamConnect)
        
        self.connect(self.ui.btnSetupScriptRun, 
                     qtCore.SIGNAL("clicked()"), 
                     self.OnRunScript)
        
        self.connect(self.ui.btnIoCfgMatrixLoad, 
                     qtCore.SIGNAL("clicked()"), 
                     self.OnLoadCfgData)
        
        self.connect(self.ui.btnSetupReInit, 
                     qtCore.SIGNAL("clicked()"), 
                     self.OnReInit)
        
        #programming
        self.PrgmObj = ApexPrgmTab.ApexPrgmTab(self.ui)
        
        self.connect( self.ui.btnPrgmFirmwareDownload,
                       qtCore.SIGNAL("clicked()"),
                     self.PrgmObj.OnProgramCamera)
        
        self.connect(self.ui.btnPrgmSerialRead,
                    qtCore.SIGNAL("clicked()"),
                     self.PrgmObj.OnSerialRead)
        
        self.connect(self.ui.btnPrgmSerialWrite,
                    qtCore.SIGNAL("clicked()"),
                     self.PrgmObj.OnSerialWrite)
        
        self.connect(self.ui.btnPrgmMacRead,
                    qtCore.SIGNAL("clicked()"),
                     self.PrgmObj.OnMacRead)
        
        #link up the aspen, ascent, and alta browse buttons
        for k,v in self.PrgmObj.AspenDict.iteritems():
            self.connect( k, 
                           qtCore.SIGNAL("clicked()"), 
                           self.PrgmObj.OnBrowseAspen )
             
        for k,v in self.PrgmObj.AscentDict.iteritems():
            self.connect( k, 
                           qtCore.SIGNAL("clicked()"), 
                           self.PrgmObj.OnBrowseAscent )
             
        for k,v in self.PrgmObj.AltaDict.iteritems():
            self.connect( k, 
                           qtCore.SIGNAL("clicked()"), 
                           self.PrgmObj.OnBrowseAlta )
        
        #control
        self.CtrlObj = ApexCtrlTab.ApexCtrlTab(self.ui)
                
        self.connect(self.ui.btnCtrCoolerToggle,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnToggleCooler)
        
        self.connect(self.ui.btnCtrlCoolerSetPtWrite,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnWriteSetPoint)
        
        self.connect(self.ui.btnCtrlCoolerSetPtRead,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnReadSetPoint)
        
        self.connect(self.ui.btnCtrlCoolerBackoffWrite,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnWriteBackoffPoint)
        
        self.connect(self.ui.btnCtrlCoolerBackoffRead,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnReadBackoffPoint)
        
        self.connect(self.ui.cbCtrlFan,
                    qtCore.SIGNAL("currentIndexChanged(int)"),
                     self.CtrlObj.OnFanMode)
        
        self.connect(self.ui.rbCtrlShutNorm,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnShutterOperation)
        
        self.connect(self.ui.rbCtrlShutOpen,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnShutterOperation)
        
        self.connect(self.ui.rbCtrlShutClose,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnShutterOperation)
        
        self.connect(self.ui.cbCtrlLedMode,
                    qtCore.SIGNAL("currentIndexChanged(int)"),
                     self.CtrlObj.OnLedMode)
        
        self.connect(self.ui.cbCtrlLedA,
                    qtCore.SIGNAL("currentIndexChanged(int)"),
                     self.CtrlObj.OnLedAState)
        
        self.connect(self.ui.cbCtrlLedB,
                    qtCore.SIGNAL("currentIndexChanged(int)"),
                     self.CtrlObj.OnLedBState)
                      
        self.connect(self.ui.cbCtrlShutterAmpCtrl,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnShutterAmpCtrl)  
        
        self.connect(self.ui.btnCtrlShutDelayRead,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnReadShutterCloseDelay)  
        
        self.connect(self.ui.btnCtrlShutDelayWrite,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnWriteShutterCloseDelay)  
        
        self.connect(self.ui.btnCtrlShutStrobePeriodRead,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnReadShutterStrobePeriod)  
        
        self.connect(self.ui.btnCtrlShutStrobePeriodWrite,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnWriteShutterStrobePeriod)
        
        self.connect(self.ui.btnCtrlShtStrobePosRead,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnReadShutterStrobePosition)  
        
        self.connect(self.ui.btnCtrlShtStrobePosWrite,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnWriteShutterStrobePosition)
        
        self.connect(self.ui.checkCtrlFlushCmdDisabled,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnDisableFlushingCmds)
        
        self.connect(self.ui.checkCtrlPostExposeFlushDisabled,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnPostExposeFlushDisabled)
        
        self.connect(self.ui.btnCtrlReadIoPort,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnReadIoPort)
        
        self.connect(self.ui.btnCtrlWriteIoPort,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnWriteIoPort)
        
        self.connect(self.ui.rbIoSig1User,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnSig1)
        
        self.connect(self.ui.rbIoSig1Trig,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnSig1)
        
        self.connect(self.ui.rbIoSig2User,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnSig2)
        
        self.connect(self.ui.rbIoSig2Shutter,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnSig2)
        
        self.connect(self.ui.rbIoSig3User,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnSig3)
        
        self.connect(self.ui.rbIoSig3ShutterStrobe,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnSig3)
        
        self.connect(self.ui.rbIoSig4User,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnSig4)
        
        self.connect(self.ui.rbIoSig4ExternalShutter,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnSig4)
        
        self.connect(self.ui.rbIoSig5User,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnSig5)
        
        self.connect(self.ui.rbIoSig5ExtReadout,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnSig5)
        
        self.connect(self.ui.rbIoSig6User,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnSig6)
        
        self.connect(self.ui.rbIoSig6TimerPulse,
                    qtCore.SIGNAL("clicked()"),
                     self.CtrlObj.OnSig6)
        
        self.connect(self.ui.cbCtrlTrigExternalShutter,
                      qtCore.SIGNAL("clicked()"),
                      self.CtrlObj.OnTriggerExtShutter)
        
        self.connect(self.ui.cbCtrlTrigExtIoReadout,
                      qtCore.SIGNAL("clicked()"),
                      self.CtrlObj.OnTriggerExtReadoutIo)
        
        self.connect(self.ui.cbCtrlTrigNormEach,
                      qtCore.SIGNAL("clicked()"),
                      self.CtrlObj.OnTriggerNormEach)
        
        self.connect(self.ui.cbCtrlTrigNormGroup,
                      qtCore.SIGNAL("clicked()"),
                      self.CtrlObj.OnTriggerNormGroup)
        
        self.connect(self.ui.cbCtrlTrigTdiKinEach,
                      qtCore.SIGNAL("clicked()"),
                      self.CtrlObj.OnTriggerTdiKinEach)
        
        self.connect(self.ui.cbCtrlTrigTdiKinGroup,
                      qtCore.SIGNAL("clicked()"),
                      self.CtrlObj.OnTriggerTdiKinGroup)
            
        #acquisition
        self.AcqObj = ApexAcqTab.ApexAcqTab( self.ui )
        
        #connect the radio buttons to the OnModeRadio fx
        for k, v in self.AcqObj.ModeDict.iteritems():
            rb = v[1]
            self.connect( rb, 
                          qtCore.SIGNAL("clicked()"),
                          self.AcqObj.OnModeRadio)
            
        self.connect(self.ui.rbAcqAdcCcdNormal, 
                          qtCore.SIGNAL("clicked()"),
                          self.AcqObj.OnCcdAcdRadio)
        
        self.connect(self.ui.rbAcqAdcCcdFast, 
                          qtCore.SIGNAL("clicked()"),
                          self.AcqObj.OnCcdAcdRadio)
        
        self.connect(self.ui.rbAcqAdcCcdVideo, 
                          qtCore.SIGNAL("clicked()"),
                          self.AcqObj.OnCcdAcdRadio)
                
        self.connect(self.ui.btnAcqAdcRead, 
                          qtCore.SIGNAL("clicked()"),
                          self.AcqObj.OnAdcSettingsRead)
        
        self.connect(self.ui.btnAcqAdcWrite, 
                          qtCore.SIGNAL("clicked()"),
                          self.AcqObj.OnAdcSettingsWrite)
                
        self.connect(self.ui.btnAcqExposeToggle, 
                          qtCore.SIGNAL("clicked()"),
                          self.AcqObj.OnToggleExposure)    
          
        self.connect(self.ui.checkAcqNormBinLink, 
                          qtCore.SIGNAL("clicked()"),
                          self.AcqObj.OnLinkChecked )
  
        self.connect(self.ui.spinAcqNormBinRow, 
                          qtCore.SIGNAL("valueChanged(int)"),
                          self.AcqObj.OnRowBin )
        
        self.connect(self.ui.spinAcqNormBinCol, 
                          qtCore.SIGNAL("valueChanged(int)"),
                          self.AcqObj.OnColBin )
        
        self.connect(self.ui.btnAcqOutDirBrowse, 
                          qtCore.SIGNAL("clicked()"),
                          self.AcqObj.OnOutDirBrowse)
        
        self.connect(self.ui.btnAcqSeqDelayRead,
                      qtCore.SIGNAL("clicked()"),
                      self.AcqObj.OnReadSeqDelay)
        
        self.connect(self.ui.btnAcqSeqDelayWrite,
                      qtCore.SIGNAL("clicked()"),
                      self.AcqObj.OnWriteSeqDelay)
        
        self.connect(self.ui.cbAcqNormBulkOnOff,
                      qtCore.SIGNAL("clicked()"),
                      self.AcqObj.OnToggleBulkSeq)
        
        self.connect(self.ui.cbAcqNormBulkVarDelay,
                      qtCore.SIGNAL("clicked()"),
                      self.AcqObj.OnToggleBulkSeqVariableDelay)
                
        self.connect(self.ui.rbAcqViewImg,
                      qtCore.SIGNAL("clicked()"),
                      self.AcqObj.OnSaveOrViewer)
        
        self.connect(self.ui.rbAcqSaveImg2File,
                      qtCore.SIGNAL("clicked()"),
                      self.AcqObj.OnSaveOrViewer)
        
        #testing
        self.TestObj = ApexTestTab.ApexTestTab( self.ui )
        
        self.connect(self.ui.rbIoTestFifo, 
                     qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnTestType)
        
        self.connect(self.ui.rbIoTestAds, 
                     qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnTestType)
        
        self.connect(self.ui.btnIoTestBrowse, 
                     qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnBrowse)
        
        self.connect(self.ui.rbIoTestFixed, 
                     qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnTestSize)
        
        self.connect(self.ui.rbIoTestRandom, 
                     qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnTestSize)
    
        self.connect(self.ui.btnIoTestToggle,
                     qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnTest)
        
        self.connect(self.ui.btnTestAscentFwConnect,
                     qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnAscentFwConnect)
         
        self.connect(self.ui.spinTestAscentFwPos, 
                          qtCore.SIGNAL("valueChanged(int)"),
                          self.TestObj.OnAscentFwPos )
        
        self.connect(self.ui.editTestRegIoRegWrite,
                    qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnRegWrite)
        
        self.connect(self.ui.editTestRegIoRegRead,
                    qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnRegRead)
        
        self.connect(self.ui.btnTestSpeedStartStop,
                    qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnSpeedTest)
        
        self.connect( self.ui.btnTestSerialConnect,
                         qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnConnectSerialPort)
        
        self.connect( self.ui.btnTestSerialWrite,
                         qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnSerialWrite)
        
        self.connect( self.ui.btnTestSerialRead,
                         qtCore.SIGNAL("clicked()"),
                     self.TestObj.OnSerialRead)
        
        #info tab
        self.InfoTabObj = ApexInfoTab.ApexInfoTab(self.ui)
        
        self.connect(self.ui.btnInfoRead, 
                           qtCore.SIGNAL("clicked()"),
                           self.InfoTabObj.OnRead)
        
        self.connect(self.ui.btnInfoWrite, 
                           qtCore.SIGNAL("clicked()"),
                           self.InfoTabObj.OnWrite)
        
        # filter wheel tab
        self.FilterWheelTabObj = ApexFilterWheelTab.ApexFilterWheelTab(self.ui)
        
        self.connect( self.ui.btnFwConnect, 
                           qtCore.SIGNAL( "clicked()" ),
                           self.FilterWheelTabObj.OnConnect )
        
        self.connect( self.ui.btnFwBrowse, 
                           qtCore.SIGNAL( "clicked()" ),
                           self.FilterWheelTabObj.OnBrowse )
        
        self.connect( self.ui.btnFwProgram, 
                           qtCore.SIGNAL( "clicked()" ),
                           self.FilterWheelTabObj.OnProgram )
        
        #status bar
        self.StatusBarObj = ApexStatusBar.ApexStatusBar( self.ui )
                
        #init variables
        self.theCamera = None

        #create onidle timer
        self.statusCount = 0
        self.statusCountMax = 5
        self.idleTimer = qtCore.QTimer()
        self.timerInterval = 1000
        self.idleTimer.setInterval( self.timerInterval )
        self.connect( self.idleTimer, 
                      qtCore.SIGNAL( "timeout()" ),
                      self.OnIdle )
        
        #load up the ui defaults
        self.ReadUpdateDefaults()
        self.ToggleOnCam( False )
        
    def ToggleOnCam(self, onOff):
        self.ui.btnSetupReInit.setEnabled( onOff )
        self.ui.tabPrgm.setEnabled( onOff )
        self.ui.tabControls.setEnabled( onOff )
        self.ui.tabAcq.setEnabled( onOff )
        self.ui.tabTest.setEnabled( onOff )
        self.ui.tabInfo.setEnabled( onOff )
        self.ui.gbIoScripts.setEnabled( onOff )
        self.ui.gbIoIni.setEnabled( onOff )
        self.ui.gbSetupCamExtra.setEnabled( onOff )
        
        opposite = False
        if( opposite == onOff ):
            opposite = True
            
        self.ui.btnFindCam.setEnabled( opposite  )
        self.ui.cbCamType.setEnabled( opposite  )
        self.ui.rbCamUsb.setEnabled( opposite )
        self.ui.rbCamEthernet.setEnabled( opposite)
        self.ui.editCamAddress.setEnabled( opposite )
                   
    def ReadUpdateDefaults(self):
                
        dataDict = self.getDefaultsFromFile()
        
        if( None == dataDict ):
            self.write2OutputWin( "no default file" )
            return
                
        self.ui.editCamAddress.setText( dataDict["deviceAddr"] )
        self.ui.cbCamType.setCurrentIndex( self.ui.cbCamType.findText( dataDict["camType" ] ) )
        self.ui.cbCamCfg.setCurrentIndex( self.ui.cbCamCfg.findText(dataDict["camId" ] ) )
        self.ui.spinIoTestImgRows.setValue( dataDict["TestRows"] )
        self.ui.spinIoTestImgCols.setValue( dataDict["TestCols"] )
        self.ui.spinIoTestNumRuns.setValue( dataDict["TestNumRuns"] )
        self.ui.editIoTestOutDir.setText( dataDict["TestOutDir" ] )  
        self.ui.editAcqOutDir.setText( dataDict["acqOutDir"] )
        self.SetupObj.lastDir.setPath( dataDict["setupDir"] )
        self.PrgmObj.lastDir.setPath( dataDict["programDir"] )  
        self.FilterWheelTabObj.lastDir.setPath( dataDict["fwFirmwareDir"] )  
        
        #get the new set path in the edit box
        self.FilterWheelTabObj.Refresh()
        
        if( dataDict["deviceInterface"] == "usb" ):
            self.ui.rbCamUsb.setChecked(True)
        else:
            self.ui.rbCamEthernet.setChecked(True)
                    
    def closeEvent(self, event):
        #save some default values to file for when the
        #app is restarted
        interfaceType = "usb"        
        if( self.ui.rbCamEthernet.isChecked() ):
                interfaceType = "ethernet"
                  
        data = {  "deviceInterface" : interfaceType,
        "deviceAddr" : unicode( self.ui.editCamAddress.text() ),
         "camType" : unicode( self.ui.cbCamType.currentText() ),
         "camId": unicode( self.ui.cbCamCfg.currentText() ),
         "TestRows" : self.ui.spinIoTestImgRows.value(),
         "TestCols" : self.ui.spinIoTestImgCols.value(),
         "TestNumRuns" : self.ui.spinIoTestNumRuns.value(),
         "TestOutDir" : unicode( self.ui.editIoTestOutDir.text() ),
         "acqOutDir" : unicode( self.ui.editAcqOutDir.text() ),
         "setupDir" : unicode( self.SetupObj.lastDir.path() ),
         "programDir" : unicode( self.PrgmObj.lastDir.path() ),
         "fwFirmwareDir": unicode(  self.FilterWheelTabObj.lastDir.path() )}
                
        defaultFile = open("defaults.txt", "w")
        json.dump(data, defaultFile, indent=2)
        defaultFile.close()
        
        #disconnecting a camera if needed
        if( self.ui.btnCamConnect.isChecked() ):
            self.DisconnectCamera()
             
        #close the viewer if it is open
        self.AcqObj.closeViewer()
       
    def getDefaultsFromFile(self):
        
        if( False == os.path.exists("defaults.txt") ) :
            return None
        
        defaultFile = open("defaults.txt", "r")
         
        if( None == defaultFile ):
            return None
               
        dataDict = json.load(defaultFile)
        defaultFile.close()
        
        return dataDict
    
    def write2OutputWin(self,text):
        if( -1 == text.find("\n") ):
            text += "\n"
        self.ui.editSetupOutput.moveCursor(11)
        self.ui.editSetupOutput.insertPlainText( unicode(text) )
        qtGui.QApplication.instance().processEvents()
        
    def OnCamConnect(self):
        if( self.ui.btnCamConnect.isChecked() ):
            camInfo = self.SetupObj.GetCamInfo()
            
            if( apg.ALTAU == camInfo[2] or apg.ALTAE == camInfo[2] ):
                self.theCamera = apg.TestCamAlta( camInfo[0], camInfo[1] )
            elif( apg.ASCENT == camInfo[2] ):
                self.theCamera = apg.TestCamAscent( camInfo[0], camInfo[1] )
            elif( apg.ALTAF == camInfo[2] ):
                self.theCamera = apg.TestCamAltaF( camInfo[0], camInfo[1] )
            elif( apg.ASPEN == camInfo[2] ):
                self.theCamera = apg.TestCamAspen( camInfo[0], camInfo[1] )
            elif( apg.QUAD == camInfo[2] ):
                self.theCamera = apg.TestCamQuad( camInfo[0], camInfo[1] )
            elif( apg.HIC == camInfo[2] ):
                self.theCamera = apg.TestCamHiC( camInfo[0], camInfo[1] )
                self.theCamera.SetPixelReorder( True )
            else:
                raise RuntimeError( "Cannot create camera object invalid type" )
            
            self.write2OutputWin( "Successfully created camera platform=%s, interface=%s, address=%s" % 
                                  ( self.SetupObj.GetCamTypeStr(), camInfo[0], camInfo[1]  ) )
            #pass a reference to this object to the tab classes
            #so they can use it
            self.SetupObj.SetCamera( self.theCamera )
            self.TestObj.SetCamera( self.theCamera )
            
            self.PrgmObj.SetCamera( self.theCamera )
                
            self.CtrlObj.SetCamera( self.theCamera )
            self.AcqObj.SetCamera( self.theCamera )
                        
            self.InfoTabObj.SetCamera( self.theCamera )
            
            self.StatusBarObj.SetCamera( self.theCamera )
            #enable and disable some buttons
            self.ToggleOnCam( True )
            
            #if an id is selected configure and init cam and ui
            self.OnReInit()
                    
            self.ui.btnCamConnect.setText( unicode("Disconnect") )
        else:
            self.DisconnectCamera()
            
            
    def DisconnectCamera(self):
        #delete the camera class to 
        #close all of the handles to it
        self.idleTimer.stop()

        self.SetupObj.RemoveCamera()
        self.TestObj.RemoveCamera()
        self.PrgmObj.RemoveCamera()
        self.CtrlObj.RemoveCamera()
        self.AcqObj.RemoveCamera()
        self.InfoTabObj.RemoveCamera()
        self.StatusBarObj.RemoveCamera()
        
        del self.theCamera
        self.theCamera = None
        self.ui.btnCamConnect.setText( unicode("Connect") )
        self.write2OutputWin( "Disconnected camera" )
        self.ui.setupInfoTextWnd.clear()
        
        #enable and disable some buttons
        self.ToggleOnCam( False )

    def OnReInit(self):
        curId = self.ui.cbCamCfg.currentText()
        if( 0 != curId.compare( qtCore.QString(unicode("None") ) ) ):
                        
                self.theCamera.CfgCamFromId( int(curId) )
                                             
                self.TestObj.Update4Cam()
                self.CtrlObj.Update4Cam()
                self.AcqObj.Update4Cam()
                self.write2OutputWin( "Successfully initialized camera with id %s" % ( curId ) )
                self.SetupObj.OnGetInfo()
                
                # bulk download on by default, turn off
                self.theCamera.SetBulkDownload( False )
            
                #if the timer is off start it
                if( False == self.idleTimer.isActive() ):
                    self.idleTimer.start( self.timerInterval )
        
    def OnRunScript(self):
        try:
            if( self.ui.btnSetupScriptRun.isChecked() ):
                self.idleTimer.stop()
                self.ui.btnSetupScriptRun.setText( "Running" )
                self.ui.btnSetupScriptRun.setEnabled( False )
                scriptName = unicode( self.ui.cbSetupScript.currentText() )
                # if directory script is in is not already in the python
                # search path add it, so that the file can import items
                # from that directory if neeed
                scriptPath = os.path.dirname( scriptName )
                pythonPath = sys.path
                
                found = False
                for i in pythonPath:
                    if( scriptPath == i ):
                        found = True
                        break
                if( False == found ):
                    sys.path.append( scriptPath )
                    
                #run the script
                execfile( scriptName )
        except RuntimeError:
            self.write2OutputWin( traceback.format_exc() )
        finally:
            self.ui.btnSetupScriptRun.setText( "Run" )
            self.ui.btnSetupScriptRun.setEnabled( True )
            self.ui.btnSetupScriptRun.setChecked( False )
            # if the camera has been configured then restart the
            # idle timer
            curId = self.ui.cbCamCfg.currentText()
            if( 0 != curId.compare( qtCore.QString(unicode("None") ) ) ):
                self.idleTimer.start( self.timerInterval )
        
    def OnLoadCfgData(self):
        fullFilename = unicode( self.ui.cbIniFile.currentText() ).encode()
        (dir, file) = os.path.split( fullFilename )
        self.theCamera.CfgCamFromFile( dir, file )
        
        self.write2OutputWin( "Camera successfully configured with file %s" % fullFilename )
        
        self.AcqObj.Update4Cam()
        
        #if the timer is off start it
        if( False == self.idleTimer.isActive() ):
            self.idleTimer.start( self.timerInterval )
        
    def OnIdle(self):
        if( None != self.theCamera ):
            #check if there is an image to download
            self.AcqObj.Check4Image()
            
            if( self.statusCount >= self.statusCountMax ):
                self.StatusBarObj.UpdateStatus()
                self.statusCount = 0
            else:
                self.statusCount += 1
                      
        
#-----------------------------
# MAIN     
if __name__ == '__main__':
    app = qtGui.QApplication(sys.argv)
    ApexApp = ApexDlg()
    ApexApp.show()
    app.exec_()