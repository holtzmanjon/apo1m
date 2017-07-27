'''
Created on Mar 2, 2010

@author: ltraynor
'''
import TabBase
import PyQt4.QtCore as qtCore
import PyQt4.QtGui as qtGui
import ApexUi
import pyApg as apg

class ApexPrgmTab(TabBase.TabBase):
    '''
    Supports the programming ui tab widget
    '''


    def __init__(self,theUi,parent=None):
        '''
        Constructor
        '''
        super(ApexPrgmTab,self).__init__(theUi,parent)
                
        self.CamStackedDict = { apg.ALTAU : 0, apg.ALTAE : 0, apg.ASCENT : 1, 
                               apg.ASPEN : 2,  apg.ALTAF : 1, apg.QUAD : 1, apg.HIC : 1}
        self.CamPrgmDict = { 0 : self.OnProgramAlta, 
                            1 : self.OnProgramAscent, 
                            2 : self.OnProgramAspen }
        
        self.AspenDict = { self.ui.btnPrgmAspenFx2 : self.ui.editPrgmAspenFx2 ,
                        self.ui.btnPrgmAspenCtrl : self.ui.editPrgmAspenCtrl ,
                        self.ui.btnPrgmAspenDescriptor : self.ui.editPrgmAspenDescriptor ,
                        self.ui.btnPrgmAspenWebServer : self.ui.editPrgmAspenWebServer ,
                        self.ui.btnPrgmAspenWebPage : self.ui.editPrgmAspenPage ,
                        self.ui.btnPrgmAspenWebCfg : self.ui.editPrgmAspenWebCfg }
        
        self.AscentDict = { self.ui.btnPrgmAscentFx2 : self.ui.editPrgmAscentFx2,
                           self.ui.btnPrgmAscentCtrl : self.ui.editPrgmAscentCtrl,
                           self.ui.btnPrgmAscentDescriptor : self.ui.editPrgmAscentDescriptor }
        
        self.AltaDict = { self.ui.btnPrgmAltaFx2 : self.ui.editPrgmAltaFx2,
                         self.ui.btnPrgmAltaGpifCamCon : self.ui.editPrgmAltaGpifCamCon,
                         self.ui.btnPrgmAltaGpifBufCon : self.ui.editPrgmAltaGpifBufCon, 
                         self.ui.btnPrgmAltaGpifFifo : self.ui.editPrgmAltaGpifFifo, 
                         self.ui.btnPrgmAltaCamCon : self.ui.editPrgmAltaCamCon,
                         self.ui.btnPrgmAltaBufCon : self.ui.editPrgmAltaBufCon }
        
        
    def PrintMsg(self, msg):
        self.ui.editPrgmDownloadStatus.setText( msg )
        self.Write2OutputWin( msg )
        self.ui.editPrgmDownloadStatus.update()
        qtGui.QApplication.instance().processEvents()
        
    def SetCamera(self,cam):
        self.camRef = cam
        index = self.CamStackedDict[ self.camRef.GetPlatformType() ] 
        self.ui.stackedPrgmCam.setCurrentIndex( index )
            
    def OnProgramCamera(self):
        index = self.CamStackedDict[ self.camRef.GetPlatformType() ] 
        
        if( index != self.ui.stackedPrgmCam.currentIndex() ):
            raise RuntimeError( "programming stacked widget out of synch." )
        
        self.CamPrgmDict[ index ]()

    def OnProgramAspen(self):
        self.PrintMsg("Programming Aspen started...This will take ~3 min to complete")
        
        self.camRef.ProgramAspen( unicode( self.ui.editPrgmAspenCtrl.text() ).encode(),
            unicode( self.ui.editPrgmAspenFx2.text() ).encode(), 
            unicode( self.ui.editPrgmAspenDescriptor.text() ).encode(),
            unicode( self.ui.editPrgmAspenPage.text() ).encode(), 
            unicode( self.ui.editPrgmAspenWebServer.text() ).encode(),
            unicode( self.ui.editPrgmAspenWebCfg.text() ).encode() )
        
        self.PrintMsg("Programming Aspen completed successfully")    
        
    def OnProgramAlta(self):
        
        self.PrintMsg("Programming Alta-U started...This will take ~1 min to complete")
        
        self.camRef.ProgramAlta(unicode( self.ui.editPrgmAltaCamCon.text() ).encode(),
            unicode( self.ui.editPrgmAltaBufCon.text() ).encode(), 
            unicode( self.ui.editPrgmAltaFx2.text() ).encode(),
            unicode( self.ui.editPrgmAltaGpifCamCon.text() ).encode(),
            unicode( self.ui.editPrgmAltaGpifBufCon.text() ).encode(),
            unicode( self.ui.editPrgmAltaGpifFifo.text() ).encode() )
        
        self.PrintMsg("Programming Alta-U completed successfully")

    def OnProgramAscent(self):
        
        self.PrintMsg("Programming Ascent-based camera started...This will take ~40 secs to complete")
        
        if( apg.QUAD ==  self.camRef.GetPlatformType() ):
            self.camRef.ProgramQuad( unicode( self.ui.editPrgmAscentCtrl.text() ).encode(),
            unicode( self.ui.editPrgmAscentFx2.text() ).encode(), 
            unicode( self.ui.editPrgmAscentDescriptor.text() ).encode() )
        elif( apg.HIC ==  self.camRef.GetPlatformType() ):
            self.camRef.ProgramHiC( unicode( self.ui.editPrgmAscentCtrl.text() ).encode(),
            unicode( self.ui.editPrgmAscentFx2.text() ).encode(), 
            unicode( self.ui.editPrgmAscentDescriptor.text() ).encode() )
        elif( apg.ALTAF ==  self.camRef.GetPlatformType() ):
            self.camRef.ProgramAltaF( unicode( self.ui.editPrgmAscentCtrl.text() ).encode(),
            unicode( self.ui.editPrgmAscentFx2.text() ).encode(), 
            unicode( self.ui.editPrgmAscentDescriptor.text() ).encode() )
        else:
            self.camRef.ProgramAscent( unicode( self.ui.editPrgmAscentCtrl.text() ).encode(),
            unicode( self.ui.editPrgmAscentFx2.text() ).encode(), 
            unicode( self.ui.editPrgmAscentDescriptor.text() ).encode() )
        
        self.PrintMsg("Programming Ascent completed successfully")
                
    def OnBrowseAspen(self):
        for k,v in self.AspenDict.iteritems():             
            if( k.isChecked() ):
                objName = k.objectName()
                selectStr = "Firmware (*.bin)"
                if( 0 == objName.compare("btnPrgmAspenFx2") ):
                     selectStr = "Firmware (*.iic)"
                name = self.FileSelect( selectStr )
                if None != name:
                    k.setChecked( False )
                    v.setText( name )
            
    def OnBrowseAscent(self):
        for k,v in self.AscentDict.iteritems():             
            if( k.isChecked() ):
                objName = k.objectName()
                selectStr = "Firmware (*.bin)"
                if( 0 == objName.compare("btnPrgmAscentFx2") ):
                     selectStr = "Firmware (*.iic)"
                name = self.FileSelect( selectStr )
                if None != name:
                    k.setChecked( False )
                    v.setText( name )
            
    def OnBrowseAlta(self):
        for k,v in self.AltaDict.iteritems():             
            if( k.isChecked() ):
                objName = k.objectName()
                selectStr = "Firmware (*.bin)"
                if( 0 == objName.compare("btnPrgmAltaFx2") ):
                     selectStr = "Firmware (*.iic)"
                name = self.FileSelect( selectStr )
                if None != name:
                    k.setChecked( False )
                    v.setText( name )
                        
    def OnSerialRead(self):
        serialStr = self.camRef.GetSerialNumber()
        self.ui.editPrgmSerialNum.setText( unicode(serialStr) )
        
    def OnSerialWrite(self):
        num = unicode( self.ui.editPrgmSerialNum.text() ).encode()
        self.camRef.SetSerialNumber( num )
        
        if( apg.ALTAU == self.camRef.GetPlatformType() ):
            qtGui.QMessageBox.information(None, "IMPORTANT", "Serial number write successful. \
            PLEASE power cycle the camera to commit the number to the camera's memory.")
            
        self.ui.editPrgmSerialNum.setText("")
        
    def OnMacRead(self):
        self.ui.editPrgmMac.setText( unicode(self.camRef.GetMacAddress()) )
        
        
        
