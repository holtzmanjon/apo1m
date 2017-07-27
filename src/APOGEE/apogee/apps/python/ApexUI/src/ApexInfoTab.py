'''
Created on Mar 22, 2011

@author: ltraynor
'''

import TabBase
import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import ApexUi
import pyApg as apg

class ApexInfoTab(TabBase.TabBase):
    '''
    Supports the programming ui tab widget
    '''


    def __init__(self,theUi,parent=None):
        '''
        Constructor
        '''
        super(ApexInfoTab,self).__init__(theUi,parent)
        
    def SetCamera(self,cam):
        self.camRef = cam
        
    def OnRead(self):
        info = self.camRef.GetCamInfo()
        
        self.ui.editInfoFactorySn.setText( unicode( info.FactorySn ) )
        self.ui.editInfoCustomerSn.setText( unicode( info.CustomerSn ) )
        self.ui.editInfoCamId.setText( unicode( info.Id ) )
        self.ui.editInfoPlatform.setText( unicode( info.Platform ) )
        self.ui.editInfoPartNum.setText( unicode( info.PartNum ) )
        self.ui.editInfoCcd.setText( unicode( info.Ccd ) )
        self.ui.editInfoCcdSn.setText( unicode( info.CcdSn ) )
        self.ui.editInfoCcdGrade.setText( unicode( info.CcdGrade ) )
        self.ui.editInfoProcBoardRev.setText( unicode( info.ProcBoardRev ) )
        self.ui.editInfoDriveBoardRev.setText( unicode( info.DriveBoardRev ) )
        self.ui.editInfoShutter.setText( unicode( info.Shutter ) )
        self.ui.editInfoWindowType.setText( unicode( info.WindowType ) )
        self.ui.editInfoMechCfg.setText( unicode( info.MechCfg ) )
        self.ui.editInfoMechRev.setText( unicode( info.MechRev ) )
        self.ui.editInfoCoolingType.setText( unicode( info.CoolingType ) )
        self.ui.editInfoFinishFront.setText( unicode( info.FinishFront ) )
        self.ui.editInfoFinishBack.setText( unicode( info.FinishBack ) )
        self.ui.editInfoMpiRev.setText( unicode( info.MpiRev ) )
        self.ui.editInfoTestDate.setText( unicode( info.TestDate ) )
        self.ui.editInfoTestedBy.setText( unicode( info.TestedBy ) )
        self.ui.editInfoTestDllRev.setText( unicode( info.TestedDllRev ) )
        self.ui.editInfoTestFwRev.setText( unicode( info.TestedFwRev ) )
        self.ui.editInfoGain.setText( unicode( info.Gain ) )
        self.ui.editInfoNoise.setText( unicode( info.Noise ) )
        self.ui.editInfoBias.setText( unicode( info.Bias ) )
        self.ui.editInfoTestTemp.setText( unicode( info.TestTemp ) )
        self.ui.editInfoDarkCount.setText( unicode( info.DarkCount ) )
        self.ui.editInfoDarkDuration.setText( unicode( info.DarkDuration ) )
        self.ui.editInfoDarkTemp.setText( unicode( info.DarkTemp ) )
        self.ui.editInfoCoolingDelta.setText( unicode( info.CoolingDelta ) )
        self.ui.editInfoAd0Offset.setText( unicode( info.Ad1Offset ) )
        self.ui.editInfoAd0Gain.setText( unicode( info.Ad1Gain ) )
        self.ui.editInfoAd1Offset.setText( unicode( info.Ad2Offset ) )
        self.ui.editInfoAd1Gain.setText( unicode( info.Ad2Gain ) )
        self.ui.editInfoRma1.setText( unicode( info.Rma1 ) )
        self.ui.editInfoRma2.setText( unicode( info.Rma2 ) )
        self.ui.editInfoComment1.setText( unicode( info.Comment1 ) )
        self.ui.editInfoComment2.setText( unicode( info.Comment2 ) )
        self.ui.editInfoComment3.setText( unicode( info.Comment3 ) )
        
    def OnWrite(self):
       out = apg.StrDb()
       out.FactorySn = unicode( self.ui.editInfoFactorySn.text() ).encode()
       out.CustomerSn = unicode( self.ui.editInfoCustomerSn.text() ).encode()
       out.Id = unicode( self.ui.editInfoCamId.text() ).encode()
       out.Platform = unicode( self.ui.editInfoPlatform.text() ).encode()
       out.PartNum = unicode( self.ui.editInfoPartNum.text() ).encode()
       out.Ccd = unicode( self.ui.editInfoCcd.text() ).encode()
       out.CcdSn = unicode( self.ui.editInfoCcdSn.text() ).encode()
       out.CcdGrade = unicode( self.ui.editInfoCcdGrade.text() ).encode()
       out.ProcBoardRev = unicode( self.ui.editInfoProcBoardRev.text() ).encode()
       out.DriveBoardRev = unicode( self.ui.editInfoDriveBoardRev.text() ).encode()
       out.Shutter = unicode( self.ui.editInfoShutter.text() ).encode()
       out.WindowType = unicode( self.ui.editInfoWindowType.text() ).encode()
       out.MechCfg = unicode( self.ui.editInfoMechCfg.text() ).encode()
       out.MechRev = unicode( self.ui.editInfoMechRev.text() ).encode()
       out.CoolingType = unicode( self.ui.editInfoCoolingType.text() ).encode()
       out.FinishFront = unicode( self.ui.editInfoFinishFront.text() ).encode()
       out.FinishBack = unicode( self.ui.editInfoFinishBack.text() ).encode()
       out.MpiRev = unicode( self.ui.editInfoMpiRev.text() ).encode()
       out.TestDate = unicode( self.ui.editInfoTestDate.text() ).encode()
       out.TestedBy = unicode( self.ui.editInfoTestedBy.text() ).encode()
       out.TestedDllRev = unicode( self.ui.editInfoTestDllRev.text() ).encode()
       out.TestedFwRev = unicode( self.ui.editInfoTestFwRev.text() ).encode()
       out.Gain = unicode( self.ui.editInfoGain.text() ).encode()
       out.Noise = unicode( self.ui.editInfoNoise.text() ).encode()
       out.Bias = unicode( self.ui.editInfoBias.text() ).encode()
       out.TestTemp = unicode( self.ui.editInfoTestTemp.text() ).encode()
       out.DarkCount = unicode( self.ui.editInfoDarkCount.text() ).encode()
       out.DarkDuration = unicode( self.ui.editInfoDarkDuration.text() ).encode()
       out.DarkTemp = unicode( self.ui.editInfoDarkTemp.text() ).encode()
       out.CoolingDelta = unicode( self.ui.editInfoCoolingDelta.text() ).encode()
       out.Ad1Offset = unicode( self.ui.editInfoAd0Offset.text() ).encode()
       out.Ad1Gain = unicode( self.ui.editInfoAd0Gain.text() ).encode()
       out.Ad2Offset = unicode( self.ui.editInfoAd1Offset.text() ).encode()
       out.Ad2Gain = unicode( self.ui.editInfoAd1Gain.text() ).encode()
       out.Rma1 = unicode( self.ui.editInfoRma1.text() ).encode()
       out.Rma2 = unicode( self.ui.editInfoRma2.text() ).encode()
       out.Comment1 = unicode( self.ui.editInfoComment1.text() ).encode()
       out.Comment2 = unicode( self.ui.editInfoComment2.text() ).encode()
       out.Comment3 = unicode( self.ui.editInfoComment3.text() ).encode()
       
       self.camRef.SetCamInfo( out )
       self.ClearEntries()
       
    def ClearEntries(self):
        self.ui.editInfoFactorySn.clear()
        self.ui.editInfoCustomerSn.clear()
        self.ui.editInfoCamId.clear()
        self.ui.editInfoPlatform.clear()
        self.ui.editInfoPartNum.clear()
        self.ui.editInfoCcd.clear()
        self.ui.editInfoCcdSn.clear()
        self.ui.editInfoCcdGrade.clear()
        self.ui.editInfoProcBoardRev.clear()
        self.ui.editInfoDriveBoardRev.clear()
        self.ui.editInfoShutter.clear()
        self.ui.editInfoWindowType.clear()
        self.ui.editInfoMechCfg.clear()
        self.ui.editInfoMechRev.clear()
        self.ui.editInfoCoolingType.clear()
        self.ui.editInfoFinishFront.clear()
        self.ui.editInfoFinishBack.clear()
        self.ui.editInfoMpiRev.clear()
        self.ui.editInfoTestDate.clear()
        self.ui.editInfoTestedBy.clear()
        self.ui.editInfoTestDllRev.clear()
        self.ui.editInfoTestFwRev.clear()
        self.ui.editInfoGain.clear()
        self.ui.editInfoNoise.clear()
        self.ui.editInfoBias.clear()
        self.ui.editInfoTestTemp.clear()
        self.ui.editInfoDarkCount.clear()
        self.ui.editInfoDarkDuration.clear()
        self.ui.editInfoDarkTemp.clear()
        self.ui.editInfoCoolingDelta.clear()
        self.ui.editInfoAd0Offset.clear()
        self.ui.editInfoAd0Gain.clear()
        self.ui.editInfoAd1Offset.clear()
        self.ui.editInfoAd1Gain.clear()
        self.ui.editInfoRma1.clear()
        self.ui.editInfoRma2.clear()
        self.ui.editInfoComment1.clear()
        self.ui.editInfoComment2.clear()
        self.ui.editInfoComment3.clear()
       