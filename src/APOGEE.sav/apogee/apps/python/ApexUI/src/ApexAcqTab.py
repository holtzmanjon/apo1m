'''
Created on Mar 11, 2010

@author: ltraynor
'''

import TabBase
import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import ApexUi
import pyApg as apg
import Image
import os
import datetime
import subprocess
import ViewerDlg
import numpy as np


class ApexAcqTab(TabBase.TabBase):
    '''
    Supports the acquisition ui tab widget
    '''


    def __init__(self,theUi,parent=None):
        '''
        Constructor
        '''
        super(ApexAcqTab,self).__init__(theUi,parent)
        
        self.AcqMode = 0;
        self.ModeDict  = { 0 : (0,self.ui.rbAcqModeNorm, self.StartExposureNorm, self.Check4ImgNorm),
                               1 : (1,self.ui.rbAcqModeTdi, self.StartExposureTdi, self.Check4ImgTdi),
                               2 : (2,self.ui.rbAcqModeKinetics, self.StartExposureKinetics, self.Check4ImgKinetics),
                               3 : (3,self.ui.rbAcqModeRatio, self.StartExposureRatio, self.Check4ImgRatio)}
      
        self.numImgsAcqed = 0
        self.colormapPil = [ i/256 for i in range(65536) ]

      
        self.ViewDlg = None
                
    def closeViewer(self):
        if self.ViewDlg:
            self.ViewDlg.done(0)
            
    def PrintMsg(self, msg):
        self.ui.editAcqStatus.setText( msg )
        outMsg = "Acq: " + msg
        self.Write2OutputWin( outMsg )
        self.ui.editAcqStatus.update()
        qtGui.QApplication.instance().processEvents()
        
    def UpdateMaxBinning(self):
        self.ui.spinAcqNormBinCol.setMaximum( self.camRef.GetMaxBinCols() )
        self.ui.spinAcqNormBinRow.setMaximum( self.camRef.GetMaxBinRows() )
        
        self.ui.spinAcqRatioBinCol.setMaximum( self.camRef.GetMaxBinCols() )
        self.ui.spinAcqRatioBinRow.setMaximum( self.camRef.GetMaxBinRows() )
                        
    def UpdateSelectedMode(self, newMode, oldMode):
        #toggle ratio mode
        if 3 == newMode:
            #this can throw, if it does reset the ui the previous
            #state
            try:
                self.ToggleRatio( True )
            except:
                self.ModeDict[oldMode][1].setChecked( True )
                raise
        
        if 3 == oldMode:
            self.ToggleRatio( False )
            
        self.AcqMode = newMode
        index = self.ModeDict[self.AcqMode][0]
        self.ui.stackedAcqMode.setCurrentIndex( index )
          
    def ToggleRatio(self,onOff):
        if(  self.camRef.IsFastSequenceOn() != onOff ):
            self.camRef.SetFastSequence( onOff )
    
    def OnSaveOrViewer(self):
        if self.ui.rbAcqViewImg.isChecked():
            self.ui.gbAcqFile.setEnabled( False )
            if None == self.ViewDlg:
                self.ViewDlg = ViewerDlg.ViewerDlg()
                self.connect(self.ViewDlg, qtCore.SIGNAL("finished(int)"),
                             self.OnViewerFinished )
                self.ViewDlg.show()
        else:
            self.ViewDlg.done(0)
            self.ui.gbAcqFile.setEnabled( True )
                
    def OnViewerFinished(self,value):
        self.ViewDlg = None
        
    def OnModeRadio(self):
        newMode = 0        
        if self.ui.rbAcqModeNorm.isChecked():
            newMode = 0
        elif self.ui.rbAcqModeTdi.isChecked():
            newMode = 1
        elif self.ui.rbAcqModeKinetics.isChecked():
            newMode = 2
        elif self.ui.rbAcqModeRatio.isChecked():
            newMode = 3
        else:
            raise RuntimeError( "No mode selected" )
        
        self.UpdateSelectedMode(newMode, self.AcqMode)
        
    def OnCcdAcdRadio(self):
        if self.ui.rbAcqAdcCcdNormal.isChecked():
            self.camRef.SetCcdAdcSpeed( apg.AdcSpeed_Normal )
        elif self.ui.rbAcqAdcCcdVideo.isChecked():
            self.camRef.SetCcdAdcSpeed( apg.AdcSpeed_Video )
        else:
            self.camRef.SetCcdAdcSpeed( apg.AdcSpeed_Fast )
           
            
        self.UpdateMaxBinning()
        
    def OnAdcSettingsRead(self):
        adc = self.ui.spinAcqAdcNum.value()
        channel = self.ui.spinAcqAdcChannel.value()
        self.ui.spinAcqAdcGain.setValue( 
                                     self.camRef.GetAdcGain( adc, channel ) )
        
         #cannot get altau 16bit adc offsets
        if( 0 == adc and apg.ALTAU == self.camRef.GetPlatformType() ):
            self.ui.spinAcqAdcOffset.setValue( 0 )
        else:
             self.ui.spinAcqAdcOffset.setValue( 
                                     self.camRef.GetAdcOffset( adc, channel ) )
        
    def OnAdcSettingsWrite(self):
        adc = self.ui.spinAcqAdcNum.value()
        channel = self.ui.spinAcqAdcChannel.value()
        
        #cannot set altau 16bit adc gains or offsets
        canSet = True
        if( 0 == adc and apg.ALTAU == self.camRef.GetPlatformType() ):
            canSet = False
            
        if( canSet ):
            self.camRef.SetAdcGain( self.ui.spinAcqAdcGain.value(), adc, channel )
            self.camRef.SetAdcOffset( self.ui.spinAcqAdcOffset.value(), adc, channel )
            
        #read back the settings
        self.OnAdcSettingsRead()
          
    def Update4Cam(self):
        #TODO - incomplete
        self.ui.spinAcqNormStartRow.setValue( self.camRef.GetRoiStartRow() )
        self.ui.spinAcqNormStartRow.setMaximum( self.camRef.GetMaxImgRows() )
        
        self.ui.spinAcqNormNumRow.setMaximum( self.camRef.GetMaxImgRows() )
        self.ui.spinAcqNormNumRow.setValue( self.camRef.GetMaxImgRows() )
        
        self.ui.spinAcqNormStartCol.setValue( self.camRef.GetRoiStartCol() )
        self.ui.spinAcqNormStartCol.setMaximum( self.camRef.GetMaxImgCols() )
        
        self.ui.spinAcqNormNumCol.setMaximum( self.camRef.GetMaxImgCols() )
        self.ui.spinAcqNormNumCol.setValue( self.camRef.GetMaxImgCols() )
        
        self.ui.spinAcqRatioStartRow.setValue( self.camRef.GetRoiStartRow() )
        self.ui.spinAcqRatioStartRow.setMaximum( self.camRef.GetMaxImgRows() )
        
        self.ui.spinAcqRatioNumRow.setMaximum( self.camRef.GetMaxImgRows() )
        self.ui.spinAcqRatioNumRow.setValue( self.camRef.GetMaxImgRows() )
        
        self.ui.spinAcqRatioStartCol.setValue( self.camRef.GetRoiStartCol() )
        self.ui.spinAcqRatioStartCol.setMaximum( self.camRef.GetMaxImgCols() )
        
        self.ui.spinAcqRatioNumCol.setMaximum( self.camRef.GetMaxImgCols() )
        self.ui.spinAcqRatioNumCol.setValue( self.camRef.GetMaxImgCols() )
        
        self.UpdateMaxBinning()
        
        if( self.camRef.IsBulkDownloadOn() ):
            self.ui.cbAcqNormBulkOnOff.setChecked( True )
        else:
            self.ui.cbAcqNormBulkOnOff.setChecked( False )
        
        self.OnReadSeqDelay()    
        
    def OnReadSeqDelay(self):
        self.ui.spinAcqNormVarDelay.setValue( self.camRef.GetSequenceDelay() )
            
    def OnWriteSeqDelay(self):
        self.camRef.SetSequenceDelay( 
                                     self.ui.spinAcqNormVarDelay.value() )
        self.OnReadSeqDelay()
        
    def OnToggleBulkSeq(self):
        if self.ui.cbAcqNormBulkOnOff.isChecked():
            self.camRef.SetBulkDownload( True )
        else:
            self.camRef.SetBulkDownload( False )
            
    def OnToggleBulkSeqVariableDelay(self):
        if self.ui.cbAcqNormBulkVarDelay.isChecked():
            self.camRef.SetVariableSequenceDelay( True )
        else:
            self.camRef.SetVariableSequenceDelay( False )
            
    def OnToggleExposure(self):
        if self.ui.btnAcqExposeToggle.isChecked() :
            self.ModeDict[self.AcqMode][2]()
            self.ui.btnAcqExposeToggle.setText("Stop")
            self.PrintMsg("Exposure Started")
        else:
            self.StopExposure()
            self.ui.btnAcqExposeToggle.setText("Start")
        
    def StartExposureNorm(self): 
        #define the ROI
        self.camRef.SetRoiStartRow( self.ui.spinAcqNormStartRow.value() )
        self.camRef.SetRoiNumRows( self.ui.spinAcqNormNumRow.value() )
        
        self.camRef.SetRoiStartCol( self.ui.spinAcqNormStartCol.value() )
        self.camRef.SetRoiNumCols( self.ui.spinAcqNormNumCol.value() )
        
        #set the number of images
        self.numImgsAcqed = 0 
        self.camRef.SetImageCount( self.ui.spinAcqNormNumImg.value() )
                        
        #start the exposure
        try:
            self.camRef.StartExposure( self.ui.spinAcqDuration.value(), 
                                   self.ui.rbAcqLigth.isChecked() )
        except:
            self.RestoreStartExposure()
            raise
       
    def StartExposureTdi(self):
        #define the ROI
        self.camRef.SetRoiStartRow( 0 )
        self.camRef.SetRoiNumRows( self.ui.spinAcqTdiRows.value() )
        
        self.camRef.SetRoiStartCol( 0 )
        self.camRef.SetRoiNumCols( 1 )
        
        #set the number of images
        self.numImgsAcqed = 0 
        self.camRef.SetImageCount( self.ui.spinAcqNormNumImg.value() )
                        
        #start the exposure
        try:
            self.camRef.StartExposure( self.ui.spinAcqDuration.value(), 
                                   self.ui.rbAcqLigth.isChecked() )
        except:
            self.RestoreStartExposure()
            raise
    
    def StartExposureKinetics(self):
        pass
    
    def StartExposureRatio(self):
        self.curSeqImgNum = 1
        self.camRef.SetImageCount( self.ui.spinAcqRatioNumImgs.value() )
        self.camRef.SetRoiStartRow( self.ui.spinAcqRatioStartRow.value() )
        self.camRef.SetRoiNumRows( self.ui.spinAcqRatioNumRow.value() )
        
        self.camRef.SetRoiStartCol( self.ui.spinAcqRatioStartCol.value() )
        self.camRef.SetRoiNumCols( self.ui.spinAcqRatioNumCol.value() )
        
                        
        #start the exposure
        self.camRef.StartExposure( self.ui.spinAcqDuration.value(), 
                                   self.ui.rbAcqLigth.isChecked() )
        
    def StopExposure(self):
        self.camRef.StopExposure( False )
        self.PrintMsg("Exposure Stopped")
    
    def Check4Image(self):
        self.ModeDict[ self.AcqMode ][3]()
      
    def Check4ImgNorm(self):
        status = self.camRef.GetImagingStatus()
        
        if( apg.Status_ImageReady == status ):
            self.PrintMsg("Image Ready")
            
            self.PrintMsg("Downloading image/s %d" % (self.numImgsAcqed) )
            
            numImgs = 0
            if( self.camRef.IsBulkDownloadOn() ):
                self.GetMultipleImages()
                numImgs = self.camRef.GetImageCount()
            else:
               self.GetSingleImage()
               numImgs = 1
            
            self.numImgsAcqed += numImgs
            
            if( self.numImgsAcqed == self.camRef.GetImageCount() ):
                if self.ui.btnAcqExposeToggle.isChecked():
                    self.PrintMsg("Acquisition completed successfully")
                else:
                    self.PrintMsg("Partial image saved")
                self.RestoreStartExposure() 
            else:
                self.PrintMsg("Image download complete")
            
    def Check4ImgTdi(self):
        pass
    
    def Check4ImgKinetics(self):
        pass
    
    def Check4ImgRatio(self):
        pass

    
    def GetSingleImage(self ):
               
        #fetching the image from the camera
        theImg = self.camRef.GetImage()
                      
        
        if( self.ui.rbAcqViewImg.isChecked() ):
            dimCR = ( self.camRef.GetRoiNumCols(), self.camRef.GetRoiNumRows() ) 
            self.ViewImage(theImg,dimCR)
        else:
            fullPath = self.GetImageFileName(None)
            
            if( self.ui.rbAcqOutputPng.isChecked() ):
                dimCR = ( self.camRef.GetRoiNumCols(), self.camRef.GetRoiNumRows() ) 
                theImg = theImg.reshape( dimCR[1], dimCR[0] )
                im = Image.fromarray( theImg.astype(np.int32), "I" )
                self.PrintMsg( "Saving image to file" )
                im.save( fullPath )
            else:
                self.PrintMsg( "Saving image to file" )
                theImg.tofile( fullPath )
            
    def GetMultipleImages(self):
        theImg = self.camRef.GetImage()
        
        dimCR = ( self.camRef.GetRoiNumCols(), self.camRef.GetRoiNumRows() )
        imgSize = dimCR[0]*dimCR[1]
        
        
        for i in range( 0, self.camRef.GetImageCount() ):
            self.PrintMsg( "Saving image %d to file" % i )
            
            fullPath = self.GetImageFileName(i)
            
            im = Image.new("I",dimCR)
            start = i*imgSize
            end = start+imgSize-1
            im.putdata( theImg[start:end] )
            im.save( fullPath )
                
    def GetImageFileName(self, num):
        
        userBaseName = unicode(self.ui.editAcqOutFilename.text()).encode()
        
        if( "" == userBaseName ):
            userBaseName= "noname"
            
        type = "bin"
        if( self.ui.rbAcqOutputPng.isChecked() ):
            type = "png"
            
        if( None != num ):
            fileName = "%s-%d.%s" % ( userBaseName, num, type )
        else:
            fileName = "%s.%s" % ( userBaseName, type )
            
        fileDir = unicode( self.ui.editAcqOutDir.text() ).encode()
        fullPath = os.path.join( fileDir, 
                             fileName)
        
        #if file already exists add time/date stamp
        if( os.path.isfile(fullPath) ):
             t = datetime.datetime.now()
             timestamp = t.strftime("%Y-%m-%d-%H%M%S")
             fileName = "%s-%s.%s" % ( userBaseName, timestamp, type )
             fullPath = os.path.join( unicode( self.ui.editAcqOutDir.text() ).encode(), 
                             fileName)
             
        return fullPath
        
    def ViewImage(self, data, sizeTuple):
        self.ViewDlg.DisplayImage( data, sizeTuple )
        
                 
    def RestoreStartExposure(self):
         #popping up the start/stop button
        self.ui.btnAcqExposeToggle.setChecked( False )
        self.ui.btnAcqExposeToggle.setText("Start")
        
    def OnLinkChecked(self):
        if( self.ui.checkAcqNormBinLink.isChecked() ):
            
            #set both to the number of columns
            self.ui.spinAcqNormBinRow.setValue( self.ui.spinAcqNormBinCol.value() )
            
            #set the max to lowest max value
            if( self.camRef.GetMaxBinCols() <= self.camRef.GetMaxBinRows() ):
                self.ui.spinAcqNormBinCol.setMaximum( self.camRef.GetMaxBinCols() )
                self.ui.spinAcqNormBinRow.setMaximum( self.camRef.GetMaxBinCols())
            else:
                self.ui.spinAcqNormBinCol.setMaximum( self.camRef.GetMaxBinRows() )
                self.ui.spinAcqNormBinRow.setMaximum( self.camRef.GetMaxBinRows() )
        else:
            self.UpdateMaxBinning()
            
    def OnRowBin(self, value):
        
        #set the binning value in the library
        self.camRef.SetRoiBinRow( value )
        
        #update the UI so these new values can
        #be used during exposure
        self.UpdateRowRoi()
        
        if( self.ui.checkAcqNormBinLink.isChecked() ):
            self.camRef.SetRoiBinCol( value )
            self.ui.spinAcqNormBinCol.setValue( value )
            self.UpdateColRoi()
            
    def UpdateRowRoi(self):
         #update the roi with new roi values
        newRoiRows = self.camRef.GetMaxImgRows() / self.camRef.GetRoiBinRow()
        self.ui.spinAcqNormNumRow.setMaximum( newRoiRows )
        self.ui.spinAcqNormNumRow.setValue( newRoiRows )
        
    def OnColBin(self, value):
        
        #set the binning value in the library
        self.camRef.SetRoiBinCol( value )
        
        #update the UI so these new values can
        #be used during exposure
        self.UpdateColRoi()
        
        if( self.ui.checkAcqNormBinLink.isChecked() ):
             self.camRef.SetRoiBinRow( value )
             self.ui.spinAcqNormBinRow.setValue( value )
             self.UpdateRowRoi()
             
    def UpdateColRoi(self):
        #update the roi with new roi values
        newRoiCols = self.camRef.GetMaxImgCols() / self.camRef.GetRoiBinCol()
        self.ui.spinAcqNormNumCol.setMaximum( newRoiCols )
        self.ui.spinAcqNormNumCol.setValue( newRoiCols )
        
    def OnOutDirBrowse(self):
             name = self.DirSelect()
             self.ui.editAcqOutDir.setText( unicode( name ) )
                         
    
        
             
                          
             
            
        
        
       
        
        
        
